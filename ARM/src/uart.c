#include "main.h"

//#define CLOCK_FREQ 64000000 // Частота ядра (из предыдущего примера)

// Буфер для приёма
//volatile uint8_t rx_buffer[ESP_BUF_SIZE];
//volatile uint16_t rx_index = 0;

extern esp_t esp;

void UART1_Init() 
{
    // Включаем тактирование USART1 и GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN;
    
    // Настройка PA9 (TX) как Alternate Push-Pull, PA10 (RX) как Input Floating
    GPIOA->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9 | GPIO_CRH_CNF10 | GPIO_CRH_MODE10);
    GPIOA->CRH |= GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9 | GPIO_CRH_CNF10_0;
    
    // Настройка скорости (BRR = CLOCK_FREQ / BAUDRATE)
    USART1->BRR = (SYSFRAQ + BAUDRATE/2) / BAUDRATE;
    
    // Включаем прерывания:
    USART1->CR1 |= USART_CR1_RXNEIE;  // По приёму байта
    USART1->CR1 |= USART_CR1_IDLEIE;  // По IDLE (таймаут)
    
    // Включаем USART, передатчик и приёмник
    USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
    
    // Настройка NVIC
    NVIC_SetPriority(USART1_IRQn, 1);
    NVIC_EnableIRQ(USART1_IRQn);
    __enable_irq();
}

void USART1_IRQHandler() 
{
    // 1. Обработка приёма байта
    if(USART1->SR & USART_SR_RXNE) {
        uint8_t data = USART1->DR; // Чтение очищает RXNE
        esp.rx_buffer[esp.rx_index++] = data;
        
        // Защита от переполнения
        //if(esp.rx_index >= sizeof(esp.rx_buffer)) esp.rx_index = 0;
    }
    
    // 2. Обработка IDLE (таймаут)
    if(USART1->SR & USART_SR_IDLE) {
        (void)USART1->DR; // Обязательное чтение DR для очистки IDLE
        //memset(rx_buffer, 0x00, ESP_BUF_SIZE);
        //rx_index=0;
        esp.dataRDY=1;
    }
}

void UART1_SendByte(uint8_t data) 
{
    while(!(USART1->SR & USART_SR_TXE)); // Ждём готовности передатчика
    USART1->DR = data;
}

void UART1_SendString(const char* str) 
{
    while(*str) {
        UART1_SendByte(*str++);
    }
}
