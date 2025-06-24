#include "main.h"

uint32_t tmpBuffer32[256];
uint8_t tmpBuffer8[1024];

uint8_t flashTmp[STR_MAX_SIZE_IN_FLASH*4];						// массив для чтения и записи настроек во flash-память
//flashparam_t flashParam;										// в этой структуре храним настройки из флеш памяти

void Internal_Flash_Write(uint8_t* data, uint32_t address, uint32_t size)
{
	unsigned int i;

	while (FLASH->SR & FLASH_SR_BSY);
	if (FLASH->SR & FLASH_SR_EOP) FLASH->SR = FLASH_SR_EOP;
	FLASH->CR |= FLASH_CR_PG;

	for (i = 0; i < size; i += 2) {
		*(volatile uint16_t*)(address + i) = (((uint16_t)data[i + 1]) << 8) + data[i];
		while (!(FLASH->SR & FLASH_SR_EOP));
		FLASH->SR = FLASH_SR_EOP;
	}
	FLASH->CR &= ~(FLASH_CR_PG);
}

void Internal_Flash_Erase(uint32_t pageAddress)
{
	while (FLASH->SR & FLASH_SR_BSY);
	if (FLASH->SR & FLASH_SR_EOP) {
		FLASH->SR = FLASH_SR_EOP;
	}

	FLASH->CR |= FLASH_CR_PER;
	FLASH->AR = pageAddress;
	FLASH->CR |= FLASH_CR_STRT;
	while (!(FLASH->SR & FLASH_SR_EOP));
	FLASH->SR = FLASH_SR_EOP;
	FLASH->CR &= ~FLASH_CR_PER;
}


void flashWrite(uint32_t address, uint8_t *data, uint32_t size, uint8_t erase)
{
	FLASH->KEYR = 0x45670123; FLASH->KEYR = 0xCDEF89AB;
	if (erase) Internal_Flash_Erase(address);
	Internal_Flash_Write(data, address, size);
	FLASH->KEYR = 0x00; FLASH->KEYR = 0x00;
}

void flashRead(uint32_t address, uint8_t *readBuffer, uint32_t size)
{
	memset(tmpBuffer8, 0x00, sizeof(tmpBuffer8)); memset(tmpBuffer32, 0x00, sizeof(tmpBuffer32));
	for(uint32_t i=0; i<size; i+=4){
		tmpBuffer32[i/4] = *(__IO uint32_t*)(address+i);
	}
	memcpy(readBuffer, tmpBuffer32, size);
    //printf("%s\r\n", tmpBuffer8);
}

//////////////////////////////////////////////////
//												//
//////////////////////////////////////////////////

uint16_t getCRC(uint8_t *buff, uint32_t size)
{
	uint16_t base=0xffff;
	while(size--) base-=(*buff++);
	return base;
}

uint8_t setFlashParam(uint8_t *param, flashAddr_t addrParam, uint8_t size)
{
	// читаем область флеш-памяти, где храняться настройки
	flashRead(FLASH_CONF_PAGE*FLASH_PAGE_SIZE, flashTmp, STR_MAX_SIZE_IN_FLASH*4);
	
	// определяем, требуется ли стирание страницы
	// если в диапозоне адресов от addrParam до addrParam+size, будет хотя бы один байт, который не равен 0xff,
	// то стираем страницу, иначе стирание не требуется
	volatile uint8_t erase=0;
	(void)erase;
	for(uint32_t i=0; i<STR_MAX_SIZE_IN_FLASH; i++){
		if(flashTmp[addrParam+i]!=0xff){erase=1; break;}
	}
	// подготавливаем массив для записи во флеш-память
	memset(&flashTmp[addrParam], 0xFF, STR_MAX_SIZE_IN_FLASH);					// устанавливаем все байты в 0xff
	memcpy(&flashTmp[addrParam], param, size);									// копируем новые данные в массив
	flashWrite(FLASH_CONF_ADDR, flashTmp, STR_MAX_SIZE_IN_FLASH*4, 1);			// записываем все настройки целиком. 
	// проверка записанных данных
	uint16_t crc=getCRC(flashTmp, STR_MAX_SIZE_IN_FLASH*4);						// считаем crc16 записанного массива 
	flashRead(FLASH_CONF_ADDR, flashTmp, STR_MAX_SIZE_IN_FLASH*4);				// считываем ещё раз все настройки, что бы проверить корректность записи
	return (crc==getCRC(flashTmp, STR_MAX_SIZE_IN_FLASH*4))?0:1;				// если суммы равны, то возвращаем 0, иначе 1
}

uint8_t searchChar(uint8_t *buff, uint8_t _char, uint16_t size)
{
	for(uint16_t i=0; i<size; i++)
		if(buff[i]==_char) return i+1;
	return 0;
}

void getFlashParam()
{
	uint8_t size=0;
	memset(flashTmp, 0x00, STR_MAX_SIZE_IN_FLASH*4);
	flashRead(FLASH_CONF_ADDR, flashTmp, STR_MAX_SIZE_IN_FLASH*4);

	size=searchChar(&flashTmp[FADDR_SERVER], 0xff, STR_MAX_SIZE_IN_FLASH);
	if(size) memcpy(esp.param.server, &flashTmp[FADDR_SERVER], size-1);

	size=searchChar(&flashTmp[FADDR_PORT], 0xff, STR_MAX_SIZE_IN_FLASH);
	if(size) memcpy(esp.param.port, &flashTmp[FADDR_PORT], size-1);

	size=searchChar(&flashTmp[FADDR_AP_NAME], 0xff, STR_MAX_SIZE_IN_FLASH);
	if(size) memcpy(esp.param.apName, &flashTmp[FADDR_AP_NAME], size-1);

	size=searchChar(&flashTmp[FADDR_AP_PASS], 0xff, STR_MAX_SIZE_IN_FLASH);
	if(size) memcpy(esp.param.apPass, &flashTmp[FADDR_AP_PASS], size-1);
}