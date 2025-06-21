#ifndef SYSFUNC_H
#define SYSFUNC_H

int _write(int file, char *ptr, int len);
uint8_t rcc_init(void);
void GPIO_Init(void);
void delay_ms(uint32_t ms);


#endif  //SYSFUNC_H