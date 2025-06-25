#include <kernel/interrupts.h>
#include <kernel/io/rtc.h>
#include <kernel/monotonic_tick.h>
#include <stdio.h>
#include <utils.h>
#ifdef TEST
#include <unistd.h>
#endif

#define NMI_disable_bit 1
#define selected_cmos_register_number 0x0F

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
    while (1) {
        while (is_update_in_progress()) {
            continue;
        }

        d = f(d);

        while (is_update_in_progress()) {
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
    return guarded_op(read, dummy);  // passing dummy param
}

void set_date_time(struct DateTime date_time) {
    guarded_op(write, date_time);  // ignoring ret val
}

void configure_rtc() {
    read_cmos_register(STATUS_REG_B);
    uint8_t flags = 0;
    flags |= (1 << 1);  // Enabling 24 hour format
    flags |= (1 << 2);  // Enable binary mode
    write_cmos_register(STATUS_REG_B, flags);
}

void print_date_time(struct DateTime d) {
    LOG("DateTime { seconds: %d, minutes: %d, hours: %d, day_of_month: %d, month: %d, year: %d, "
        "century: %d }\n",
        d.seconds, d.minutes, d.hours, d.day_of_month, d.month, d.year, d.century);
}

static int get_timestamp_wrapper(char* buffer, int size, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size = vsnprintf(buffer, size, fmt, args);
    va_end(args);
    return size;
}

// [2025-05-30T07:30:28+0530]
int get_timestamp(char* buffer) {
    struct DateTime d = get_date_time();
    va_list args;
    return get_timestamp_wrapper(buffer, 100, "[%d%d-%d-%dT%d:%d:%d]  ", d.century, d.year, d.month,
                                 d.hours, d.minutes, d.seconds);
}

static void configure_rtc_interrupts() {
    disable_cmos_nmi();
    select_cmos_register(0x8B);
    char prev = inb(CMOS_DATA_REG);
    outb(CMOS_CONTROL_REG, 0x8B);
    outb(CMOS_DATA_REG, prev | 0x40);
    write_cmos_register(0xA, 1);  // set frequency to 256 Hz (MC146818)
}

void process_rtc_interrupt() {
    process_tick();
    outb(CMOS_CONTROL_REG, 0x0C);
    inb(CMOS_DATA_REG);
}

void register_rtc_driver() {
    INTERRUPT_GUARDED({
        configure_rtc_interrupts();
        register_interrupt(PIC_2_OFFSET, process_rtc_interrupt);
    });
}

#ifdef TEST
static void test_rtc_interrupts() {
    uint32_t prev_tick = get_tick();
    sleep(4);
    uint32_t now_tick = get_tick();
    uint8_t delta = 0;  // zero ticks error rate
    assert((now_tick - prev_tick) / 256 <= 4 + delta, "sleep test failed");
}

void run_rtc_tests() {
    test_rtc_interrupts();
    LOG_GREEN("RTC: [OK]");
}
#endif
