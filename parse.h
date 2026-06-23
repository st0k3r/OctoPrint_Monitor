#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool    loaded;
    char    state[32];
    float   tool_actual;
    float   tool_target;
    float   bed_actual;
    float   bed_target;
    char    file_name[64];
    float   progress;       /* 0..100, or -1 if no job */
    int32_t time_left_s;    /* seconds remaining, or -1 if unknown */
    char    version[16];    /* OctoPrint server version string */
} PrinterStatus;

void parse_version_json(const char* json, PrinterStatus* s);
void parse_printer_json(const char* json, PrinterStatus* s);
void parse_job_json(const char* json, PrinterStatus* s);
