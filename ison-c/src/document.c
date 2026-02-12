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

ison_document_t *ison_document_create(void) {
    ison_document_t *doc = calloc(1, sizeof(ison_document_t));
    return doc;
}

void ison_document_add_block(ison_document_t *doc, ison_block_t *block) {
    if (!doc || !block || !block->name) return;
    
    for (size_t i = 0; i < doc->block_count; i++) {
        if (strcmp(doc->blocks[i]->name, block->name) == 0) {
            ison_block_free(doc->blocks[i]);
            doc->blocks[i] = block;
            return;
        }
    }
    
    if (doc->block_count >= doc->block_capacity) {
        size_t new_cap = doc->block_capacity == 0 ? 8 : doc->block_capacity * 2;
        
        ison_block_t **new_blocks = realloc(doc->blocks, new_cap * sizeof(ison_block_t *));
        if (!new_blocks) return;
        doc->blocks = new_blocks;
        
        char **new_order = realloc(doc->order, new_cap * sizeof(char *));
        if (!new_order) return;
        doc->order = new_order;
        
        doc->block_capacity = new_cap;
    }
    
    doc->blocks[doc->block_count] = block;
    doc->order[doc->order_count] = strdup_safe(block->name);
    doc->order_count++;
    doc->block_count++;
}

ison_block_t *ison_document_get(const ison_document_t *doc, const char *name) {
    if (!doc || !name) return NULL;
    
    for (size_t i = 0; i < doc->block_count; i++) {
        if (strcmp(doc->blocks[i]->name, name) == 0) {
            return doc->blocks[i];
        }
    }
    return NULL;
}

const char **ison_document_get_order(const ison_document_t *doc, size_t *count) {
    if (!doc || !count) return NULL;
    
    *count = doc->order_count;
    if (*count == 0) return NULL;
    
    return (const char **)doc->order;
}

void ison_document_free(ison_document_t *doc) {
    if (!doc) return;
    
    for (size_t i = 0; i < doc->block_count; i++) {
        ison_block_free(doc->blocks[i]);
    }
    free(doc->blocks);
    
    for (size_t i = 0; i < doc->order_count; i++) {
        free(doc->order[i]);
    }
    free(doc->order);
    
    free(doc);
}
