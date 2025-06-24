#ifndef ESP_H
#define ESP_H

#define ESP_BUF_SIZE            1200

#define STR_MAX_SIZE_IN_FLASH   128

typedef struct{
    uint8_t server[STR_MAX_SIZE_IN_FLASH];
    uint8_t port[STR_MAX_SIZE_IN_FLASH];
    uint8_t apName[STR_MAX_SIZE_IN_FLASH];
    uint8_t apPass[STR_MAX_SIZE_IN_FLASH];
}flashparam_t;

typedef struct{
	GPIO_TypeDef	*GPIOx;
	uint16_t		GPIO_Pin;
    uint8_t         connect_status;
    uint8_t         getFileMode;
	uint8_t 		dataRDY;
    uint8_t         rx_buffer[ESP_BUF_SIZE];
    uint16_t        rx_index;
    uint32_t        timeoutRX;
    flashparam_t    param;         
}esp_t;

void esp_reset(void);
extern esp_t esp;

#endif  //ESP_H