#ifndef FLASH_H
#define FLASH_H

#define FLASH_PAGE_SIZE                 0x00000800U    // FLASH Page Size, 2 KBytes //
#define FLASH_KEY1                      0x45670123U   //Flash key1 //
#define FLASH_KEY2                      0xCDEF89ABU   // Flash key2: used with FLASH_KEY1

void FLASH_Unlock(void);
void FLASH_Lock(void);
void FLASH_Program_DoubleWord(uint32_t Address, uint8_t *Data);
void FLASH_PageErase(uint32_t addr);

#endif
