#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include "stm32f1xx_hal.h"
#define SECONDS_FROM_1970_TO_2000 946684800

const uint8_t daysInMonth [] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static uint32_t seconds;

static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d)
{
    uint8_t i;
    if (y >= 2000)
        y -= 2000;
    uint16_t days = d;
    for (i = 1; i < m; ++i)
        days +=  daysInMonth[i - 1];
    if (m > 2 && y % 4 == 0)
        ++days;
    return days + 365 * y + (y + 3) / 4 - 1;
}

static long time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s)
{
    return ((days * 24L + h) * 60 + m) * 60 + s;
}


uint32_t set_time(uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute, uint32_t second)
{
    struct tm time_struct;
    time_t m_time;
    time_struct.tm_year = year - 1900; //1900
    time_struct.tm_mon = month;
    time_struct.tm_mday = day;
    time_struct.tm_hour = hour;
    time_struct.tm_min = minute;
    time_struct.tm_sec = second;
    m_time = mktime(&time_struct);// Convert to timestamp
    return m_time;
}


struct tm *get_time(time_t Timestamp)
{
    return localtime(&Timestamp);
}

struct tm convert_time_stamp(uint32_t t)
{
    struct tm time_struct;
    uint32_t yOff, m, d, hh, mm, ss;

    t -= 946684800;//SECONDS_FROM_1970_TO_2000;    // bring to 2000 timestamp from 1970
    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for (yOff = 0; ; ++yOff)
    {
        leap = yOff % 4 == 0;
        if (days < 365 + leap)
            break;
        days -= 365 + leap;
    }
    for (m = 1; ; ++m)
    {
        uint8_t daysPerMonth = daysInMonth[m - 1];
        if (leap && m == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }
    d = days + 1;


    time_struct.tm_year = (yOff + 100);   
    time_struct.tm_mon = m - 1;
    time_struct.tm_mday = d;
    time_struct.tm_hour = hh;
    time_struct.tm_min = mm;
    time_struct.tm_sec = ss;
    mktime(&time_struct);
    return time_struct;
}

char *get_time_string(uint32_t Timestamp)
{
    static char cal_string[80];
    strftime(cal_string, 80, "%x - %H:%M:%S",get_time(Timestamp));
    return cal_string;
}

