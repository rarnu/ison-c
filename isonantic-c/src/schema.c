/**
 * @file schema.c
 * @brief Schema validation implementation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <regex.h>
#include <math.h>
#include "isonantic.h"

/* ==================== Helper Functions ==================== */

static int string_length(const char* str) {
    return str ? strlen(str) : 0;
}

static bool strings_equal(const char* a, const char* b) {
    if (a == NULL && b == NULL) return true;
    if (a == NULL || b == NULL) return false;
    return strcmp(a, b) == 0;
}

/* Email validation pattern */
static bool is_valid_email(const char* str) {
    if (str == NULL) return false;
    regex_t regex;
    regcomp(&regex, "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$", REG_EXTENDED);
    bool result = regexec(&regex, str, 0, NULL, 0) == 0;
    regfree(&regex);
    return result;
}

/* URL validation pattern */
static bool is_valid_url(const char* str) {
    if (str == NULL) return false;
    regex_t regex;
    regcomp(&regex, "^https?://[^\\s/$.?#].[^\\s]*$", REG_EXTENDED);
    bool result = regexec(&regex, str, 0, NULL, 0) == 0;
    regfree(&regex);
    return result;
}

/* ==================== Base Schema Functions ==================== */

void isonantic_schema_set_optional(IsonanticSchema* schema) {
    if (schema == NULL) return;
    schema->optional = true;
}

void isonantic_schema_set_default(IsonanticSchema* schema, IsonanticValue* value) {
    if (schema == NULL) return;
    schema->has_default = true;
    schema->default_value = value;
}

void isonantic_schema_set_description(IsonanticSchema* schema, const char* desc) {
    if (schema == NULL || desc == NULL) return;
    free(schema->description);
    schema->description = strdup(desc);
}

void isonantic_schema_add_refinement(IsonanticSchema* schema, IsonanticRefinementFn fn, void* user_data, const char* message) {
    if (schema == NULL || fn == NULL) return;

    /* Create refinement struct */
    IsonanticRefinement* refinement = (IsonanticRefinement*)malloc(sizeof(IsonanticRefinement));
    if (refinement == NULL) return;
    refinement->fn = fn;
    refinement->user_data = user_data;
    refinement->error_message = message ? strdup(message) : NULL;

    if (schema->refinements == NULL) {
        schema->refinements = isonantic_array_create(4);
    }
    isonantic_array_add(schema->refinements, refinement);
}

IsonanticValidationErrors* isonantic_schema_run_refinements(IsonanticSchema* schema, IsonanticValue* value) {
    if (schema == NULL || schema->refinements == NULL) return NULL;

    IsonanticValidationErrors* errors = NULL;
    int size = isonantic_array_size(schema->refinements);

    for (int i = 0; i < size; i++) {
        IsonanticRefinement* refinement = (IsonanticRefinement*)isonantic_array_get(schema->refinements, i);
        if (refinement == NULL || refinement->fn == NULL) continue;

        IsonanticValidationErrors* result = refinement->fn(value, refinement->user_data);
        if (result != NULL && isonantic_validation_errors_has_errors(result)) {
            if (errors == NULL) {
                errors = isonantic_validation_errors_create();
            }
            /* Add first error from refinement */
            if (result->head != NULL && refinement->error_message) {
                IsonanticValidationError* err = isonantic_validation_error_create(
                    "", refinement->error_message, value);
                isonantic_validation_errors_add(errors, err);
            }
            isonantic_validation_errors_free(result);
        } else if (result != NULL) {
            isonantic_validation_errors_free(result);
        }
    }

    return errors;
}

/* ==================== String Schema ==================== */

