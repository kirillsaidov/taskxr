#include "cronjob.h"

int32_t cj_parse_time(const char *const time_str) {
    if (strncmp(time_str, "*", CJ_MAX_COMMAND_LENGTH) == 0) {
        return -1;
    }
    return vt_conv_str_to_i32(time_str);
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

bool cj_validate_cron_file(const char *const file_path) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        printf(">> Jobs file does not exist: %s\n", file_path);
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