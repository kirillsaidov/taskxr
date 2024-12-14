#include "cronjob.h"

int32_t cj_parse_time(const char *const time_str) {
    if (strncmp(time_str, "*", CJ_MAX_COMMAND_LENGTH) == 0) {
        return -1;
    }
    return vt_conv_str_to_i32(time_str);
}

struct CronJob cj_parse_cron_line(const char *const line) {
    // parse
    char cron_parts[6][CJ_MAX_COMMAND_LENGTH] = {};
    sscanf(line, "%s %s %s %s %s %[^\n]", cron_parts[0], cron_parts[1], cron_parts[2], cron_parts[3], cron_parts[4], cron_parts[5]);

    // convert
    struct CronJob job = {
        .minute = cj_parse_time(cron_parts[0]),
        .hour = cj_parse_time(cron_parts[1]),
        .day_of_month = cj_parse_time(cron_parts[2]),
        .month = cj_parse_time(cron_parts[3]),
        .day_of_week = cj_parse_time(cron_parts[4]),
    };
    strncpy(job.command, cron_parts[5], CJ_MAX_COMMAND_LENGTH);
    job.command[CJ_MAX_COMMAND_LENGTH-1] = '\0'; // ensure null termination

    return job;
}

vt_vec_t *cj_parse_cron_file(const char *const jobs_file) {
    FILE *file = fopen(jobs_file, "r");
    if (!file) return NULL;

    // parse jobs
    char line[CJ_MAX_COMMAND_LENGTH];
    vt_vec_t *jobs = vt_vec_create(VT_ARRAY_DEFAULT_INIT_ELEMENTS, sizeof(struct CronJob), alloctr);
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || strlen(line) <= 1) {
            continue;
        }

        // validate
        if (cj_validate_cron_line(line)) {
            struct CronJob job = cj_parse_cron_line(line);
            vt_vec_push_back(jobs, &job);
        }
    }

    fclose(file);
    return jobs;
}

bool cj_job_should_run(struct CronJob job) {
    const struct VitaDateTime current_time = vt_datetime_get_now();
    return (job.minute == -1 || job.minute == current_time.minute) &&
        (job.hour == -1 || job.hour == current_time.hour) &&
        (job.day_of_month == -1 || job.day_of_month == current_time.month_day) &&
        (job.month == -1 || job.month == current_time.month + 1) &&
        (job.day_of_week == -1 || job.day_of_week == current_time.week_day);
}

void cj_loop(const vt_vec_t *const jobs) {
    while (true) {
        VT_FOREACH(i, 0, vt_vec_len(jobs)) {
            struct CronJob job = *(struct CronJob*)vt_vec_get(jobs, i);
            if (cj_job_should_run(job)) {
                // execute command in detached process
                cj_execute_command(job.command);
            }
        }

        // wait 60 seconds before checking again
        sleep(60);
    }
}

void cj_loop_detached(const char *const jobs_file, const char *const lock_file) {
    // parse jobs
    vt_vec_t *jobs = cj_parse_cron_file(jobs_file);
    if (!vt_vec_len(jobs)) {
        printf(">> No jobs found.\n");
        return;
    }

    // start child process
    pid_t pid = fork();
    if (pid == 0) {
        // detach child process and create a new session
        setsid();

        // add job to lock file
        cj_lock_add_job(lock_file, jobs_file, getpid());

        // run jobs loop
        cj_loop(jobs);
        exit(EXIT_SUCCESS); // exit child when done
    } else return;
}

void cj_execute_command(const char *const command) {
    // start child process
    pid_t pid = fork();
    if (pid == 0) {
        system(command);
        exit(EXIT_SUCCESS); // exit child process when done
    }
}

bool cj_validate_cron_line(const char *const line) {
    char temp_line[CJ_MAX_COMMAND_LENGTH];
    strncpy(temp_line, line, CJ_MAX_COMMAND_LENGTH);

    int32_t field_count = 0;
    char *token = strtok(temp_line, " ");
    while (token != NULL && field_count < 5) {
        if (!(strncmp(token, "*", CJ_MAX_COMMAND_LENGTH) == 0 || (isdigit(token[0]) && atoi(token) >= 0))) {
            return 0;
        }
        token = strtok(NULL, " ");
        field_count++;
    }

    // ensure 5 fields and a command are passed
    return field_count == 5 && token != NULL;
}

