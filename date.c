#include "date.h"

#include "sys.h"

char *itoa(unsigned val);
char *stpcpy(char *dst, const char *src);

struct year
{
    unsigned year;
    unsigned first_day_ts;
};

struct date
{
    const char *month;
    unsigned day;
};

static int is_leap_year(unsigned y)
{
    if (y % 4 != 0)   return 0;
    if (y % 100 != 0) return 1;
    if (y % 400 == 0) return 1;
    return 0;
}

static struct year get_year(time_t tv_sec)
{
    struct year year;
    unsigned days = 0, old_days;
    year.year = 1970;
    for (;; ++year.year) {
        old_days = days;
        days += 365 + is_leap_year(year.year);
        if (days * 24 * 3600 > tv_sec) {
            year.first_day_ts = old_days * 24 * 3600;
            return year;
        }
    }
}

static struct date get_date(unsigned day_of_year, int leap_year)
{
    static const char *months[] = { "Jan", "Feb", "Mar", "Apr",
                                    "May", "Jun", "Jul", "Aug",
                                    "Sep", "Oct", "Nov", "Dec" };
    static unsigned month_len[] = { 31, 28, 31, 30, 31, 30,
                                    31, 31, 30, 31, 30, 31 };
    struct date date;
    unsigned day = 0, month = 0, old_day;
    month_len[1] = 28 + leap_year;
    for (; month < 12; ++month) {
        old_day = day;
        day += month_len[month];
        if (day > day_of_year) {
            break;
        }
    }
    date.month = months[month];
    date.day = day_of_year - old_day + 1;
    return date;
}

static const char *get_rfc(time_t tv_sec)
{
    static const char *day[] = { "Thu", "Fri", "Sat", "Sun",
                                 "Mon", "Tue", "Wed" };
    struct year year = get_year(tv_sec);
    unsigned day_of_year = (tv_sec - year.first_day_ts) / 24 / 3600;
    struct date date = get_date(day_of_year, is_leap_year(year.year));

    static char buffer[32];
    char *ptr = buffer;
    ptr = stpcpy(ptr, day[tv_sec / 3600 / 24 % 7]);
    ptr = stpcpy(ptr, ", ");
    ptr = stpcpy(ptr, itoa(date.day));
    ptr = stpcpy(ptr, " ");
    ptr = stpcpy(ptr, date.month);
    ptr = stpcpy(ptr, " ");
    ptr = stpcpy(ptr, itoa(year.year));
    ptr = stpcpy(ptr, " ");
    ptr = stpcpy(ptr, itoa(tv_sec / 3600 % 24));
    ptr = stpcpy(ptr, ":");
    ptr = stpcpy(ptr, itoa(tv_sec / 60 % 60));
    ptr = stpcpy(ptr, ":");
    ptr = stpcpy(ptr, itoa(tv_sec % 60));
    ptr = stpcpy(ptr, " GMT");
    return buffer;
}

const char *get_rfc_now(void)
{
    struct timeval tv;
    sys_gettimeofday(&tv, NULL);
    return get_rfc(tv.tv_sec);
}
