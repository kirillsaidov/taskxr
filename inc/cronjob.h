#pragma once

#include "main.h"

// consts
#define CJ_MAX_COMMAND_LENGTH 4024

// cron job object
struct CronJob {
    int32_t minute;
    int32_t hour;
    int32_t day_of_month;
    int32_t month;
    int32_t day_of_week;
    char command[CJ_MAX_COMMAND_LENGTH];
};

// locked job object (launched background jobs)
struct LockedJob {
    pid_t pid;
    char filepath[CJ_MAX_COMMAND_LENGTH];
};

extern int32_t cj_parse_time(const char *const time_str);
extern struct CronJob cj_parse_cron_line(const char *const line);
extern vt_vec_t *cj_parse_cron_file(const char *const jobs_file);

extern bool cj_job_should_run(struct CronJob job);
extern void cj_loop(const vt_vec_t *const jobs);
extern void cj_loop_detached(const char *const jobs_file, const char *const lock_file);
extern void cj_execute_command(const char *const command);

extern bool cj_validate_cron_line(const char *const line);
extern bool cj_validate_cron_file(const char *const jobs_file);

extern vt_vec_t *cj_lock_parse_jobs(const char *const lock_file);
extern void cj_lock_list_jobs(const char *const lock_file);
extern void cj_lock_add_job(const char *const lock_file, const char *const jobs_file, const pid_t pid);
extern void cj_lock_stop_job(const char *const lock_file, const char *const jobs_file);

