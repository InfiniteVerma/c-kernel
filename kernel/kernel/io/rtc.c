#include <kernel/io/rtc.h>
#include <stdio.h>

#define NMI_disable_bit 1
#define selected_cmos_register_number 0x0F

static void outb(uint16_t port, uint8_t value) {
    asm volatile (
        "outb %0, %1"
        :  // No output operands
        : "a"(value), "Nd"(port)
    );
}

static uint8_t inb(uint16_t port) {
    uint8_t value;
    asm volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );
    return value;
}

static void disable_cmos_nmi() {
    uint8_t value = (NMI_disable_bit << 7) | selected_cmos_register_number;
    outb(CMOS_CONTROL_REG, value);
}

static void select_cmos_register(uint8_t register_num) {
    outb(CMOS_CONTROL_REG, register_num);
}

static uint8_t read_cmos_register(uint8_t register_num) {
    select_cmos_register(register_num);
    return inb(CMOS_DATA_REG);
}

static void write_cmos_register(uint8_t register_num, uint8_t val) {
    select_cmos_register(register_num);
    outb(CMOS_DATA_REG, val);
}

static int is_update_in_progress() {
    uint8_t ret = read_cmos_register(STATUS_REG_A);
    return ((ret >> 7) & 1) == 1;
}

static struct DateTime guarded_op(F f, struct DateTime d) {
    while(1) {
        while(is_update_in_progress())  {
            continue;
        }

        d = f(d);

        while(is_update_in_progress())  {
            continue;
        }
        break;
    }
    return d;
}

static struct DateTime read(struct DateTime d) {
    struct DateTime date_time;
    date_time.seconds = read_cmos_register(SECONDS_REG);
    date_time.minutes = read_cmos_register(MINUTES_REG);
    date_time.hours = read_cmos_register(HOURS_REG);
    date_time.week_day = read_cmos_register(WEEK_DAY_REG);
    date_time.day_of_month = read_cmos_register(DAY_OF_MONTH_REG);
    date_time.month = read_cmos_register(MONTH_REG);
    date_time.year = read_cmos_register(YEAR_REG);
    date_time.century = read_cmos_register(CENTURY_REG);
    return date_time;
}

static struct DateTime write(struct DateTime d) {
    struct DateTime dummy;
    write_cmos_register(SECONDS_REG, d.seconds);
    write_cmos_register(MINUTES_REG, d.minutes);
    write_cmos_register(HOURS_REG, d.hours);
    write_cmos_register(WEEK_DAY_REG, d.week_day);
    write_cmos_register(DAY_OF_MONTH_REG, d.day_of_month);
    write_cmos_register(MONTH_REG, d.month);
    write_cmos_register(YEAR_REG, d.year);
    write_cmos_register(CENTURY_REG, d.century);
    return dummy;
}

struct DateTime get_date_time() {
    struct DateTime dummy;
    return guarded_op(read, dummy); // passing dummy param
}

void set_date_time(struct DateTime date_time) {
    guarded_op(write, date_time); // ignoring ret val
}

void configure_rtc() {
    read_cmos_register(STATUS_REG_B);
    uint8_t flags = 0;
    flags |= (1 << 1); // Enabling 24 hour format
    flags |= (1 << 2); // Enable binary mode
    write_cmos_register(STATUS_REG_B, flags);
}

void print_date_time(struct DateTime d) {
    printf("DateTime { seconds: %d, minutes: %d, hours: %d, day_of_month: %d, month: %d, year: %d, century: %d }\n",
            d.seconds, d.minutes, d.hours, d.day_of_month, d.month, d.year, d.century);
    //printf("DateTime { seconds: %d, minutes: %d, hours: %d, week_day: %d, day_of_month: %d,\nmonth: %d, year: %d, century: %d }\n", FIXME: fix overflow issue
    //        d.seconds, d.minutes, d.hours, d.week_day, d.day_of_month, d.month, d.year, d.century);
}