static IsonanticValidationErrors* string_validate(IsonanticSchema* schema, IsonanticValue* value) {
    IsonanticStringSchema* s = (IsonanticStringSchema*)schema;
    IsonanticValidationErrors* errors = NULL;

    if (value == NULL) {
        if (s->base.optional) {
            return NULL;
        }
        errors = isonantic_validation_errors_create();
        IsonanticValidationError* err = isonantic_validation_error_create(
            "", "required field is missing", NULL);
        isonantic_validation_errors_add(errors, err);
        return errors;
    }

    if (value->type != ISONANTIC_VALUE_STRING) {
        errors = isonantic_validation_errors_create();
        char msg[64];
        snprintf(msg, sizeof(msg), "expected string, got %d", value->type);
        IsonanticValidationError* err = isonantic_validation_error_create("", msg, value);
        isonantic_validation_errors_add(errors, err);
        return errors;
    }

    const char* str = value->data.string_value;

    if (s->min_len != NULL && string_length(str) < *s->min_len) {
        if (errors == NULL) errors = isonantic_validation_errors_create();
        char msg[64];
        snprintf(msg, sizeof(msg), "string must be at least %d characters", *s->min_len);
        IsonanticValidationError* err = isonantic_validation_error_create("", msg, value);
        isonantic_validation_errors_add(errors, err);
    }

    if (s->max_len != NULL && string_length(str) > *s->max_len) {
        if (errors == NULL) errors = isonantic_validation_errors_create();
        char msg[64];
        snprintf(msg, sizeof(msg), "string must be at most %d characters", *s->max_len);
        IsonanticValidationError* err = isonantic_validation_error_create("", msg, value);
        isonantic_validation_errors_add(errors, err);
    }

    if (s->exact_len != NULL && string_length(str) != *s->exact_len) {
        if (errors == NULL) errors = isonantic_validation_errors_create();
        char msg[64];
        snprintf(msg, sizeof(msg), "string must be exactly %d characters", *s->exact_len);
        IsonanticValidationError* err = isonantic_validation_error_create("", msg, value);
        isonantic_validation_errors_add(errors, err);
    }

    if (s->is_email && !is_valid_email(str)) {
        if (errors == NULL) errors = isonantic_validation_errors_create();
        IsonanticValidationError* err = isonantic_validation_error_create("", "invalid email format", value);
        isonantic_validation_errors_add(errors, err);
    }

    if (s->is_url && !is_valid_url(str)) {
        if (errors == NULL) errors = isonantic_validation_errors_create();
        IsonanticValidationError* err = isonantic_validation_error_create("", "invalid URL format", value);
        isonantic_validation_errors_add(errors, err);
    }

    if (s->pattern != NULL) {
        regex_t* regex = (regex_t*)s->pattern;
        if (regexec(regex, str, 0, NULL, 0) != 0) {
            if (errors == NULL) errors = isonantic_validation_errors_create();
            IsonanticValidationError* err = isonantic_validation_error_create("", "string does not match required pattern", value);
            isonantic_validation_errors_add(errors, err);
        }
    }

    /* Run refinements */
    IsonanticValidationErrors* refinements = isonantic_schema_run_refinements(schema, value);
    if (refinements != NULL) {
        if (errors == NULL) errors = refinements;
        else {
            IsonanticValidationError* curr = refinements->head;
            while (curr != NULL) {
                isonantic_validation_errors_add(errors, curr);
                curr = curr->next;
            }
            free(refinements);
        }
    }

    return errors;
}

static void string_free_schema(IsonanticSchema* schema) {
    if (schema == NULL) return;
    IsonanticStringSchema* s = (IsonanticStringSchema*)schema;
    free(s->min_len);
    free(s->max_len);
    free(s->exact_len);
    if (s->pattern != NULL) {
        regfree((regex_t*)s->pattern);
        free(s->pattern);
    }
    if (s->base.refinements != NULL) {
        int size = isonantic_array_size(s->base.refinements);
        for (int i = 0; i < size; i++) {
            free(isonantic_array_get(s->base.refinements, i));
        }
        isonantic_array_free(s->base.refinements);
    }
    free(s->base.description);
    free(s);
}

