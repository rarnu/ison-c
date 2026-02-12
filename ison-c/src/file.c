#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ison.h"

char *ison_read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (size < 0) {
        fclose(f);
        return NULL;
    }
    
    char *buf = malloc(size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    
    size_t read = fread(buf, 1, size, f);
    fclose(f);
    
    buf[read] = '\0';
    if (out_len) *out_len = read;
    return buf;
}

ison_error_t ison_write_file(const char *path, const char *content) {
    if (!path || !content) return ISON_ERROR_INVALID;
    
    FILE *f = fopen(path, "wb");
    if (!f) return ISON_ERROR_IO;
    
    size_t len = strlen(content);
    if (fwrite(content, 1, len, f) != len) {
        fclose(f);
        return ISON_ERROR_IO;
    }
    
    fclose(f);
    return ISON_OK;
}

ison_document_t *ison_load(const char *path, ison_error_t *error) {
    if (error) *error = ISON_OK;
    
    char *content = ison_read_file(path, NULL);
    if (!content) {
        if (error) *error = ISON_ERROR_IO;
        return NULL;
    }
    
    ison_document_t *doc = ison_parse(content, error);
    free(content);
    return doc;
}

ison_error_t ison_dump(const ison_document_t *doc, const char *path) {
    char *content = ison_dumps(doc);
    if (!content) return ISON_ERROR_MEMORY;
    
    ison_error_t err = ison_write_file(path, content);
    free(content);
    return err;
}

ison_document_t *ison_load_isonl(const char *path, ison_error_t *error) {
    if (error) *error = ISON_OK;
    
    char *content = ison_read_file(path, NULL);
    if (!content) {
        if (error) *error = ISON_ERROR_IO;
        return NULL;
    }
    
    ison_document_t *doc = ison_parse_isonl(content, error);
    free(content);
    return doc;
}

ison_error_t ison_dump_isonl(const ison_document_t *doc, const char *path) {
    char *content = ison_dumps_isonl(doc);
    if (!content) return ISON_ERROR_MEMORY;
    
    ison_error_t err = ison_write_file(path, content);
    free(content);
    return err;
}
