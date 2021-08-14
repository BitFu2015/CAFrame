#ifndef ADC_H
#define ADC_H

void AdcInit(void);
void AdcStart(void);
void AdcStop(void);
uint16_t GetTemp(uint8_t ch);

#endif
