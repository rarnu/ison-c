/**
 * @file isonantic.h
 * @brief Zod-like validation and type-safe schemas for ISON format in C.
 */

#ifndef ISONANTIC_H
#define ISONANTIC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Version */
#define ISONANTIC_VERSION "1.0.0"

/* Forward declarations */
typedef struct IsonanticSchema IsonanticSchema;
typedef struct IsonanticValidationError IsonanticValidationError;
typedef struct IsonanticValidationErrors IsonanticValidationErrors;
typedef struct IsonanticSafeParseResult IsonanticSafeParseResult;
typedef struct IsonanticDict IsonanticDict;
typedef struct IsonanticArray IsonanticArray;
typedef struct IsonanticDictEntry IsonanticDictEntry;

/* Value types for validation */
typedef enum {
    ISONANTIC_VALUE_NULL,
    ISONANTIC_VALUE_STRING,
    ISONANTIC_VALUE_NUMBER,
    ISONANTIC_VALUE_BOOLEAN,
    ISONANTIC_VALUE_OBJECT,
    ISONANTIC_VALUE_ARRAY,
    ISONANTIC_VALUE_REFERENCE
} IsonanticValueType;

/* Generic value container */
typedef struct {
    IsonanticValueType type;
    union {
        char* string_value;
        double number_value;
        bool boolean_value;
        IsonanticDict* object_value;
        IsonanticArray* array_value;
        char* ref_value;
    } data;
} IsonanticValue;

/* Validation error */
struct IsonanticValidationError {
    char* field;
    char* message;
    IsonanticValue* value;
    IsonanticValidationError* next;
};

/* Collection of validation errors */
struct IsonanticValidationErrors {
    IsonanticValidationError* head;
    IsonanticValidationError* tail;
    int count;
};

/* Safe parse result */
struct IsonanticSafeParseResult {
    bool success;
    IsonanticDict* data;
    IsonanticValidationErrors* error;
};

/* Refinement function type */
typedef IsonanticValidationErrors* (*IsonanticRefinementFn)(IsonanticValue* value, void* user_data);

/* Refinement structure */
typedef struct {
    IsonanticRefinementFn fn;
    void* user_data;
    char* error_message;
} IsonanticRefinement;

/* Dictionary/Map implementation */
struct IsonanticDictEntry {
    char* key;
    void* value;
    struct IsonanticDictEntry* next;
};

struct IsonanticDict {
    struct IsonanticDictEntry** buckets;
    int capacity;
    int size;
};

/* Array implementation */
struct IsonanticArray {
    void** items;
    int size;
    int capacity;
};

/* Base schema structure */
struct IsonanticSchema {
    /* Validation */
    IsonanticValidationErrors* (*validate)(IsonanticSchema* schema, IsonanticValue* value);

    /* Properties */
    bool optional;
    bool has_default;
    IsonanticValue* default_value;
    char* description;

    /* Refinements */
    IsonanticArray* refinements;

    /* Free function */
    void (*free_schema)(IsonanticSchema* schema);
};

/* String schema */
typedef struct {
    IsonanticSchema base;
    int* min_len;
    int* max_len;
    int* exact_len;
    void* pattern;
    bool is_email;
    bool is_url;
} IsonanticStringSchema;

/* Number schema */
typedef struct {
    IsonanticSchema base;
    double* min_val;
    double* max_val;
    bool is_int;
    bool is_positive;
    bool is_negative;
} IsonanticNumberSchema;

/* Boolean schema */
typedef struct {
    IsonanticSchema base;
} IsonanticBooleanSchema;

/* Null schema */
typedef struct {
    IsonanticSchema base;
} IsonanticNullSchema;

/* Reference schema */
typedef struct {
    IsonanticSchema base;
    char* ns;
    char* relationship;
} IsonanticRefSchema;

/* Object schema */
typedef struct {
    IsonanticSchema base;
    IsonanticDict* fields;
} IsonanticObjectSchema;

/* Array schema */
typedef struct {
    IsonanticSchema base;
    IsonanticSchema* item_schema;
    int* min_len;
    int* max_len;
} IsonanticArraySchema;

