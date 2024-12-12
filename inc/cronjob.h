#pragma once

#include "main.h"

// consts
#define CJ_MAX_COMMAND_LENGTH 1024

// cron job object
typedef struct {
    int32_t minute;
    int32_t hour;
    int32_t day_of_month;
    int32_t month;
    int32_t day_of_week;
    char command[CJ_MAX_COMMAND_LENGTH];
} CronJob;

extern int32_t cj_parse_time(const char *const time_str);
extern bool cj_validate_cron_line(const char *const line);
extern bool cj_validate_cron_file(const char *const file_path);

