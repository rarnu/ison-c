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

ison_block_t *ison_block_create(const char *kind, const char *name) {
    ison_block_t *block = calloc(1, sizeof(ison_block_t));
    if (!block) return NULL;
    
    block->kind = strdup_safe(kind);
    block->name = strdup_safe(name);
    block->fields = NULL;
    block->field_count = 0;
    block->field_capacity = 0;
    block->rows = NULL;
    block->row_count = 0;
    block->row_capacity = 0;
    block->summary_row = NULL;
    
    return block;
}

void ison_block_add_field(ison_block_t *block, const char *name, const char *type_hint) {
    if (!block || !name) return;
    
    if (block->field_count >= block->field_capacity) {
        size_t new_cap = block->field_capacity == 0 ? 8 : block->field_capacity * 2;
        ison_field_info_t *new_fields = realloc(block->fields, new_cap * sizeof(ison_field_info_t));
        if (!new_fields) return;
        block->fields = new_fields;
        block->field_capacity = new_cap;
    }
    
    block->fields[block->field_count].name = strdup_safe(name);
    block->fields[block->field_count].type_hint = strdup_safe(type_hint);
    block->field_count++;
}

void ison_block_add_row(ison_block_t *block, const ison_row_t *row) {
    if (!block || !row) return;
    
    if (block->row_count >= block->row_capacity) {
        size_t new_cap = block->row_capacity == 0 ? 8 : block->row_capacity * 2;
        ison_row_t **new_rows = realloc(block->rows, new_cap * sizeof(ison_row_t *));
        if (!new_rows) return;
        block->rows = new_rows;
        block->row_capacity = new_cap;
    }
    
    ison_row_t *copy = ison_row_create();
    if (!copy) return;
    
    ison_row_entry_t *entry = row->head;
    while (entry) {
        ison_row_set(copy, entry->key, &entry->value);
        entry = entry->next;
    }
    
    block->rows[block->row_count++] = copy;
}

void ison_block_set_summary(ison_block_t *block, const ison_row_t *row) {
    if (!block) return;
    if (block->summary_row) {
        ison_row_free(block->summary_row);
    }
    if (!row) {
        block->summary_row = NULL;
        return;
    }
    
    block->summary_row = ison_row_create();
    if (!block->summary_row) return;
    
    ison_row_entry_t *entry = row->head;
    while (entry) {
        ison_row_set(block->summary_row, entry->key, &entry->value);
        entry = entry->next;
    }
}

char **ison_block_get_field_names(const ison_block_t *block, size_t *count) {
    if (!block || !count) return NULL;
    
    *count = block->field_count;
    if (*count == 0) return NULL;
    
    char **names = malloc(*count * sizeof(char *));
    if (!names) return NULL;
    
    for (size_t i = 0; i < *count; i++) {
        names[i] = strdup_safe(block->fields[i].name);
    }
    
    return names;
}

void ison_block_free(ison_block_t *block) {
    if (!block) return;
    
    free(block->kind);
    free(block->name);
    
    for (size_t i = 0; i < block->field_count; i++) {
        free(block->fields[i].name);
        free(block->fields[i].type_hint);
    }
    free(block->fields);
    
    for (size_t i = 0; i < block->row_count; i++) {
        ison_row_free(block->rows[i]);
    }
    free(block->rows);
    
    if (block->summary_row) {
        ison_row_free(block->summary_row);
    }
    
    free(block);
}
