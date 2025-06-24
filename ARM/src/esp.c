#include "main.h"


esp_t esp={
	.GPIOx=GPIOA,
	.GPIO_Pin=(1<<8),
	.dataRDY=0,
    .rx_index=0,
    .connect_status=0,
    .getFileMode=0,
	.timeoutRX=0,
	.param.server="",
	.param.port="",
	.param.apName="",
	.param.apPass=""
};

void esp_reset()
{
	NVIC_DisableIRQ(USART1_IRQn);
	esp.GPIOx->ODR&=~esp.GPIO_Pin; delay_ms(100);
	esp.GPIOx->ODR|=esp.GPIO_Pin; delay_ms(300);
	NVIC_EnableIRQ(USART1_IRQn);
	UART1_SendString("ATE0\r\n");
}