IsonanticSchema* isonantic_string_create(void) {
    IsonanticStringSchema* schema = (IsonanticStringSchema*)malloc(sizeof(IsonanticStringSchema));
    if (schema == NULL) return NULL;
    memset(schema, 0, sizeof(IsonanticStringSchema));
    schema->base.validate = string_validate;
    schema->base.free_schema = string_free_schema;
    return (IsonanticSchema*)schema;
}

IsonanticSchema* isonantic_string_min(IsonanticSchema* schema, int n) {
    if (schema == NULL) return NULL;
    IsonanticStringSchema* s = (IsonanticStringSchema*)schema;
    s->min_len = (int*)malloc(sizeof(int));
    *s->min_len = n;
    return schema;
}

IsonanticSchema* isonantic_string_max(IsonanticSchema* schema, int n) {
    if (schema == NULL) return NULL;
    IsonanticStringSchema* s = (IsonanticStringSchema*)schema;
    s->max_len = (int*)malloc(sizeof(int));
    *s->max_len = n;
    return schema;
}

IsonanticSchema* isonantic_string_length(IsonanticSchema* schema, int n) {
    if (schema == NULL) return NULL;
    IsonanticStringSchema* s = (IsonanticStringSchema*)schema;
    s->exact_len = (int*)malloc(sizeof(int));
    *s->exact_len = n;
    return schema;
}

IsonanticSchema* isonantic_string_email(IsonanticSchema* schema) {
    if (schema == NULL) return NULL;
    ((IsonanticStringSchema*)schema)->is_email = true;
    return schema;
}

IsonanticSchema* isonantic_string_url(IsonanticSchema* schema) {
    if (schema == NULL) return NULL;
    ((IsonanticStringSchema*)schema)->is_url = true;
    return schema;
}

IsonanticSchema* isonantic_string_regex(IsonanticSchema* schema, const char* pattern) {
    if (schema == NULL || pattern == NULL) return NULL;
    IsonanticStringSchema* s = (IsonanticStringSchema*)schema;
    regex_t* regex = (regex_t*)malloc(sizeof(regex_t));
    if (regex == NULL) return schema;
    if (regcomp(regex, pattern, REG_EXTENDED) != 0) {
        free(regex);
        return schema;
    }
    s->pattern = regex;
    return schema;
}

IsonanticSchema* isonantic_string_optional(IsonanticSchema* schema) {
    if (schema == NULL) return NULL;
    isonantic_schema_set_optional(schema);
    return schema;
}

IsonanticSchema* isonantic_string_default(IsonanticSchema* schema, const char* val) {
    if (schema == NULL) return NULL;
    isonantic_schema_set_default(schema, isonantic_value_create_string(val));
    return schema;
}

IsonanticSchema* isonantic_string_describe(IsonanticSchema* schema, const char* desc) {
    if (schema == NULL) return NULL;
    isonantic_schema_set_description(schema, desc);
    return schema;
}

static IsonanticValidationErrors* string_refine_fn(IsonanticValue* value, void* user_data) {
    if (value == NULL || value->type != ISONANTIC_VALUE_STRING) return NULL;
    bool (*fn)(const char*) = (bool (*)(const char*))user_data;
    if (fn == NULL) return NULL;
    if (fn(value->data.string_value)) return NULL;

    IsonanticValidationErrors* errors = isonantic_validation_errors_create();
    IsonanticValidationError* err = isonantic_validation_error_create("", "", value);
    isonantic_validation_errors_add(errors, err);
    return errors;
}

IsonanticSchema* isonantic_string_refine(IsonanticSchema* schema, bool (*fn)(const char*), const char* msg) {
    if (schema == NULL) return NULL;
    isonantic_schema_add_refinement(schema, string_refine_fn, (void*)fn, msg);
    return schema;
}

/* ==================== Number Schema ==================== */

