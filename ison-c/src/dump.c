#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ison.h"

static char *strdup_safe(const char *str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char *copy = malloc(len + 1);
    if (copy) memcpy(copy, str, len + 1);
    return copy;
}

static void append_string(char **buf, size_t *len, size_t *cap, const char *str) {
    if (!str) return;
    size_t str_len = strlen(str);
    if (*len + str_len + 1 > *cap) {
        *cap = (*cap + str_len + 1) * 2;
        *buf = realloc(*buf, *cap);
    }
    memcpy(*buf + *len, str, str_len);
    *len += str_len;
    (*buf)[*len] = '\0';
}

static void append_char(char **buf, size_t *len, size_t *cap, char ch) {
    if (*len + 2 > *cap) {
        *cap = *cap * 2;
        *buf = realloc(*buf, *cap);
    }
    (*buf)[*len] = ch;
    (*len)++;
    (*buf)[*len] = '\0';
}

char *ison_dumps_with_options(const ison_document_t *doc, const ison_dumps_options_t *opts) {
    if (!doc) return strdup_safe("");
    
    const char *delim = opts && opts->delimiter ? opts->delimiter : " ";
    size_t len = 0, cap = 1024;
    char *result = malloc(cap);
    if (!result) return NULL;
    *result = '\0';
    
    for (size_t i = 0; i < doc->order_count; i++) {
        if (i > 0) append_char(&result, &len, &cap, '\n');
        
        ison_block_t *block = ison_document_get(doc, doc->order[i]);
        if (!block) continue;
        
        char header[256];
        snprintf(header, sizeof(header), "%s.%s\n", block->kind, block->name);
        append_string(&result, &len, &cap, header);
        
        for (size_t j = 0; j < block->field_count; j++) {
            if (j > 0) append_string(&result, &len, &cap, delim);
            if (block->fields[j].type_hint && *block->fields[j].type_hint) {
                char field[256];
                snprintf(field, sizeof(field), "%s:%s", block->fields[j].name, block->fields[j].type_hint);
                append_string(&result, &len, &cap, field);
            } else {
                append_string(&result, &len, &cap, block->fields[j].name);
            }
        }
        append_char(&result, &len, &cap, '\n');
        
        for (size_t r = 0; r < block->row_count; r++) {
            ison_row_t *row = block->rows[r];
            for (size_t j = 0; j < block->field_count; j++) {
                if (j > 0) append_string(&result, &len, &cap, delim);
                ison_value_t *val = ison_row_get_ptr(row, block->fields[j].name);
                if (val) {
                    char *str = ison_value_to_ison(val);
                    append_string(&result, &len, &cap, str);
                    free(str);
                } else {
                    append_char(&result, &len, &cap, '~');
                }
            }
            append_char(&result, &len, &cap, '\n');
        }
        
        if (block->summary_row) {
            append_string(&result, &len, &cap, "---\n");
            for (size_t j = 0; j < block->field_count; j++) {
                if (j > 0) append_string(&result, &len, &cap, delim);
                ison_value_t *val = ison_row_get_ptr(block->summary_row, block->fields[j].name);
                if (val) {
                    char *str = ison_value_to_ison(val);
                    append_string(&result, &len, &cap, str);
                    free(str);
                } else {
                    append_char(&result, &len, &cap, '~');
                }
            }
            append_char(&result, &len, &cap, '\n');
        }
    }
    
    return result;
}

char *ison_dumps(const ison_document_t *doc) {
    return ison_dumps_with_options(doc, NULL);
}

char *ison_dumps_isonl(const ison_document_t *doc) {
    if (!doc) return strdup_safe("");
    
    size_t len = 0, cap = 1024;
    char *result = malloc(cap);
    if (!result) return NULL;
    *result = '\0';
    
    for (size_t i = 0; i < doc->order_count; i++) {
        ison_block_t *block = ison_document_get(doc, doc->order[i]);
        if (!block) continue;
        
        char header[256];
        snprintf(header, sizeof(header), "%s.%s|", block->kind, block->name);
        
        for (size_t j = 0; j < block->field_count; j++) {
            if (j > 0) {
                append_char(&result, &len, &cap, ' ');
            }
            if (block->fields[j].type_hint && *block->fields[j].type_hint) {
                char field[256];
                snprintf(field, sizeof(field), "%s:%s", block->fields[j].name, block->fields[j].type_hint);
                append_string(&result, &len, &cap, field);
            } else {
                append_string(&result, &len, &cap, block->fields[j].name);
            }
        }
        append_char(&result, &len, &cap, '|');
        
        char *field_header_end = result + len;
        (void)field_header_end;
        
        for (size_t r = 0; r < block->row_count; r++) {
            if (r > 0 || i > 0) append_char(&result, &len, &cap, '\n');
            append_string(&result, &len, &cap, header);
            
            for (size_t j = 0; j < block->field_count; j++) {
                if (j > 0) append_char(&result, &len, &cap, ' ');
                if (block->fields[j].type_hint && *block->fields[j].type_hint) {
                    char field[256];
                    snprintf(field, sizeof(field), "%s:%s", block->fields[j].name, block->fields[j].type_hint);
                    append_string(&result, &len, &cap, field);
                } else {
                    append_string(&result, &len, &cap, block->fields[j].name);
                }
            }
            append_char(&result, &len, &cap, '|');
            
            ison_row_t *row = block->rows[r];
            for (size_t j = 0; j < block->field_count; j++) {
                if (j > 0) append_char(&result, &len, &cap, ' ');
                ison_value_t *val = ison_row_get_ptr(row, block->fields[j].name);
                if (val) {
                    char *str = ison_value_to_ison(val);
                    append_string(&result, &len, &cap, str);
                    free(str);
                } else {
                    append_char(&result, &len, &cap, '~');
                }
            }
        }
    }
    
    return result;
}

ison_dumps_options_t ison_default_dumps_options(void) {
    ison_dumps_options_t opts = {0};
    opts.align_columns = 0;
    opts.delimiter = NULL;
    return opts;
}

ison_fromdict_options_t ison_default_fromdict_options(void) {
    ison_fromdict_options_t opts = {0};
    opts.auto_refs = 0;
    opts.smart_order = 0;
    return opts;
}
