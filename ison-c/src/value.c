#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ison.h"

static char *strdup_safe(const char *str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char *copy = malloc(len + 1);
    if (copy) memcpy(copy, str, len + 1);
    return copy;
}

ison_value_t ison_null(void) {
    ison_value_t v;
    v.type = ISON_TYPE_NULL;
    return v;
}

ison_value_t ison_bool(bool value) {
    ison_value_t v;
    v.type = ISON_TYPE_BOOL;
    v.data.bool_val = value;
    return v;
}

ison_value_t ison_int(int64_t value) {
    ison_value_t v;
    v.type = ISON_TYPE_INT;
    v.data.int_val = value;
    return v;
}

ison_value_t ison_float(double value) {
    ison_value_t v;
    v.type = ISON_TYPE_FLOAT;
    v.data.float_val = value;
    return v;
}

ison_value_t ison_string(const char *value) {
    return ison_string_n(value, value ? strlen(value) : 0);
}

ison_value_t ison_string_n(const char *value, size_t len) {
    ison_value_t v;
    v.type = ISON_TYPE_STRING;
    if (value) {
        v.data.string_val = malloc(len + 1);
        if (v.data.string_val) {
            memcpy(v.data.string_val, value, len);
            v.data.string_val[len] = '\0';
        }
    } else {
        v.data.string_val = NULL;
    }
    return v;
}

ison_value_t ison_ref(const ison_reference_t *ref) {
    ison_value_t v;
    v.type = ISON_TYPE_REFERENCE;
    if (ref) {
        v.data.ref_val.id = strdup_safe(ref->id);
        v.data.ref_val.ns = strdup_safe(ref->ns);
        v.data.ref_val.relationship = strdup_safe(ref->relationship);
    } else {
        v.data.ref_val.id = NULL;
        v.data.ref_val.ns = NULL;
        v.data.ref_val.relationship = NULL;
    }
    return v;
}

bool ison_value_is_null(const ison_value_t *value) {
    return value && value->type == ISON_TYPE_NULL;
}

bool ison_value_as_bool(const ison_value_t *value, bool *out) {
    if (!value || value->type != ISON_TYPE_BOOL) return false;
    if (out) *out = value->data.bool_val;
    return true;
}

bool ison_value_as_int(const ison_value_t *value, int64_t *out) {
    if (!value || value->type != ISON_TYPE_INT) return false;
    if (out) *out = value->data.int_val;
    return true;
}

bool ison_value_as_float(const ison_value_t *value, double *out) {
    if (!value) return false;
    if (value->type == ISON_TYPE_FLOAT) {
        if (out) *out = value->data.float_val;
        return true;
    }
    if (value->type == ISON_TYPE_INT) {
        if (out) *out = (double)value->data.int_val;
        return true;
    }
    return false;
}

bool ison_value_as_string(const ison_value_t *value, const char **out) {
    if (!value || value->type != ISON_TYPE_STRING) return false;
    if (out) *out = value->data.string_val;
    return true;
}

bool ison_value_as_ref(const ison_value_t *value, ison_reference_t *out) {
    if (!value || value->type != ISON_TYPE_REFERENCE) return false;
    if (out) *out = value->data.ref_val;
    return true;
}