static IsonanticValidationErrors* number_validate(IsonanticSchema* schema, IsonanticValue* value) {
    IsonanticNumberSchema* s = (IsonanticNumberSchema*)schema;
    IsonanticValidationErrors* errors = NULL;

    if (value == NULL) {
        if (s->base.optional) {
            return NULL;
        }
        errors = isonantic_validation_errors_create();
        IsonanticValidationError* err = isonantic_validation_error_create(
            "", "required field is missing", NULL);
        isonantic_validation_errors_add(errors, err);
        return errors;
    }

    double num = 0;
    bool valid = false;

    switch (value->type) {
        case ISONANTIC_VALUE_NUMBER:
            num = value->data.number_value;
            valid = true;
            break;
        default:
            break;
    }

    if (!valid) {
        errors = isonantic_validation_errors_create();
        char msg[64];
        snprintf(msg, sizeof(msg), "expected number, got %d", value->type);
        IsonanticValidationError* err = isonantic_validation_error_create("", msg, value);
        isonantic_validation_errors_add(errors, err);
        return errors;
    }

    if (s->is_int && fmod(num, 1.0) != 0.0) {
        if (errors == NULL) errors = isonantic_validation_errors_create();
        IsonanticValidationError* err = isonantic_validation_error_create("", "expected integer, got float", value);
        isonantic_validation_errors_add(errors, err);
    }

    if (s->min_val != NULL && num < *s->min_val) {
        if (errors == NULL) errors = isonantic_validation_errors_create();
        char msg[64];
        snprintf(msg, sizeof(msg), "number must be at least %g", *s->min_val);
        IsonanticValidationError* err = isonantic_validation_error_create("", msg, value);
        isonantic_validation_errors_add(errors, err);
    }

    if (s->max_val != NULL && num > *s->max_val) {
        if (errors == NULL) errors = isonantic_validation_errors_create();
        char msg[64];
        snprintf(msg, sizeof(msg), "number must be at most %g", *s->max_val);
        IsonanticValidationError* err = isonantic_validation_error_create("", msg, value);
        isonantic_validation_errors_add(errors, err);
    }

    if (s->is_positive && num <= 0) {
        if (errors == NULL) errors = isonantic_validation_errors_create();
        IsonanticValidationError* err = isonantic_validation_error_create("", "number must be positive", value);
        isonantic_validation_errors_add(errors, err);
    }

    if (s->is_negative && num >= 0) {
        if (errors == NULL) errors = isonantic_validation_errors_create();
        IsonanticValidationError* err = isonantic_validation_error_create("", "number must be negative", value);
        isonantic_validation_errors_add(errors, err);
    }

    /* Run refinements */
    IsonanticValidationErrors* refinements = isonantic_schema_run_refinements(schema, value);
    if (refinements != NULL) {
        if (errors == NULL) errors = refinements;
        else {
            IsonanticValidationError* curr = refinements->head;
            while (curr != NULL) {
                isonantic_validation_errors_add(errors, curr);
                curr = curr->next;
            }
            free(refinements);
        }
    }

    return errors;
}

static void number_free_schema(IsonanticSchema* schema) {
    if (schema == NULL) return;
    IsonanticNumberSchema* s = (IsonanticNumberSchema*)schema;
    free(s->min_val);
    free(s->max_val);
    if (s->base.refinements != NULL) {
        int size = isonantic_array_size(s->base.refinements);
        for (int i = 0; i < size; i++) {
            free(isonantic_array_get(s->base.refinements, i));
        }
        isonantic_array_free(s->base.refinements);
    }
    free(s->base.description);
    free(s);
}

IsonanticSchema* isonantic_number_create(void) {
    IsonanticNumberSchema* schema = (IsonanticNumberSchema*)malloc(sizeof(IsonanticNumberSchema));
    if (schema == NULL) return NULL;
    memset(schema, 0, sizeof(IsonanticNumberSchema));
    schema->base.validate = number_validate;
    schema->base.free_schema = number_free_schema;
    return (IsonanticSchema*)schema;
}

