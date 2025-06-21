#ifndef FLASH_H
#define FLASH_H

#define FLASH_START_ADDR        0x08000000
#define FLASH_PAGE_SIZE			1024
#define FLASH_CONF_PAGE         10
#define FLASH_CONF_ADDR         (FLASH_START_ADDR+(FLASH_CONF_PAGE*FLASH_PAGE_SIZE))

typedef enum{
    FADDR_SERVER    = /*FLASH_START_ADDR+(FLASH_CONF_PAGE*FLASH_PAGE_SIZE)+*/(128*0),
    FADDR_PORT      = /*FLASH_START_ADDR+(FLASH_CONF_PAGE*FLASH_PAGE_SIZE)+*/(128*1),
    FADDR_AP_NAME   = /*FLASH_START_ADDR+(FLASH_CONF_PAGE*FLASH_PAGE_SIZE)+*/(128*2),
    FADDR_AP_PASS   = /*FLASH_START_ADDR+(FLASH_CONF_PAGE*FLASH_PAGE_SIZE)+*/(128*3)
}flashAddr_t;

void flashWrite(uint32_t address, uint8_t *data, uint32_t size, uint8_t erase);
void flashRead(uint32_t address, uint8_t *readBuffer, uint32_t size);

#endif  //FLASH_H