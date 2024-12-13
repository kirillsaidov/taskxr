#pragma once

#include "main.h"

// consts
#define CJ_MAX_COMMAND_LENGTH 1024

// cron job object
struct CronJob {
    int32_t minute;
    int32_t hour;
    int32_t day_of_month;
    int32_t month;
    int32_t day_of_week;
    char command[CJ_MAX_COMMAND_LENGTH];
};

extern int32_t cj_parse_time(const char *const time_str);
extern struct CronJob cj_parse_cron_line(const char *const line);
extern vt_vec_t *cj_parse_cron_file(const char *const jobs_file);

extern bool cj_job_should_run(struct CronJob job);
extern void cj_loop(const vt_vec_t *const jobs);
extern void cj_loop_detached(const vt_vec_t *const jobs);
extern void cj_execute_command(const char *const command);

extern bool cj_validate_cron_line(const char *const line);
extern bool cj_validate_cron_file(const char *const jobs_file);

