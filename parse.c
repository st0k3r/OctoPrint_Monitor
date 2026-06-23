#include "parse.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Returns pointer to the value part of "key": ... (whitespace after colon
 * already skipped).  Returns NULL if the key is not found. */
static const char* find_key(const char* json, const char* key) {
    char pattern[72];
    snprintf(pattern, sizeof(pattern), "\"%s\":", key);
    const char* p = strstr(json, pattern);
    if(!p) return NULL;
    p += strlen(pattern);
    while(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    return p;
}

static bool extract_str(const char* json, const char* key, char* out, size_t out_sz) {
    const char* p = find_key(json, key);
    if(!p || *p != '"') return false;
    p++;
    size_t i = 0;
    while(*p && *p != '"' && i < out_sz - 1) out[i++] = *p++;
    out[i] = '\0';
    return i > 0;
}

static bool extract_float(const char* json, const char* key, float* out) {
    const char* p = find_key(json, key);
    if(!p) return false;
    char* end;
    float v = strtof(p, &end);
    if(end == p) return false;
    *out = v;
    return true;
}

static bool extract_int(const char* json, const char* key, int32_t* out) {
    float f;
    if(!extract_float(json, key, &f)) return false;
    *out = (int32_t)f;
    return true;
}

void parse_version_json(const char* json, PrinterStatus* s) {
    extract_str(json, "server", s->version, sizeof(s->version));
}

void parse_printer_json(const char* json, PrinterStatus* s) {
    /* State is nested: "state": { "text": "Printing", ... } */
    const char* state_obj = find_key(json, "state");
    if(state_obj) extract_str(state_obj, "text", s->state, sizeof(s->state));

    /* Hotend temperature */
    const char* tool = find_key(json, "tool0");
    if(tool) {
        extract_float(tool, "actual", &s->tool_actual);
        extract_float(tool, "target", &s->tool_target);
    }

    /* Bed temperature — search from beginning to avoid matching inside tool0 */
    const char* bed = strstr(json, "\"bed\":");
    if(bed) {
        extract_float(bed, "actual", &s->bed_actual);
        extract_float(bed, "target", &s->bed_target);
    }
}

void parse_job_json(const char* json, PrinterStatus* s) {
    /* File name inside "file": { "name": "..." } */
    const char* file_obj = find_key(json, "file");
    if(file_obj) {
        char name[sizeof(s->file_name)];
        if(extract_str(file_obj, "name", name, sizeof(name)) &&
           strcmp(name, "null") != 0) {
            strncpy(s->file_name, name, sizeof(s->file_name) - 1);
            s->file_name[sizeof(s->file_name) - 1] = '\0';
        }
    }

    /* Progress inside "progress": { "completion": N, "printTimeLeft": N } */
    const char* prog_obj = find_key(json, "progress");
    if(prog_obj) {
        float completion = -1.0f;
        if(extract_float(prog_obj, "completion", &completion) && completion >= 0.0f)
            s->progress = completion;

        int32_t time_left = -1;
        if(extract_int(prog_obj, "printTimeLeft", &time_left) && time_left > 0)
            s->time_left_s = time_left;
    }
}
