//iic.h
#ifndef IIC_H
#define IIC_H

#define IIC_ACK			1
#define IIC_NOT_ACK		0

void iic_delaybus(void);

void iic_init(void);
void iic_start(void);
void iic_stop(void);
uint8_t  iic_writebyte(uint8_t c);
uint8_t  iic_readbyte(uint8_t ack);

#endif
