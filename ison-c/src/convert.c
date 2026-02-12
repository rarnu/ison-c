#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ison.h"

static void append_string(char **buf, size_t *len, size_t *cap, const char *str);
static void append_char(char **buf, size_t *len, size_t *cap, char ch);

char *ison_to_isonl(const char *ison_text, ison_error_t *error) {
    if (error) *error = ISON_OK;
    if (!ison_text) return NULL;
    
    ison_document_t *doc = ison_parse(ison_text, error);
    if (!doc) return NULL;
    
    char *result = ison_dumps_isonl(doc);
    ison_document_free(doc);
    return result;
}

char *isonl_to_ison(const char *isonl_text, ison_error_t *error) {
    if (error) *error = ISON_OK;
    if (!isonl_text) return NULL;
    
    ison_document_t *doc = ison_parse_isonl(isonl_text, error);
    if (!doc) return NULL;
    
    char *result = ison_dumps(doc);
    ison_document_free(doc);
    return result;
}

char *ison_to_json(const char *ison_text, ison_error_t *error) {
    if (error) *error = ISON_OK;
    if (!ison_text) {
        if (error) *error = ISON_ERROR_INVALID;
        return NULL;
    }
    
    ison_document_t *doc = ison_parse(ison_text, error);
    if (!doc) {
        if (error && *error == ISON_OK) *error = ISON_ERROR_PARSE;
        return NULL;
    }
    
    size_t len = 0, cap = 1024;
    char *result = malloc(cap);
    if (!result) {
        ison_document_free(doc);
        if (error) *error = ISON_ERROR_MEMORY;
        return NULL;
    }
    *result = '\0';
    
    append_string(&result, &len, &cap, "{");
    
    for (size_t i = 0; i < doc->order_count; i++) {
        if (i > 0) append_string(&result, &len, &cap, ",");
        
        ison_block_t *block = ison_document_get(doc, doc->order[i]);
        if (!block) continue;
        
        append_char(&result, &len, &cap, '"');
        append_string(&result, &len, &cap, block->name);
        append_string(&result, &len, &cap, "\":[ ");
        
        for (size_t r = 0; r < block->row_count; r++) {
            if (r > 0) append_string(&result, &len, &cap, ",");
            append_string(&result, &len, &cap, "{");
            
            ison_row_t *row = block->rows[r];
            int first = 1;
            for (size_t j = 0; j < block->field_count; j++) {
                ison_value_t *val = ison_row_get_ptr(row, block->fields[j].name);
                if (val) {
                    if (!first) append_string(&result, &len, &cap, ",");
                    first = 0;
                    
                    append_char(&result, &len, &cap, '"');
                    append_string(&result, &len, &cap, block->fields[j].name);
                    append_string(&result, &len, &cap, "\":");
                    
                    char *json_val = ison_value_to_json(val);
                    append_string(&result, &len, &cap, json_val);
                    free(json_val);
                }
            }
            append_string(&result, &len, &cap, "}");
        }
        
        append_string(&result, &len, &cap, "]");
    }
    
    append_string(&result, &len, &cap, "}");
    ison_document_free(doc);
    return result;
}

static void skip_ws(const char **p) {
    while (**p && (**p == ' ' || **p == '\t' || **p == '\n' || **p == '\r')) (*p)++;
}

