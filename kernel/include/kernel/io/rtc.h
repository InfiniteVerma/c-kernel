#ifndef __RTC__
#define __RTC__

#include <stdint.h>

#define CMOS_CONTROL_REG 0x70
#define CMOS_DATA_REG 0x71

#define SECONDS_REG 0x00
#define MINUTES_REG 0x02
#define HOURS_REG 0x04
#define WEEK_DAY_REG 0x06
#define DAY_OF_MONTH_REG 0x07
#define MONTH_REG 0x08
#define YEAR_REG 0x09
#define CENTURY_REG 0x32
#define STATUS_REG_A 0x0a
#define STATUS_REG_B 0x0b

#define RTC_FREQ 256

typedef struct DateTime (*F)(struct DateTime);

struct DateTime {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t week_day;
    uint8_t day_of_month;
    uint8_t month;
    uint8_t year;
    uint8_t century;
};

void print_date_time(struct DateTime);

struct DateTime get_date_time();
void set_date_time(struct DateTime);
void configure_rtc();
void register_rtc_driver();

int get_timestamp(char*);

void run_rtc_tests();

#endif /* __RTC__ */
