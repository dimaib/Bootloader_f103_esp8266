#ifndef FLASH_H
#define FLASH_H

#include "esp.h"

#define FLASH_START_ADDR        0x08000000
#define FLASH_PAGE_SIZE			1024
#define FLASH_CONF_PAGE         12
#define FLASH_CONF_ADDR         (FLASH_START_ADDR+(FLASH_CONF_PAGE*FLASH_PAGE_SIZE))

typedef enum{
    FADDR_SERVER    = (STR_MAX_SIZE_IN_FLASH*0),
    FADDR_PORT      = (STR_MAX_SIZE_IN_FLASH*1),
    FADDR_AP_NAME   = (STR_MAX_SIZE_IN_FLASH*2),
    FADDR_AP_PASS   = (STR_MAX_SIZE_IN_FLASH*3)
}flashAddr_t;

void flashWrite(uint32_t address, uint8_t *data, uint32_t size, uint8_t erase);
void flashRead(uint32_t address, uint8_t *readBuffer, uint32_t size);

uint8_t setFlashParam(uint8_t *param, flashAddr_t addrParam, uint8_t size);
void getFlashParam(void);
uint8_t searchChar(uint8_t *buff, uint8_t _char, uint16_t size);

#endif  //FLASH_H