bool cj_validate_cron_file(const char *const jobs_file) {
    FILE *file = fopen(jobs_file, "r");
    if (!file) {
        printf(">> Jobs file does not exist: %s\n", jobs_file);
        return false;
    }

    bool is_valid = true;
    int32_t linenum = 1;
    char line[CJ_MAX_COMMAND_LENGTH];
    while (fgets(line, CJ_MAX_COMMAND_LENGTH, file)) {
        if (line[0] == '#' || strnlen(line, CJ_MAX_COMMAND_LENGTH) <= 1) {
            continue; // skip comments or empty lines
        }

        // strip control symbols
        vt_str_t _line = vt_str_create_static(line);
        vt_str_strip(&_line);

        // validate each line separately
        if (!cj_validate_cron_line(line)) {
            printf("--> %d | Invalid cron line: %s\n", linenum, line);
            is_valid = false;
            break;
        }

        // count cron line number
        linenum++;
    }

    fclose(file);
    return is_valid;
}

vt_vec_t *cj_lock_parse_jobs(const char *const lock_file) {
    FILE *file = fopen(lock_file, "r");
    if (!file) return NULL;

    // parse jobs
    char line[CJ_MAX_COMMAND_LENGTH];
    vt_vec_t *jobs = vt_vec_create(VT_ARRAY_DEFAULT_INIT_ELEMENTS, sizeof(struct LockedJob), alloctr);
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || strlen(line) <= 1) {
            continue;
        }

        // parse active jobs
        struct LockedJob job;
        sscanf(line, "%d %[^\n]", &job.pid, job.filepath);
        vt_vec_push_back(jobs, &job);
    }

    fclose(file);
    return jobs;
}

void cj_lock_list_jobs(const char *const lock_file) {
    vt_vec_t *jobs = cj_lock_parse_jobs(lock_file);
    if (!jobs) {
        printf(">> Failed to read jobs.lock file: %s\n", lock_file);
        return;
    } else if (!vt_vec_len(jobs)) {
        printf(">> No jobs running.\n");
        return;
    }

    // list active jobs
    printf("%5s\t%s\n", "PID", "Jobs file");
    VT_FOREACH(i, 0, vt_vec_len(jobs)) {
        const struct LockedJob job = *(struct LockedJob*)vt_vec_get(jobs, i);
        printf("%5d\t%s\n", job.pid, job.filepath);
    }
}

void cj_lock_add_job(const char *const lock_file, const char *const jobs_file, const pid_t pid) {
    // get absolute path of the jobs file
    char absolute_path[CJ_MAX_COMMAND_LENGTH];
    realpath(jobs_file, absolute_path);

    // append to lock file
    vt_file_writefc(lock_file, false, true, true, "%d %s", pid, absolute_path);
}

void cj_lock_remove_job(const char *const lock_file, const char *const jobs_file) {
    vt_vec_t *jobs = cj_lock_parse_jobs(lock_file);
    if (!jobs) {
        printf(">> Failed to read jobs.lock file: %s\n", lock_file);
        return;
    } else if (!vt_vec_len(jobs)) {
        printf(">> No jobs running.\n");
        return;
    }

    // parse pid
    const pid_t pid = vt_str_is_numeric_z(jobs_file, strnlen(jobs_file, 16))
        ? vt_conv_str_to_i32(jobs_file)
        : -1;    

    // remove lock old lock file
    vt_path_remove(lock_file);

    // remove jobs
    char buffer[CJ_MAX_COMMAND_LENGTH];
    VT_FOREACH(i, 0, vt_vec_len(jobs)) {
        const struct LockedJob job = *(struct LockedJob*)vt_vec_get(jobs, i);

        // check if job should be stopped
        const bool should_stop = pid > 0
            ? (job.pid == pid)
            : vt_str_equals_z(realpath(jobs_file, buffer), job.filepath);
        
        // stop job TODO: here
        // snprintf(buffer, sizeof(buffer), "kill -9 %d", pid);
        // if (should_stop && system(buffer) == 0) {
        //     printf(">> Job stopped: PID=%d, file=%s\n", job.pid, job.filepath);
        // } else if (!should_stop) {
        //     cj_lock_add_job(lock_file, job.filepath, job.pid);
        // } else {
        //     printf(">> Failed to stop the job: PID=%d, file=%s\n", job.pid, job.filepath);
        // }
    }
}

