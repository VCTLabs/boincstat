#ifndef __defined_common_h
#define __defined_common_h

#include <time.h>

typedef struct wu
{
        char name[40];
        char app_name[30];
        char app_version[10];
        char actname[40];
        float final_cpu_time;
        float current_progress;
        float cpu_used;
        int state;
        int slot;
        time_t report_deadline;
        time_t start_time;
        float current_cpu;
        struct wu *next;
} workunit ;

void printWorkunit(workunit* currwu);

#endif