IsonanticSchema* isonantic_int_create(void) {
    IsonanticSchema* schema = isonantic_number_create();
    if (schema == NULL) return NULL;
    ((IsonanticNumberSchema*)schema)->is_int = true;
    return schema;
}

IsonanticSchema* isonantic_number_min(IsonanticSchema* schema, double n) {
    if (schema == NULL) return NULL;
    IsonanticNumberSchema* s = (IsonanticNumberSchema*)schema;
    s->min_val = (double*)malloc(sizeof(double));
    *s->min_val = n;
    return schema;
}

IsonanticSchema* isonantic_number_max(IsonanticSchema* schema, double n) {
    if (schema == NULL) return NULL;
    IsonanticNumberSchema* s = (IsonanticNumberSchema*)schema;
    s->max_val = (double*)malloc(sizeof(double));
    *s->max_val = n;
    return schema;
}

IsonanticSchema* isonantic_number_positive(IsonanticSchema* schema) {
    if (schema == NULL) return NULL;
    ((IsonanticNumberSchema*)schema)->is_positive = true;
    return schema;
}

IsonanticSchema* isonantic_number_negative(IsonanticSchema* schema) {
    if (schema == NULL) return NULL;
    ((IsonanticNumberSchema*)schema)->is_negative = true;
    return schema;
}

IsonanticSchema* isonantic_number_optional(IsonanticSchema* schema) {
    if (schema == NULL) return NULL;
    isonantic_schema_set_optional(schema);
    return schema;
}

IsonanticSchema* isonantic_number_default(IsonanticSchema* schema, double val) {
    if (schema == NULL) return NULL;
    isonantic_schema_set_default(schema, isonantic_value_create_number(val));
    return schema;
}

IsonanticSchema* isonantic_number_describe(IsonanticSchema* schema, const char* desc) {
    if (schema == NULL) return NULL;
    isonantic_schema_set_description(schema, desc);
    return schema;
}

static IsonanticValidationErrors* number_refine_fn(IsonanticValue* value, void* user_data) {
    if (value == NULL || value->type != ISONANTIC_VALUE_NUMBER) return NULL;
    bool (*fn)(double) = (bool (*)(double))user_data;
    if (fn == NULL) return NULL;
    if (fn(value->data.number_value)) return NULL;

    IsonanticValidationErrors* errors = isonantic_validation_errors_create();
    IsonanticValidationError* err = isonantic_validation_error_create("", "", value);
    isonantic_validation_errors_add(errors, err);
    return errors;
}

IsonanticSchema* isonantic_number_refine(IsonanticSchema* schema, bool (*fn)(double), const char* msg) {
    if (schema == NULL) return NULL;
    isonantic_schema_add_refinement(schema, number_refine_fn, (void*)fn, msg);
    return schema;
}

/* ==================== Boolean Schema ==================== */

static IsonanticValidationErrors* boolean_validate(IsonanticSchema* schema, IsonanticValue* value) {
    IsonanticValidationErrors* errors = NULL;

    if (value == NULL) {
        if (schema->optional) {
            return NULL;
        }
        errors = isonantic_validation_errors_create();
        IsonanticValidationError* err = isonantic_validation_error_create(
            "", "required field is missing", NULL);
        isonantic_validation_errors_add(errors, err);
        return errors;
    }

    if (value->type != ISONANTIC_VALUE_BOOLEAN) {
        errors = isonantic_validation_errors_create();
        char msg[64];
        snprintf(msg, sizeof(msg), "expected boolean, got %d", value->type);
        IsonanticValidationError* err = isonantic_validation_error_create("", msg, value);
        isonantic_validation_errors_add(errors, err);
        return errors;
    }

    return isonantic_schema_run_refinements(schema, value);
}

