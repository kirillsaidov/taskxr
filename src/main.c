#include "main.h"
#include "cronjob.h"

vt_mallocator_t *alloctr = NULL;
int32_t main(const int argc, const char* argv[]) {
    // check number of arguments
    if (argc < 2) {
        printf(">> No commands specified! See 'help' for more information.\n");
        return 0;
    }

    // create alloctr
    alloctr = vt_mallocator_create();

    // check and setup environment (.taskxr/jobs.cfg, .taskxr/jobs.lock)
    const char *const taskxr_dir = vt_str_z(vt_path_expand_tilda("~/.taskxr", alloctr));
    const char *const taskxr_jobs_file = vt_str_z(vt_path_expand_tilda("~/.taskxr/jobs.cfg", alloctr));
    const char *const taskxr_lock_file = vt_str_z(vt_path_expand_tilda("~/.taskxr/jobs.lock", alloctr));
    if (!vt_path_exists(taskxr_dir)) vt_path_mkdir(taskxr_dir);
    if (!vt_path_exists(taskxr_jobs_file)) vt_file_write(taskxr_jobs_file, "");
    if (!vt_path_exists(taskxr_lock_file)) vt_file_write(taskxr_lock_file, "");

    // parse arguments
    const char *const command = argv[1];
    const char *const jobs_file = argc > 2 ? argv[2] : taskxr_jobs_file;
    if (vt_str_equals_z(command, "serve")) {
        cj_loop_detached(jobs_file, taskxr_lock_file); // launch daemon process
    } else if (vt_str_equals_z(command, "stop")) {
        printf("stop: unimplemented, WIP\n");
        // cj_lock_remove_job(taskxr_lock_file, jobs_file);
    } else if (vt_str_equals_z(command, "list")) {
        cj_lock_list_jobs(taskxr_lock_file);
    } else if (vt_str_equals_z(command, "validate")) {
        printf(">> Validating jobs file: %s\n", jobs_file);
        if (!cj_validate_cron_file(jobs_file)) printf(">> Validation failed!\n");
        else printf(">> All good.\n");
    } else {
        printf("%s\n", PROJECT_HELP_MESSAGE);
    }

    vt_mallocator_destroy(alloctr);
    return 0;
}
