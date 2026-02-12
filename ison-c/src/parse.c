#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ison.h"

typedef struct {
    const char *text;
    char **lines;
    size_t line_count;
    size_t pos;
} parser_t;

static char *strdup_safe(const char *str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char *copy = malloc(len + 1);
    if (copy) memcpy(copy, str, len + 1);
    return copy;
}

static void trim_right(char *str) {
    if (!str) return;
    size_t len = strlen(str);
    while (len > 0 && (str[len-1] == '\r' || str[len-1] == '\n')) {
        str[--len] = '\0';
    }
}

static char *trim(const char *str) {
    if (!str) return strdup_safe("");
    while (*str && isspace((unsigned char)*str)) str++;
    if (!*str) return strdup_safe("");
    
    const char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    size_t len = end - str + 1;
    char *result = malloc(len + 1);
    if (result) {
        memcpy(result, str, len);
        result[len] = '\0';
    }
    return result;
}

static char **split_lines(const char *text, size_t *count) {
    size_t cap = 16;
    char **lines = malloc(cap * sizeof(char *));
    *count = 0;
    
    const char *p = text;
    while (*p) {
        const char *end = p;
        while (*end && *end != '\n') end++;
        
        size_t len = end - p;
        char *line = malloc(len + 1);
        if (line) {
            memcpy(line, p, len);
            line[len] = '\0';
            trim_right(line);
        }
        
        if (*count >= cap) {
            cap *= 2;
            lines = realloc(lines, cap * sizeof(char *));
        }
        lines[(*count)++] = line;
        
        if (*end == '\n') end++;
        p = end;
    }
    
    return lines;
}

static int is_valid_kind(const char *kind) {
    return strcmp(kind, "table") == 0 ||
           strcmp(kind, "object") == 0 ||
           strcmp(kind, "meta") == 0;
}

static char **tokenize(const char *line, size_t *count) {
    size_t cap = 16;
    char **tokens = malloc(cap * sizeof(char *));
    *count = 0;
    
    size_t len = strlen(line);
    char *current = malloc(len + 1);
    size_t cur_len = 0;
    int in_quotes = 0;
    int escaped = 0;
    
    for (size_t i = 0; i <= len; i++) {
        char ch = line[i];
        
        if (escaped) {
            switch (ch) {
                case 'n': current[cur_len++] = '\n'; break;
                case 't': current[cur_len++] = '\t'; break;
                case '"': current[cur_len++] = '"'; break;
                case '\\': current[cur_len++] = '\\'; break;
                default: current[cur_len++] = ch;
            }
            escaped = 0;
            continue;
        }
        
        if (ch == '\\' && in_quotes) {
            escaped = 1;
            continue;
        }
        
        if (ch == '"') {
            in_quotes = !in_quotes;
            continue;
        }
        
        if (!in_quotes && (ch == ' ' || ch == '\t' || ch == '\0')) {
            if (cur_len > 0) {
                current[cur_len] = '\0';
                if (*count >= cap) {
                    cap *= 2;
                    tokens = realloc(tokens, cap * sizeof(char *));
                }
                tokens[(*count)++] = strdup_safe(current);
                cur_len = 0;
            }
            continue;
        }
        
        current[cur_len++] = ch;
    }
    
    free(current);
    return tokens;
}

static void parse_field_def(const char *field, char **name, char **type_hint) {
    const char *colon = strchr(field, ':');
    if (colon && colon != field) {
        size_t name_len = colon - field;
        *name = malloc(name_len + 1);
        memcpy(*name, field, name_len);
        (*name)[name_len] = '\0';
        *type_hint = strdup_safe(colon + 1);
    } else {
        *name = strdup_safe(field);
        *type_hint = strdup_safe("");
    }
}

static int is_all_upper(const char *str) {
    if (!str || !*str) return 0;
    for (const char *p = str; *p; p++) {
        if (*p != '_' && (*p < 'A' || *p > 'Z')) return 0;
    }
    return 1;
}

