#ifndef SERIAL_COMMS_H
#define SERIAL_COMMS_H

void Uart1Init(void);
void Uart1Post(uint8_t *buf,int len);
void Uart1Send(uint8_t *buf,int len);
uint8_t Uart1ReadRecv(uint8_t *buf);

void Uart2Init(void);
void Uart2Post(uint8_t *buf,int len);
void Uart2Send(uint8_t *buf,int len);
uint8_t Uart2ReadRecv(uint8_t *buf);

void Uart3Init(void);
void Uart3Post(uint8_t *buf,int len);
void Uart3Send(uint8_t *buf,int len);
uint8_t Uart3ReadRecv(uint8_t *buf);

void UartTimerInit(void);

#endif

