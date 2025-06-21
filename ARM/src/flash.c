#include "main.h"

uint8_t __dump[]={
	0x14, 0x0C, 0xBC, 0xE8, 0x0F, 0x00, 0xAE, 0xE8, 
	0x0F, 0x00, 0xDC, 0xF8, 0x00, 0x30, 0xCE, 0xF8, 
	0x00, 0x30, 0x00, 0x21, 0x68, 0x46, 0x03, 0xF0, 
	0x88, 0xF8, 0x06, 0x4B, 0x18, 0x60, 0x0A, 0xB0
};

uint8_t dump[]="111 read/write in 55555 memory!";
uint32_t tmpBuffer32[256];
uint8_t tmpBuffer8[1024];

void Internal_Flash_Write(uint8_t* data, uint32_t address, uint32_t size) {
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
	memset(tmpBuffer8, 0x00, 1024); memset(tmpBuffer32, 0x00, 256);
	for(uint32_t i=0; i<size; i+=4){
		tmpBuffer32[i/4] = *(__IO uint32_t*)(address+i);
	}
	memcpy(readBuffer, tmpBuffer32, size);
    //printf("%s\r\n", tmpBuffer8);
}