static ison_reference_t parse_reference(const char *token) {
    ison_reference_t ref = {NULL, NULL, NULL};
    if (!token || *token != ':') return ref;
    
    token++;
    char *copy = strdup_safe(token);
    char *colon = strchr(copy, ':');
    
    if (!colon) {
        ref.id = copy;
        return ref;
    }
    
    *colon = '\0';
    char *ns = copy;
    char *id = colon + 1;
    
    if (is_all_upper(ns)) {
        ref.relationship = strdup_safe(ns);
    } else {
        ref.ns = strdup_safe(ns);
    }
    ref.id = strdup_safe(id);
    free(copy);
    return ref;
}

static ison_value_t parse_value_token(const char *token, const char *type_hint) {
    if (!token || strcmp(token, "~") == 0 ||
        strcasecmp(token, "null") == 0 ||
        strcasecmp(token, "NULL") == 0) {
        return ison_null();
    }
    
    if (strcasecmp(token, "true") == 0 || strcasecmp(token, "TRUE") == 0) {
        return ison_bool(1);
    }
    if (strcasecmp(token, "false") == 0 || strcasecmp(token, "FALSE") == 0) {
        return ison_bool(0);
    }
    
    if (*token == ':') {
        ison_reference_t ref = parse_reference(token);
        return ison_ref(&ref);
    }
    
    if (type_hint && *type_hint) {
        if (strcmp(type_hint, "int") == 0) {
            char *end;
            long val = strtol(token, &end, 10);
            if (*end == '\0') return ison_int(val);
        } else if (strcmp(type_hint, "float") == 0) {
            char *end;
            double val = strtod(token, &end);
            if (*end == '\0') return ison_float(val);
        } else if (strcmp(type_hint, "bool") == 0) {
            if (strcmp(token, "true") == 0 || strcmp(token, "1") == 0)
                return ison_bool(1);
            if (strcmp(token, "false") == 0 || strcmp(token, "0") == 0)
                return ison_bool(0);
        } else if (strcmp(type_hint, "string") == 0) {
            return ison_string(token);
        }
    }
    
    char *end;
    long ival = strtol(token, &end, 10);
    if (*end == '\0') return ison_int(ival);
    
    double fval = strtod(token, &end);
    if (*end == '\0') return ison_float(fval);
    
    return ison_string(token);
}

static ison_block_t *parse_block(parser_t *p, const char *kind, const char *name) {
    ison_block_t *block = ison_block_create(kind, name);
    p->pos++;
    
    while (p->pos < p->line_count) {
        char *line = trim(p->lines[p->pos]);
        if (strlen(line) > 0 && line[0] != '#') {
            free(line);
            break;
        }
        free(line);
        p->pos++;
    }
    
    if (p->pos >= p->line_count) return block;
    
    char *fields_line = trim(p->lines[p->pos]);
    size_t field_count;
    char **field_tokens = tokenize(fields_line, &field_count);
    
    for (size_t i = 0; i < field_count; i++) {
        char *fname, *ftype;
        parse_field_def(field_tokens[i], &fname, &ftype);
        ison_block_add_field(block, fname, ftype);
        free(fname);
        free(ftype);
        free(field_tokens[i]);
    }
    free(field_tokens);
    free(fields_line);
    p->pos++;
    
    int in_summary = 0;
    while (p->pos < p->line_count) {
        char *line = trim(p->lines[p->pos]);
        
        if (strlen(line) == 0) {
            free(line);
            p->pos++;
            break;
        }
        
        if (line[0] == '#') {
            free(line);
            p->pos++;
            continue;
        }
        
        if (strchr(line, '.') && line[0] != '"') {
            char *dot = strchr(line, '.');
            char *kind_part = malloc(dot - line + 1);
            memcpy(kind_part, line, dot - line);
            kind_part[dot - line] = '\0';
            if (is_valid_kind(kind_part)) {
                free(kind_part);
                free(line);
                break;
            }
            free(kind_part);
        }
        
        if (strcmp(line, "---") == 0) {
            in_summary = 1;
            free(line);
            p->pos++;
            continue;
        }
        
        size_t token_count;
        char **tokens = tokenize(line, &token_count);
        ison_row_t *row = ison_row_create();
        
        for (size_t i = 0; i < token_count && i < block->field_count; i++) {
            ison_value_t val = parse_value_token(tokens[i], block->fields[i].type_hint);
            ison_row_set(row, block->fields[i].name, &val);
            free(tokens[i]);
        }
        free(tokens);
        
        if (in_summary) {
            ison_block_set_summary(block, row);
        } else {
            ison_block_add_row(block, row);
        }
        free(row);
        
        free(line);
        p->pos++;
    }
    
    return block;
}

