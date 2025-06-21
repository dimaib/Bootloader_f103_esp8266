#include "main.h"

int _write(int file, char *ptr, int len) 
{
    SEGGER_RTT_Write(0, ptr, len);
    return len;
}

uint8_t rcc_init()
{
    // 1. Включаем тактирование необходимых периферийных блоков
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | 
                   	RCC_APB2ENR_IOPDEN | RCC_APB2ENR_AFIOEN;

    for (uint8_t i=0; ; i++){
        if(RCC->CR & (1<<RCC_CR_HSIRDY_Pos))
          break;
        if(i == 255)
          return 1;
    }
    RCC->CFGR |= RCC_CFGR_PLLMULL9;     /* 0x000C0000 - PLL input clock*9 */
    RCC->CFGR |= RCC_CFGR_SW_1;         /* 0x00000002 - PLL selected as system clock */
    RCC->CR |=RCC_CR_PLLON;             /* 0x01000000 - PLL enable */
    for (uint8_t i=0; ; i++){
        if(RCC->CR & (1U<<RCC_CR_PLLRDY_Pos))
            break;
        if(i==255){
            RCC->CR &= ~(1U<<RCC_CR_PLLON_Pos);
            return 2;
        }
    }
    return 0;
}

void GPIO_Init()
{
    // 1. Включаем тактирование портов A и C
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPCEN;
    
    // 2. Настройка PA8 как выход (push-pull, 50 MHz)
    GPIOA->CRH &= ~(GPIO_CRH_CNF8_Msk);                     // Очищаем биты PA8
    GPIOA->CRH |= GPIO_CRH_MODE8_1 | GPIO_CRH_MODE8_0;      // Output 50 MHz
    GPIOA->CRH |= GPIO_CRH_CNF8_0;                          // Push-pull (0b00)
    // 3. Настройка PC13 как выход (push-pull, 2 MHz)
    GPIOC->CRH &= ~(GPIO_CRH_CNF13_Msk);                    // Очищаем биты PC13
    GPIOC->CRH |= GPIO_CRH_MODE13_0;                        // Output 2 MHz (0b01)
    GPIOC->CRH &= ~GPIO_CRH_CNF13;                          // Push-pull (0b00)
    // 4. Инициализация начального состояния
    GPIOA->BSRR = GPIO_BSRR_BR8;                            // PA8 = 1 (включить)
    GPIOC->BSRR = GPIO_BSRR_BS13;                           // тут инвертный светодиод. Поэтому, что бы его выключить подаём единицу на пин
}


void delay_ms(uint32_t ms)
{
    // Калибровка для 64 МГц (из вашего примера)
    //#define CYCLES_PER_MS 36000
    
    while(ms--) {
        volatile uint32_t count = (SYSFRAQ/1000)/7;
        while(count--);
    }
}