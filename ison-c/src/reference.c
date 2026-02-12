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

ison_reference_t ison_reference_make(const char *id, const char *ns, const char *relationship) {
    ison_reference_t ref;
    ref.id = strdup_safe(id);
    ref.ns = strdup_safe(ns);
    ref.relationship = strdup_safe(relationship);
    return ref;
}

char *ison_reference_to_ison(const ison_reference_t *ref) {
    if (!ref || !ref->id) return NULL;
    
    if (ref->relationship && *ref->relationship) {
        size_t len = strlen(ref->relationship) + strlen(ref->id) + 3;
        char *result = malloc(len);
        if (result) sprintf(result, ":%s:%s", ref->relationship, ref->id);
        return result;
    }
    if (ref->ns && *ref->ns) {
        size_t len = strlen(ref->ns) + strlen(ref->id) + 3;
        char *result = malloc(len);
        if (result) sprintf(result, ":%s:%s", ref->ns, ref->id);
        return result;
    }
    size_t len = strlen(ref->id) + 2;
    char *result = malloc(len);
    if (result) sprintf(result, ":%s", ref->id);
    return result;
}

bool ison_reference_is_relationship(const ison_reference_t *ref) {
    return ref && ref->relationship && *ref->relationship;
}

char *ison_reference_get_ns(const ison_reference_t *ref) {
    if (!ref) return NULL;
    if (ref->relationship && *ref->relationship) return ref->relationship;
    return ref->ns;
}

void ison_reference_free(ison_reference_t *ref) {
    if (!ref) return;
    free(ref->id);
    free(ref->ns);
    free(ref->relationship);
    ref->id = NULL;
    ref->ns = NULL;
    ref->relationship = NULL;
}