ison_document_t *ison_parse(const char *text, ison_error_t *error) {
    if (error) *error = ISON_OK;
    if (!text) return ison_document_create();
    
    parser_t p;
    p.text = text;
    p.lines = split_lines(text, &p.line_count);
    p.pos = 0;
    
    ison_document_t *doc = ison_document_create();
    
    while (p.pos < p.line_count) {
        char *line = trim(p.lines[p.pos]);
        
        if (strlen(line) == 0 || line[0] == '#') {
            free(line);
            p.pos++;
            continue;
        }
        
        char *dot = strchr(line, '.');
        if (dot && line[0] != '"') {
            size_t kind_len = dot - line;
            char *kind = malloc(kind_len + 1);
            memcpy(kind, line, kind_len);
            kind[kind_len] = '\0';
            
            if (is_valid_kind(kind)) {
                char *name = strdup_safe(dot + 1);
                ison_block_t *block = parse_block(&p, kind, name);
                ison_document_add_block(doc, block);
                free(kind);
                free(name);
                free(line);
                continue;
            }
            free(kind);
        }
        
        free(line);
        p.pos++;
    }
    
    for (size_t i = 0; i < p.line_count; i++) {
        free(p.lines[i]);
    }
    free(p.lines);
    
    return doc;
}

ison_document_t *ison_parse_isonl(const char *text, ison_error_t *error) {
    if (error) *error = ISON_OK;
    if (!text) return ison_document_create();
    
    ison_document_t *doc = ison_document_create();
    size_t line_count;
    char **lines = split_lines(text, &line_count);
    
    for (size_t i = 0; i < line_count; i++) {
        char *line = trim(lines[i]);
        if (strlen(line) == 0 || line[0] == '#') {
            free(line);
            continue;
        }
        
        char *p1 = strchr(line, '|');
        char *p2 = p1 ? strchr(p1 + 1, '|') : NULL;
        
        if (!p1 || !p2) {
            free(line);
            continue;
        }
        
        *p1 = '\0';
        *p2 = '\0';
        
        char *header = line;
        char *fields_str = p1 + 1;
        char *data_str = p2 + 1;
        
        char *dot = strchr(header, '.');
        if (!dot) {
            free(line);
            continue;
        }
        
        *dot = '\0';
        char *kind = header;
        char *name = dot + 1;
        
        ison_block_t *block = ison_document_get(doc, name);
        if (!block) {
            block = ison_block_create(kind, name);
            
            size_t field_count;
            char **field_tokens = tokenize(fields_str, &field_count);
            for (size_t j = 0; j < field_count; j++) {
                char *fname, *ftype;
                parse_field_def(field_tokens[j], &fname, &ftype);
                ison_block_add_field(block, fname, ftype);
                free(fname);
                free(ftype);
                free(field_tokens[j]);
            }
            free(field_tokens);
            
            ison_document_add_block(doc, block);
        }
        
        size_t data_count;
        char **data_tokens = tokenize(data_str, &data_count);
        ison_row_t *row = ison_row_create();
        
        for (size_t j = 0; j < data_count && j < block->field_count; j++) {
            ison_value_t val = parse_value_token(data_tokens[j], block->fields[j].type_hint);
            ison_row_set(row, block->fields[j].name, &val);
            free(data_tokens[j]);
        }
        free(data_tokens);
        
        ison_block_add_row(block, row);
        free(row);
        
        free(line);
    }
    
    for (size_t i = 0; i < line_count; i++) {
        free(lines[i]);
    }
    free(lines);
    
    return doc;
}