static char *parse_json_string(const char **p) {
    if (**p != '"') return NULL;
    (*p)++;
    
    const char *start = *p;
    size_t len = 0;
    while (**p && **p != '"') {
        if (**p == '\\') {
            (*p) += 2;
            len++;
        } else {
            (*p)++;
            len++;
        }
    }
    
    char *result = malloc(len + 1);
    if (!result) return NULL;
    
    const char *src = start;
    char *dst = result;
    while (src < *p) {
        if (*src == '\\') {
            src++;
            switch (*src) {
                case '"': *dst++ = '"'; break;
                case '\\': *dst++ = '\\'; break;
                case '/': *dst++ = '/'; break;
                case 'b': *dst++ = '\b'; break;
                case 'f': *dst++ = '\f'; break;
                case 'n': *dst++ = '\n'; break;
                case 'r': *dst++ = '\r'; break;
                case 't': *dst++ = '\t'; break;
                default: *dst++ = *src;
            }
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    
    if (**p == '"') (*p)++;
    return result;
}

static ison_value_t parse_json_value(const char **p);

static ison_row_t *parse_json_object(const char **p) {
    if (**p != '{') return NULL;
    (*p)++;
    
    ison_row_t *row = ison_row_create();
    skip_ws(p);
    
    if (**p == '}') {
        (*p)++;
        return row;
    }
    
    while (**p) {
        skip_ws(p);
        char *key = parse_json_string(p);
        if (!key) break;
        
        skip_ws(p);
        if (**p == ':') (*p)++;
        skip_ws(p);
        
        ison_value_t val = parse_json_value(p);
        ison_row_set(row, key, &val);
        free(key);
        
        skip_ws(p);
        if (**p == ',') {
            (*p)++;
            continue;
        }
        if (**p == '}') {
            (*p)++;
            break;
        }
    }
    
    return row;
}

static ison_value_t parse_json_value(const char **p) {
    skip_ws(p);
    
    if (**p == '"') {
        char *s = parse_json_string(p);
        ison_value_t v = ison_string(s);
        free(s);
        return v;
    }
    if (strncmp(*p, "true", 4) == 0) {
        *p += 4;
        return ison_bool(1);
    }
    if (strncmp(*p, "false", 5) == 0) {
        *p += 5;
        return ison_bool(0);
    }
    if (strncmp(*p, "null", 4) == 0) {
        *p += 4;
        return ison_null();
    }
    if (**p == '{') {
        ison_row_t *row = parse_json_object(p);
        return ison_ref(&(ison_reference_t){(char*)row, NULL, NULL});
    }
    if (**p == '[') {
        (*p)++;
        skip_ws(p);
        if (**p == ']') {
            (*p)++;
            return ison_null();
        }
        
        while (**p && **p != ']') {
            parse_json_value(p);
            skip_ws(p);
            if (**p == ',') (*p)++;
            skip_ws(p);
        }
        if (**p == ']') (*p)++;
        return ison_null();
    }
    
    char *end;
    double d = strtod(*p, &end);
    if (end != *p) {
        *p = end;
        if (d == (double)(int64_t)d) return ison_int((int64_t)d);
        return ison_float(d);
    }
    
    return ison_null();
}

ison_document_t *ison_from_json(const char *json_text, ison_error_t *error) {
    if (error) *error = ISON_OK;
    if (!json_text) {
        if (error) *error = ISON_ERROR_INVALID;
        return NULL;
    }
    
    ison_document_t *doc = ison_document_create();
    const char *p = json_text;
    
    skip_ws(&p);
    if (*p != '{') {
        if (error) *error = ISON_ERROR_PARSE;
        ison_document_free(doc);
        return NULL;
    }
    p++;
    
    while (*p) {
        skip_ws(&p);
        if (*p == '}') break;
        
        char *name = parse_json_string(&p);
        if (!name) break;
        
        skip_ws(&p);
        if (*p == ':') p++;
        skip_ws(&p);
        
        if (*p == '[') {
            p++;
            skip_ws(&p);
            
            ison_block_t *block = ison_block_create("table", name);
            
            if (*p != ']') {
                const char *peek = p;
                skip_ws(&peek);
                if (*peek == '{') {
                    ison_row_t *first = parse_json_object(&p);
                    if (first) {
                        ison_row_entry_t *entry = first->head;
                        while (entry) {
                            ison_block_add_field(block, entry->key, "");
                            entry = entry->next;
                        }
                        
                        ison_block_add_row(block, first);
                        ison_row_free(first);
                        
                        skip_ws(&p);
                        while (*p && *p != ']') {
                            if (*p == ',') p++;
                            skip_ws(&p);
                            if (*p != '{') break;
                            
                            ison_row_t *row = parse_json_object(&p);
                            if (row) {
                                ison_block_add_row(block, row);
                                ison_row_free(row);
                            }
                            skip_ws(&p);
                        }
                    }
                }
            }
            
            if (*p == ']') p++;
            ison_document_add_block(doc, block);
        } else if (*p == '{') {
            ison_row_t *row = parse_json_object(&p);
            if (row) {
                ison_block_t *block = ison_block_create("object", name);
                ison_row_entry_t *entry = row->head;
                while (entry) {
                    ison_block_add_field(block, entry->key, "");
                    entry = entry->next;
                }
                ison_block_add_row(block, row);
                ison_row_free(row);
                ison_document_add_block(doc, block);
            }
        } else {
            parse_json_value(&p);
        }
        
        free(name);
        
        skip_ws(&p);
        if (*p == ',') {
            p++;
            continue;
        }
        if (*p == '}') break;
    }
    
    return doc;
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
