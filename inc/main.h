#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "vita/vita.h"

// project description
#define PROJECT_NAME "taskxr"
#define PROJECT_VERSION "0.1.0"
#define PROJECT_HELP_MESSAGE "" \
PROJECT_NAME " v" PROJECT_VERSION " -- a simple cron-like utility to schedule tasks with multiple job files.\n" \
"\nUSAGE: " PROJECT_NAME " [command] <jobs.cfg>\n" \
"\nCOMMANDS: \n" \
"   serve   launch taskxr in background with custom jobs.cfg\n" \
"    stop   stop taskxr daemon identified with custom jobs.cfg\n" \
"validate   validate jobs.cfg format\n" \
"    help   This help message.\n" \
"\nNOTE: \n" \
"   By default `~/.taskxr/jobs.cfg` is used, unless a custom one is specified.\n"

// shared alloctr
extern vt_mallocator_t *alloctr;

