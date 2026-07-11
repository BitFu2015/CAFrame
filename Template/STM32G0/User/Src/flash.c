#include "driver.h"

void FLASH_Unlock(void)
{
    WRITE_REG(FLASH->KEYR, FLASH_KEY1);
    WRITE_REG(FLASH->KEYR, FLASH_KEY2);
}

void FLASH_Lock(void)
{
    SET_BIT(FLASH->CR, FLASH_CR_LOCK);
}

void FLASH_Program_DoubleWord(uint32_t Address, uint8_t *Data)
{
    uint32_t d1;

    while ((FLASH->SR & FLASH_SR_BSY1) != 0x00U)
    {
    }

    SET_BIT(FLASH->CR, FLASH_CR_PG);

   
    d1 = *(uint32_t *)Data;
   
    *(uint32_t *)Address = d1;

    __ISB();
   
    Data += 4;

    d1 = *(uint32_t *)Data;

    *(uint32_t *)(Address + 4U) = d1;

    while ((FLASH->SR & FLASH_SR_BSY1) != 0x00U)
    {
    }

    CLEAR_BIT(FLASH->CR, FLASH_CR_PG);
}
/*
void EarseSector(int sector)
{
    uint32_t tmp;
   
    FLASH_Unlock();
   
    //判断flash 是否忙碌
    while ((FLASH->SR & FLASH_SR_BSY1) != 0x00U)
    {
    }
   
    tmp = (FLASH->CR & ~FLASH_CR_PNB);
   
    FLASH->CR = (tmp | (FLASH_CR_STRT | (sector <<  FLASH_CR_PNB_Pos) | FLASH_CR_PER));
   
    while ((FLASH->SR & FLASH_SR_BSY1) != 0x00U)
    {
    }
   
    CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
   
    FLASH_Lock();
}

// Gets the page of a given address 
uint32_t ulGetPage(uint32_t startAddr)
{
    return ((startAddr - FLASH_BASE) / FLASH_PAGE_SIZE);
}*/

// Erase the specified FLASH memory page 
void FLASH_PageErase(uint32_t addr)
{
    uint32_t tmp  = 0;
		uint32_t page=((addr - FLASH_BASE) / FLASH_PAGE_SIZE);//地址转换成页

    // Get configuration register, then clear page number 
    tmp = (FLASH->CR & ~FLASH_CR_PNB);

    // Set page number, Page Erase bit & Start bit 
    FLASH->CR = (tmp | (FLASH_CR_STRT | (page <<  FLASH_CR_PNB_Pos) | FLASH_CR_PER));

    //wait for BSY1 in order to be sure that flash operation is ended before allowing prefetch in flash 
    while ((FLASH->SR & FLASH_SR_BSY1) != 0x00U)
    {
    }

    /* If operation is completed or interrupted, disable the Page Erase Bit */
    CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
}