/* Table schema */
typedef struct {
    IsonanticSchema base;
    char* name;
    IsonanticDict* fields;
    IsonanticSchema* row_schema;
} IsonanticTableSchema;

/* Document schema */
typedef struct {
    IsonanticDict* blocks;
} IsonanticDocumentSchema;

/* I namespace - provides access to all schema builders */
typedef struct {
    IsonanticSchema* (*String)(void);
    IsonanticSchema* (*Number)(void);
    IsonanticSchema* (*Int)(void);
    IsonanticSchema* (*Float)(void);
    IsonanticSchema* (*Boolean)(void);
    IsonanticSchema* (*Bool)(void);
    IsonanticSchema* (*Null)(void);
    IsonanticSchema* (*Ref)(void);
    IsonanticSchema* (*Reference)(void);
    IsonanticSchema* (*Object)(IsonanticDict* fields);
    IsonanticSchema* (*Array)(IsonanticSchema* item_schema);
    IsonanticSchema* (*Table)(const char* name, IsonanticDict* fields);
} IsonanticINamespace;

extern IsonanticINamespace I;

/* ==================== Utility Functions ==================== */

IsonanticValue* isonantic_value_create_string(const char* str);
IsonanticValue* isonantic_value_create_number(double num);
IsonanticValue* isonantic_value_create_boolean(bool val);
IsonanticValue* isonantic_value_create_null(void);
IsonanticValue* isonantic_value_create_ref(const char* ref);
void isonantic_value_free(IsonanticValue* value);

/* ==================== Dictionary Functions ==================== */

IsonanticDict* isonantic_dict_create(int capacity);
void isonantic_dict_set(IsonanticDict* dict, const char* key, void* value);
void* isonantic_dict_get(IsonanticDict* dict, const char* key);
bool isonantic_dict_has_key(IsonanticDict* dict, const char* key);
int isonantic_dict_size(IsonanticDict* dict);
void isonantic_dict_free(IsonanticDict* dict);

/* ==================== Array Functions ==================== */

IsonanticArray* isonantic_array_create(int capacity);
void isonantic_array_add(IsonanticArray* arr, void* item);
void* isonantic_array_get(IsonanticArray* arr, int index);
int isonantic_array_size(IsonanticArray* arr);
void isonantic_array_free(IsonanticArray* arr);

/* ==================== Validation Error Functions ==================== */

IsonanticValidationError* isonantic_validation_error_create(const char* field, const char* message, IsonanticValue* value);
void isonantic_validation_errors_add(IsonanticValidationErrors* errors, IsonanticValidationError* error);
IsonanticValidationErrors* isonantic_validation_errors_create(void);
bool isonantic_validation_errors_has_errors(IsonanticValidationErrors* errors);
int isonantic_validation_errors_count(IsonanticValidationErrors* errors);
char* isonantic_validation_errors_to_string(IsonanticValidationErrors* errors);
void isonantic_validation_errors_free(IsonanticValidationErrors* errors);

/* ==================== Base Schema Functions ==================== */

void isonantic_schema_set_optional(IsonanticSchema* schema);
void isonantic_schema_set_default(IsonanticSchema* schema, IsonanticValue* value);
void isonantic_schema_set_description(IsonanticSchema* schema, const char* desc);
void isonantic_schema_add_refinement(IsonanticSchema* schema, IsonanticRefinementFn fn, void* user_data, const char* message);
IsonanticValidationErrors* isonantic_schema_run_refinements(IsonanticSchema* schema, IsonanticValue* value);

/* ==================== String Schema ==================== */

IsonanticSchema* isonantic_string_create(void);
IsonanticSchema* isonantic_string_min(IsonanticSchema* schema, int n);
IsonanticSchema* isonantic_string_max(IsonanticSchema* schema, int n);
IsonanticSchema* isonantic_string_length(IsonanticSchema* schema, int n);
IsonanticSchema* isonantic_string_email(IsonanticSchema* schema);
IsonanticSchema* isonantic_string_url(IsonanticSchema* schema);
IsonanticSchema* isonantic_string_regex(IsonanticSchema* schema, const char* pattern);
IsonanticSchema* isonantic_string_optional(IsonanticSchema* schema);
IsonanticSchema* isonantic_string_default(IsonanticSchema* schema, const char* val);
IsonanticSchema* isonantic_string_describe(IsonanticSchema* schema, const char* desc);
IsonanticSchema* isonantic_string_refine(IsonanticSchema* schema, bool (*fn)(const char*), const char* msg);