static void boolean_free_schema(IsonanticSchema* schema) {
    if (schema == NULL) return;
    IsonanticBooleanSchema* s = (IsonanticBooleanSchema*)schema;
    if (s->base.refinements != NULL) {
        int size = isonantic_array_size(s->base.refinements);
        for (int i = 0; i < size; i++) {
            free(isonantic_array_get(s->base.refinements, i));
        }
        isonantic_array_free(s->base.refinements);
    }
    free(s->base.description);
    free(s);
}

IsonanticSchema* isonantic_boolean_create(void) {
    IsonanticBooleanSchema* schema = (IsonanticBooleanSchema*)malloc(sizeof(IsonanticBooleanSchema));
    if (schema == NULL) return NULL;
    memset(schema, 0, sizeof(IsonanticBooleanSchema));
    schema->base.validate = boolean_validate;
    schema->base.free_schema = boolean_free_schema;
    return (IsonanticSchema*)schema;
}

IsonanticSchema* isonantic_boolean_optional(IsonanticSchema* schema) {
    if (schema == NULL) return NULL;
    isonantic_schema_set_optional(schema);
    return schema;
}

IsonanticSchema* isonantic_boolean_default(IsonanticSchema* schema, bool val) {
    if (schema == NULL) return NULL;
    isonantic_schema_set_default(schema, isonantic_value_create_boolean(val));
    return schema;
}

IsonanticSchema* isonantic_boolean_describe(IsonanticSchema* schema, const char* desc) {
    if (schema == NULL) return NULL;
    isonantic_schema_set_description(schema, desc);
    return schema;
}

/* ==================== Null Schema ==================== */

static IsonanticValidationErrors* null_validate(IsonanticSchema* schema, IsonanticValue* value) {
    (void)schema;
    if (value != NULL && value->type != ISONANTIC_VALUE_NULL) {
        IsonanticValidationErrors* errors = isonantic_validation_errors_create();
        char msg[64];
        snprintf(msg, sizeof(msg), "expected null, got %d", value->type);
        IsonanticValidationError* err = isonantic_validation_error_create("", msg, value);
        isonantic_validation_errors_add(errors, err);
        return errors;
    }
    return NULL;
}

static void null_free_schema(IsonanticSchema* schema) {
    if (schema == NULL) return;
    free(schema);
}

IsonanticSchema* isonantic_null_create(void) {
    IsonanticSchema* schema = (IsonanticSchema*)malloc(sizeof(IsonanticSchema));
    if (schema == NULL) return NULL;
    memset(schema, 0, sizeof(IsonanticSchema));
    schema->validate = null_validate;
    schema->free_schema = null_free_schema;
    return schema;
}

/* ==================== Reference Schema ==================== */

static bool is_valid_ref(const char* str) {
    return str != NULL && str[0] == ':';
}

