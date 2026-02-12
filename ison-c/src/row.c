#include <stdlib.h>
#include <string.h>
#include "ison.h"

ison_row_t *ison_row_create(void) {
    ison_row_t *row = calloc(1, sizeof(ison_row_t));
    return row;
}

void ison_row_set(ison_row_t *row, const char *key, const ison_value_t *value) {
    if (!row || !key) return;
    
    ison_row_entry_t *entry = row->head;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            ison_value_free(&entry->value);
            entry->value = *value;
            return;
        }
        entry = entry->next;
    }
    
    entry = malloc(sizeof(ison_row_entry_t));
    if (!entry) return;
    
    entry->key = malloc(strlen(key) + 1);
    if (!entry->key) {
        free(entry);
        return;
    }
    strcpy(entry->key, key);
    entry->value = *value;
    entry->next = NULL;
    
    if (row->tail) {
        row->tail->next = entry;
    } else {
        row->head = entry;
    }
    row->tail = entry;
    row->count++;
}

bool ison_row_get(const ison_row_t *row, const char *key, ison_value_t *out) {
    if (!row || !key) return false;
    
    ison_row_entry_t *entry = row->head;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (out) *out = entry->value;
            return true;
        }
        entry = entry->next;
    }
    return false;
}

ison_value_t *ison_row_get_ptr(const ison_row_t *row, const char *key) {
    if (!row || !key) return NULL;
    
    ison_row_entry_t *entry = row->head;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return &entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

void ison_row_free(ison_row_t *row) {
    if (!row) return;
    
    ison_row_entry_t *entry = row->head;
    while (entry) {
        ison_row_entry_t *next = entry->next;
        free(entry->key);
        ison_value_free(&entry->value);
        free(entry);
        entry = next;
    }
    free(row);
}
