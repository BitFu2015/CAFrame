//24cxx.h

#ifndef AT24CXX_H
#define AT24CXX_H

//AT24C32,AT24C64,AT24C128,AT24C256,AT24C512中选其一
#define AT24C256	   //此处定义器件型号

#if defined AT24C32 || defined AT24C64
	#define EEP_PAGE_SIZE 32
#elif defined AT24C128 || defined AT24C256 
	#define EEP_PAGE_SIZE 64
#elif defined AT24C512
	#define EEP_PAGE_SIZE 128
#else
	#warning("24cxx.h文件中器件型号未定义!")
#endif

void eep_read(uint16_t addr,uint8_t *buf,uint8_t len);

void eep_write(uint16_t addr,uint8_t *buf,uint8_t len);

void eep_write_page(uint16_t addr,uint8_t *buf); 

void eep_init(void);

#endif
