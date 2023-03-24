/* rtc.h - Defines used in interactions with the RTC
 * controller
 * vim:ts=4 noexpandtab
 */

#ifndef _RTC_H
#define _RTC_H

 /* Defined variables as used in the PIC functions */
#define BIT_SIX_ON          0x40
#define RTC_IRQ             0x08
#define MINFREQ             0x02
#define MAXFREQ             1024
#define RATEBITS            0xF0


/* Ports that RTC/CMOS sits on */
#define RTC_PORT_IDX        0x70
#define RTC_PORT_RW         0x71
#define REG_A               0x8A
#define REG_B               0x8B
#define REG_C               0x8C
#define REG_D               0x8D

#define RATE_1024           0x06

/* Possible rates corresponding to valid frequencies: Looked at the Datasheet for these values. */
#define RATE_FOR_1024       0   
#define RATE_FOR_512        1   
#define RATE_FOR_256        2    
#define RATE_FOR_128        4    
#define RATE_FOR_64         8    
#define RATE_FOR_32         16   
#define RATE_FOR_16         32    
#define RATE_FOR_8          64    
#define RATE_FOR_4          128   
#define RATE_FOR_2          256   
#define OPEN_AT_2HZ         2
#define WARNING_SUPPRESS    0xFFFF


volatile int interrupt_flag_rtc; /* 1 == interrupt active. 0 == interrupt not active */


/* Externally-visible functions */

/* Initialize the RTC */
void rtc_init(void);
/* RTC's Interrupt Handler */
extern void rtc_handler(void);
/* Change frequency of RTC */
int rtc_set_freq(int newfreq);

/* Functions for RTC driver - open / close / r / w */

/* initializes RTC frequency to 2HZ, return 0 */
int32_t open_rtc(const uint8_t *filename,int32_t fd);
/* probably does nothing, unless you virtualize RTC, return 0 */
int32_t close_rtc(int32_t fd);
/* should block until the next interrupt, return 0 */
int32_t read_rtc(int32_t fd, void* buf, int32_t nbytes);
/* must be able to change frequency, return 0 or -1 */
int32_t write_rtc(int32_t fd, const void* buf, int32_t nbytes);

#endif /* _RTC_H */