/* ==================== Number Schema ==================== */

IsonanticSchema* isonantic_number_create(void);
IsonanticSchema* isonantic_int_create(void);
IsonanticSchema* isonantic_number_min(IsonanticSchema* schema, double n);
IsonanticSchema* isonantic_number_max(IsonanticSchema* schema, double n);
IsonanticSchema* isonantic_number_positive(IsonanticSchema* schema);
IsonanticSchema* isonantic_number_negative(IsonanticSchema* schema);
IsonanticSchema* isonantic_number_optional(IsonanticSchema* schema);
IsonanticSchema* isonantic_number_default(IsonanticSchema* schema, double val);
IsonanticSchema* isonantic_number_describe(IsonanticSchema* schema, const char* desc);
IsonanticSchema* isonantic_number_refine(IsonanticSchema* schema, bool (*fn)(double), const char* msg);

/* ==================== Boolean Schema ==================== */

IsonanticSchema* isonantic_boolean_create(void);
IsonanticSchema* isonantic_boolean_optional(IsonanticSchema* schema);
IsonanticSchema* isonantic_boolean_default(IsonanticSchema* schema, bool val);
IsonanticSchema* isonantic_boolean_describe(IsonanticSchema* schema, const char* desc);

/* ==================== Null Schema ==================== */

IsonanticSchema* isonantic_null_create(void);

/* ==================== Reference Schema ==================== */

IsonanticSchema* isonantic_ref_create(void);
IsonanticSchema* isonantic_ref_namespace(IsonanticSchema* schema, const char* ns);
IsonanticSchema* isonantic_ref_relationship(IsonanticSchema* schema, const char* rel);
IsonanticSchema* isonantic_ref_optional(IsonanticSchema* schema);
IsonanticSchema* isonantic_ref_describe(IsonanticSchema* schema, const char* desc);

/* ==================== Object Schema ==================== */

IsonanticSchema* isonantic_object_create(IsonanticDict* fields);
IsonanticSchema* isonantic_object_optional(IsonanticSchema* schema);
IsonanticSchema* isonantic_object_describe(IsonanticSchema* schema, const char* desc);
IsonanticSchema* isonantic_object_extend(IsonanticSchema* schema, IsonanticDict* fields);
IsonanticSchema* isonantic_object_pick(IsonanticSchema* schema, int num_keys, const char** keys);
IsonanticSchema* isonantic_object_omit(IsonanticSchema* schema, int num_keys, const char** keys);

/* ==================== Array Schema ==================== */

IsonanticSchema* isonantic_array_create_schema(IsonanticSchema* item_schema);
IsonanticSchema* isonantic_array_min(IsonanticSchema* schema, int n);
IsonanticSchema* isonantic_array_max(IsonanticSchema* schema, int n);
IsonanticSchema* isonantic_array_optional(IsonanticSchema* schema);
IsonanticSchema* isonantic_array_describe(IsonanticSchema* schema, const char* desc);

/* ==================== Table Schema ==================== */

IsonanticSchema* isonantic_table_create(const char* name, IsonanticDict* fields);
IsonanticSchema* isonantic_table_optional(IsonanticSchema* schema);
IsonanticSchema* isonantic_table_describe(IsonanticSchema* schema, const char* desc);
const char* isonantic_table_get_name(IsonanticSchema* schema);

/* ==================== Document Schema ==================== */

IsonanticSchema* isonantic_document_create(IsonanticDict* blocks);
IsonanticDict* isonantic_document_parse(IsonanticSchema* schema, IsonanticDict* value, IsonanticValidationErrors** error);
IsonanticSafeParseResult isonantic_document_safe_parse(IsonanticSchema* schema, IsonanticDict* value);

/* ==================== Free Functions ==================== */

void isonantic_schema_free(IsonanticSchema* schema);

#ifdef __cplusplus
}
#endif

#endif /* ISONANTIC_H */