char *ison_value_to_ison(const ison_value_t *value) {
    if (!value) return strdup_safe("~");
    
    char buf[256];
    switch (value->type) {
        case ISON_TYPE_NULL:
            return strdup_safe("~");
        case ISON_TYPE_BOOL:
            return strdup_safe(value->data.bool_val ? "true" : "false");
        case ISON_TYPE_INT:
            snprintf(buf, sizeof(buf), "%ld", (long)value->data.int_val);
            return strdup_safe(buf);
        case ISON_TYPE_FLOAT:
            snprintf(buf, sizeof(buf), "%g", value->data.float_val);
            return strdup_safe(buf);
        case ISON_TYPE_STRING: {
            const char *str = value->data.string_val;
            if (!str) return strdup_safe("~");
            int needs_quotes = !*str || strchr(str, ' ') || strchr(str, '\t') || 
                              strchr(str, '\n') || strchr(str, '"');
            if (!needs_quotes) return strdup_safe(str);
            
            size_t len = strlen(str);
            size_t extra = 2;
            for (size_t i = 0; i < len; i++) {
                if (str[i] == '\\' || str[i] == '"' || str[i] == '\n' || str[i] == '\t') extra++;
            }
            char *result = malloc(len + extra + 1);
            if (!result) return NULL;
            
            char *p = result;
            *p++ = '"';
            for (size_t i = 0; i < len; i++) {
                switch (str[i]) {
                    case '\\': *p++ = '\\'; *p++ = '\\'; break;
                    case '"': *p++ = '\\'; *p++ = '"'; break;
                    case '\n': *p++ = '\\'; *p++ = 'n'; break;
                    case '\t': *p++ = '\\'; *p++ = 't'; break;
                    default: *p++ = str[i];
                }
            }
            *p++ = '"';
            *p = '\0';
            return result;
        }
        case ISON_TYPE_REFERENCE:
            return ison_reference_to_ison(&value->data.ref_val);
        default:
            return strdup_safe("~");
    }
}

char *ison_value_to_json(const ison_value_t *value) {
    if (!value) return strdup_safe("null");
    
    char buf[256];
    switch (value->type) {
        case ISON_TYPE_NULL:
            return strdup_safe("null");
        case ISON_TYPE_BOOL:
            return strdup_safe(value->data.bool_val ? "true" : "false");
        case ISON_TYPE_INT:
            snprintf(buf, sizeof(buf), "%ld", (long)value->data.int_val);
            return strdup_safe(buf);
        case ISON_TYPE_FLOAT:
            snprintf(buf, sizeof(buf), "%g", value->data.float_val);
            return strdup_safe(buf);
        case ISON_TYPE_STRING: {
            const char *str = value->data.string_val;
            if (!str) return strdup_safe("null");
            size_t len = strlen(str);
            size_t extra = 2;
            for (size_t i = 0; i < len; i++) {
                if (str[i] == '\\' || str[i] == '"' || str[i] < 0x20) extra++;
            }
            char *result = malloc(len + extra + 1);
            if (!result) return NULL;
            
            char *p = result;
            *p++ = '"';
            for (size_t i = 0; i < len; i++) {
                switch (str[i]) {
                    case '\\': *p++ = '\\'; *p++ = '\\'; break;
                    case '"': *p++ = '\\'; *p++ = '"'; break;
                    case '\n': *p++ = '\\'; *p++ = 'n'; break;
                    case '\r': *p++ = '\\'; *p++ = 'r'; break;
                    case '\t': *p++ = '\\'; *p++ = 't'; break;
                    default:
                        if (str[i] < 0x20) {
                            sprintf(p, "\\u%04x", (unsigned char)str[i]);
                            p += 6;
                        } else {
                            *p++ = str[i];
                        }
                }
            }
            *p++ = '"';
            *p = '\0';
            return result;
        }
        case ISON_TYPE_REFERENCE: {
            char *ref_ison = ison_reference_to_ison(&value->data.ref_val);
            size_t len = strlen(ref_ison);
            char *result = malloc(len + 3);
            if (result) {
                result[0] = '"';
                memcpy(result + 1, ref_ison, len);
                result[len + 1] = '"';
                result[len + 2] = '\0';
            }
            free(ref_ison);
            return result;
        }
        default:
            return strdup_safe("null");
    }
}

void ison_value_free(ison_value_t *value) {
    if (!value) return;
    switch (value->type) {
        case ISON_TYPE_STRING:
            free(value->data.string_val);
            value->data.string_val = NULL;
            break;
        case ISON_TYPE_REFERENCE:
            ison_reference_free(&value->data.ref_val);
            break;
        default:
            break;
    }
}
