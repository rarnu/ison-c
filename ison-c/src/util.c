#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ison.h"

const char *ison_error_string(ison_error_t error) {
    switch (error) {
        case ISON_OK: return "OK";
        case ISON_ERROR_MEMORY: return "Memory allocation failed";
        case ISON_ERROR_PARSE: return "Parse error";
        case ISON_ERROR_IO: return "I/O error";
        case ISON_ERROR_INVALID: return "Invalid argument";
        default: return "Unknown error";
    }
}