static IsonanticValidationErrors* ref_validate(IsonanticSchema* schema, IsonanticValue* value) {
    IsonanticRefSchema* s = (IsonanticRefSchema*)schema;
    IsonanticValidationErrors* errors = NULL;

    if (value == NULL) {
        if (s->base.optional) {
            return NULL;
        }
        errors = isonantic_validation_errors_create();
        IsonanticValidationError* err = isonantic_validation_error_create(
            "", "required field is missing", NULL);
        isonantic_validation_errors_add(errors, err);
        return errors;
    }

    switch (value->type) {
        case ISONANTIC_VALUE_REFERENCE:
            if (!is_valid_ref(value->data.ref_value)) {
                errors = isonantic_validation_errors_create();
                IsonanticValidationError* err = isonantic_validation_error_create(
                    "", "expected reference string starting with ':'", value);
                isonantic_validation_errors_add(errors, err);
            }
            break;

        case ISONANTIC_VALUE_OBJECT: {
            IsonanticDict* dict = value->data.object_value;
            char* ref_str = (char*)isonantic_dict_get(dict, "_ref");
            if (ref_str == NULL) {
                errors = isonantic_validation_errors_create();
                IsonanticValidationError* err = isonantic_validation_error_create(
                    "", "expected reference object with _ref field", value);
                isonantic_validation_errors_add(errors, err);
            } else if (s->ns != NULL) {
                char* ns = (char*)isonantic_dict_get(dict, "_namespace");
                if (ns == NULL || strcmp(ns, s->ns) != 0) {
                    errors = isonantic_validation_errors_create();
                    char msg[128];
                    snprintf(msg, sizeof(msg), "expected namespace %s", s->ns);
                    IsonanticValidationError* err = isonantic_validation_error_create("", msg, value);
                    isonantic_validation_errors_add(errors, err);
                }
            } else if (s->relationship != NULL) {
                char* rel = (char*)isonantic_dict_get(dict, "_relationship");
                if (rel == NULL || strcmp(rel, s->relationship) != 0) {
                    errors = isonantic_validation_errors_create();
                    char msg[128];
                    snprintf(msg, sizeof(msg), "expected relationship %s", s->relationship);
                    IsonanticValidationError* err = isonantic_validation_error_create("", msg, value);
                    isonantic_validation_errors_add(errors, err);
                }
            }
            break;
        }

        default: {
            errors = isonantic_validation_errors_create();
            char msg[64];
            snprintf(msg, sizeof(msg), "expected reference, got %d", value->type);
            IsonanticValidationError* err = isonantic_validation_error_create("", msg, value);
            isonantic_validation_errors_add(errors, err);
            break;
        }
    }

    return isonantic_schema_run_refinements(schema, value);
}

static void ref_free_schema(IsonanticSchema* schema) {
    if (schema == NULL) return;
    IsonanticRefSchema* s = (IsonanticRefSchema*)schema;
    free(s->ns);
    free(s->relationship);
    if (s->base.refinements != NULL) {
        int size = isonantic_array_size(s->base.refinements);
        for (int i = 0; i < size; i++) {
            free(isonantic_array_get(s->base.refinements, i));
        }
        isonantic_array_free(s->base.refinements);
    }
    free(s->base.description);
    free(s);
}

IsonanticSchema* isonantic_ref_create(void) {
    IsonanticRefSchema* schema = (IsonanticRefSchema*)malloc(sizeof(IsonanticRefSchema));
    if (schema == NULL) return NULL;
    memset(schema, 0, sizeof(IsonanticRefSchema));
    schema->base.validate = ref_validate;
    schema->base.free_schema = ref_free_schema;
    return (IsonanticSchema*)schema;
}

IsonanticSchema* isonantic_ref_namespace(IsonanticSchema* schema, const char* ns) {
    if (schema == NULL) return NULL;
    IsonanticRefSchema* s = (IsonanticRefSchema*)schema;
    s->ns = strdup(ns);
    return schema;
}

IsonanticSchema* isonantic_ref_relationship(IsonanticSchema* schema, const char* rel) {
    if (schema == NULL) return NULL;
    IsonanticRefSchema* s = (IsonanticRefSchema*)schema;
    s->relationship = strdup(rel);
    return schema;
}

IsonanticSchema* isonantic_ref_optional(IsonanticSchema* schema) {
    if (schema == NULL) return NULL;
    isonantic_schema_set_optional(schema);
    return schema;
}

IsonanticSchema* isonantic_ref_describe(IsonanticSchema* schema, const char* desc) {
    if (schema == NULL) return NULL;
    isonantic_schema_set_description(schema, desc);
    return schema;
}

/* ==================== Free Functions ==================== */

void isonantic_schema_free(IsonanticSchema* schema) {
    if (schema == NULL) return;
    if (schema->free_schema != NULL) {
        schema->free_schema(schema);
    } else {
        free(schema);
    }
}
