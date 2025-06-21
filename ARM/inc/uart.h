#ifndef UART_H
#define UART_H

#define BAUDRATE (115200U)


void UART1_Init(void);
void UART1_SendString(const char* str);

#endif  //UART_H