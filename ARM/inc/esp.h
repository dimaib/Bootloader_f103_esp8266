#ifndef ESP_H
#define ESP_H

#define ESP_BUF_SIZE            1200

typedef struct{
	GPIO_TypeDef	*GPIOx;
	uint16_t		GPIO_Pin;
    uint8_t         connect_status;
    uint8_t         getFileMode;
	uint8_t 		dataRDY;
    uint8_t         rx_buffer[ESP_BUF_SIZE];
    uint16_t        rx_index;
    uint32_t        timeoutRX;
}esp_t;

void esp_reset(void);
extern esp_t esp;

#endif  //ESP_H