#include "main.h"

uint8_t _go=0;

uint16_t getDataSize(uint8_t* str)
{
    uint8_t *point = (uint8_t *)strstr((const char*)str, "+IPD,0");
    if (point!=NULL && point[7] >= '0' && point[7]<= '9') 
        return atoi((const char*)&point[7]);
    return 0;
}


void sendToClient(uint8_t *str, uint16_t size)
{
	uint8_t strTmp[32]={0};
	sprintf((char*)strTmp,"AT+CIPSEND=0,%d\r\n", size);
	UART1_SendString((const char*)strTmp); delay_ms(20);
	UART1_SendString((const char*)str); delay_ms(10);
}

uint8_t elapsedTimer(uint32_t* _timer, uint32_t ms)
{
    if ((*_timer) == 0 || (*_timer)>ms) {
        *_timer = ms;
    } else if (--(*_timer) == 0) {
        *_timer = ms;
        return 1;
    }
    return 0;
}

uint8_t test[][128]={
	"http://google.com",
	"8080",
	"fuji_roll",
	"sfhjkjfdgrfsrghdytj"
};

uint8_t _test[4][128];

int main()
{
	rcc_init();
	GPIO_Init();
	UART1_Init();
	printf("start to rtt\r\n");
	esp_reset();

	UART1_SendString("AT+CWMODE=2\r\n"); delay_ms(100);
	UART1_SendString("AT+CWSAP=\"stm32_bootloader\",\"23092309\",5,3\r\n"); delay_ms(100);
	UART1_SendString("AT+CIPMUX=1\r\n"); delay_ms(100);
	UART1_SendString("AT+CIPSERVER=1,8080\r\n"); delay_ms(100);
	esp.dataRDY=0; esp.rx_index=0;
	memset(esp.rx_buffer, 0x00, ESP_BUF_SIZE);

	uint16_t packetNum=0;
	uint16_t packetNumPre=0xffff;
	uint32_t eTimer[]={0,0,0};
	uint32_t shiftAddr=0;

	// инициализируем структуру настроек для wifi. Загружаем данные из flash
	getFlashParam();

	while(1){
		if (elapsedTimer(&eTimer[0], 1500)) 			{GPIOC->ODR&=~(1<<13); eTimer[2]=1;}
		if (eTimer[2] && elapsedTimer(&eTimer[1], 5)) 	{GPIOC->ODR|=(1<<13); eTimer[2]=0;}
		
		delay_ms(1);
		if(_go){
			_go=0;
			// тестовый код для запуска из отладчика, путём установки глобальной переменной _go в 1
		}
		if(esp.getFileMode) esp.timeoutRX++;
		if(esp.timeoutRX>=7000){
			printf("exit getFileMode\r\n");
			esp.getFileMode=0; esp.timeoutRX=0;
			packetNum=0; packetNumPre=0xffff;
			shiftAddr=0;
			memset(esp.rx_buffer, 0x00, esp.rx_index); esp.rx_index=0;
		}
		if(esp.dataRDY){
			esp.dataRDY=0;
			// вывод всей принятой информации из uart
			//printf("size: %d[%d]: %s\r\n", esp.rx_index, getDataSize(esp.rx_buffer), esp.rx_buffer);
			if(strstr((const char*)esp.rx_buffer, "0,CONNECT")) {esp.connect_status=1; memset(esp.rx_buffer, 0x00, esp.rx_index); esp.rx_index=0;}
			if(strstr((const char*)esp.rx_buffer, "0,CLOSED")) 	{esp.connect_status=0; memset(esp.rx_buffer, 0x00, esp.rx_index); esp.rx_index=0;}
			if(strstr((const char*)esp.rx_buffer, "+IPD,0")){
				uint16_t size=getDataSize(esp.rx_buffer);
				uint8_t dataPos=searchChar(esp.rx_buffer, ':', ESP_BUF_SIZE);
				if(!esp.getFileMode){
					if(strstr((const char*)esp.rx_buffer, "start_transmit")) {
						printf("begin recive file..\r\n");
						sendToClient((uint8_t*)"R", 1);
						esp.getFileMode=1;
						//memset(esp.rx_buffer, 0x00, esp.rx_index); esp.rx_index=0;
					}else if(strstr((const char*)esp.rx_buffer, "setServer=")){
						memcpy(esp.param.server, &esp.rx_buffer[dataPos+10], size-10);
						setFlashParam(esp.param.server, FADDR_SERVER, strlen((char*)esp.param.server));
						printf("setServer.. %d\r\n", size);
					}else if(strstr((const char*)esp.rx_buffer, "setPort=")){
						memcpy(esp.param.port, &esp.rx_buffer[dataPos+8], size-8);
						setFlashParam(esp.param.port, FADDR_PORT, strlen((char*)esp.param.port));
						printf("setPort..\r\n");
					}else if(strstr((const char*)esp.rx_buffer, "setAPN=")){
						memcpy(esp.param.apName, &esp.rx_buffer[dataPos+7], size-7);
						setFlashParam(esp.param.apName, FADDR_AP_NAME, strlen((char*)esp.param.apName));
						printf("setAPN..\r\n");
					}else if(strstr((const char*)esp.rx_buffer, "setAPP=")){
						memcpy(esp.param.apPass, &esp.rx_buffer[dataPos+7], size-7);
						setFlashParam(esp.param.apPass, FADDR_AP_PASS, strlen((char*)esp.param.apPass));
						printf("setAPP..\r\n");
					}
					memset(esp.rx_buffer, 0x00, esp.rx_index); esp.rx_index=0;
				}else{
					uint16_t sum=0;
					//uint16_t size=getDataSize(esp.rx_buffer);
					if(strstr((const char*)esp.rx_buffer, "finish_transmit")){
						printf("file recived\r\n");
						esp.getFileMode=0;
						packetNum=0;
						packetNumPre=0xffff;
						shiftAddr=0;
						memset(esp.rx_buffer, 0x00, esp.rx_index); esp.rx_index=0;
					}else{
						uint8_t *point=(uint8_t *)strstr((const char*)esp.rx_buffer, ":");
						if(point++!=NULL){
							printf("data#%d: ", packetNum);
							for(uint16_t i=0; i<size-2; i++){
								sum+=point[i];
								if(i<32) printf("0x%x ", point[i]);
							}
							printf("\t[0x%x]\r\n", sum);
						}else{
							printf("---\r\n");
						}
						point+=(size-2);
						uint16_t sum_from=0;
						memcpy(&sum_from, point, 2);
						memcpy(&packetNum, point-2,2);
						//printf("packetNum: %d/%d\r\n", packetNum, (uint16_t)(packetNumPre+1));
						if(sum==sum_from && packetNum==(uint16_t)(packetNumPre+1)){
							point=(uint8_t *)strstr((const char*)esp.rx_buffer, ":");
							//uint32_t _size=getDataSize(esp.rx_buffer)-4;
							shiftAddr+=1024;
							esp.timeoutRX=0;
							packetNumPre++;
							sendToClient((uint8_t*)"O", 1);
						}else{
							printf("sum is bad 0x%x\r\n", sum_from);
							delay_ms(20);
							sendToClient((uint8_t*)"B", 1);
						}
						memset(esp.rx_buffer, 0x00, esp.rx_index); esp.rx_index=0;	
					}			
				}

			}else{
				memset(esp.rx_buffer, 0x00, esp.rx_index); esp.rx_index=0;				
			}
		}
	}
}

// функции заглушки. Убираем не нужные варнинги при компиляции..
void _fstat_r() {}
void _close_r() {}
void _isatty_r(){}
void _lseek_r() {}
void _read_r()  {}
