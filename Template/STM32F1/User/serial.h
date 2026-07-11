//serial.h
#ifndef SERIAL_COMMS_H
#define SERIAL_COMMS_H

#define SERIAL_BUFF_SIZE 1024

void Uart1Init(uint32_t ulWantedBaud);
int Uart1Recv(uint8_t *buf);
uint8_t Uart1Send(uint8_t *buf,int len);
void Uart1Post(uint8_t *buf,int len);

void Uart2Init(uint32_t ulWantedBaud);
int Uart2Recv(uint8_t *buf);
uint8_t Uart2Send(uint8_t *buf,int len);
void Uart2Post(uint8_t *buf,int len);

void Uart3Init(uint32_t ulWantedBaud);
int Uart3Recv(uint8_t *buf);
uint8_t Uart3Send(uint8_t *buf,int len);
void Uart3Post(uint8_t *buf,int len);

void SerialTimerInit(void);


uint8_t drv_serial_input(uint8_t *buf);
void drv_serial_output(uint8_t *buf,uint8_t size);

#endif

