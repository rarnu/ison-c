/**
 * ison.h - ISON (Interchange Simple Object Notation) C Library
 * 
 * A minimal, token-efficient data format optimized for LLMs and Agentic AI workflows.
 * 
 * Version: 1.0.0
 * License: MIT
 */

#ifndef ISON_H
#define ISON_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Version */
#define ISON_VERSION "1.0.0"

/* Error codes */
typedef enum {
    ISON_OK = 0,
    ISON_ERROR_MEMORY = -1,
    ISON_ERROR_PARSE = -2,
    ISON_ERROR_IO = -3,
    ISON_ERROR_INVALID = -4
} ison_error_t;

/* Value types */
typedef enum {
    ISON_TYPE_NULL = 0,
    ISON_TYPE_BOOL,
    ISON_TYPE_INT,
    ISON_TYPE_FLOAT,
    ISON_TYPE_STRING,
    ISON_TYPE_REFERENCE
} ison_type_t;

/* Reference structure */
typedef struct {
    char *id;
    char *ns;
    char *relationship;
} ison_reference_t;

/* Value structure */
typedef struct {
    ison_type_t type;
    union {
        bool bool_val;
        int64_t int_val;
        double float_val;
        char *string_val;
        ison_reference_t ref_val;
    } data;
} ison_value_t;

/* Field information */
typedef struct {
    char *name;
    char *type_hint;  /* "int", "float", "bool", "string", "ref", or "" */
} ison_field_info_t;

/* Row - linked list of field-value pairs */
typedef struct ison_row_entry {
    char *key;
    ison_value_t value;
    struct ison_row_entry *next;
} ison_row_entry_t;

typedef struct {
    ison_row_entry_t *head;
    ison_row_entry_t *tail;
    size_t count;
} ison_row_t;

/* Block - table, object, or meta */
typedef struct {
    char *kind;        /* "table", "object", or "meta" */
    char *name;
    ison_field_info_t *fields;
    size_t field_count;
    size_t field_capacity;
    ison_row_t **rows;
    size_t row_count;
    size_t row_capacity;
    ison_row_t *summary_row;
} ison_block_t;

/* Document */
typedef struct {
    ison_block_t **blocks;
    size_t block_count;
    size_t block_capacity;
    char **order;
    size_t order_count;
} ison_document_t;

/* Serialization options */
typedef struct {
    bool align_columns;
    char *delimiter;   /* default: " " */
} ison_dumps_options_t;

/* FromDict options */
typedef struct {
    bool auto_refs;
    bool smart_order;
} ison_fromdict_options_t;

/* ISONL record for streaming */
typedef struct {
    char *kind;
    char *name;
    char **fields;
    size_t field_count;
    ison_value_t *values;
} isonl_record_t;

/* Callback for ISONL streaming */
typedef void (*isonl_callback_t)(const isonl_record_t *record, void *userdata);

/* ==================== Value Constructors ==================== */

ison_value_t ison_null(void);
ison_value_t ison_bool(bool value);
ison_value_t ison_int(int64_t value);
ison_value_t ison_float(double value);
ison_value_t ison_string(const char *value);
ison_value_t ison_string_n(const char *value, size_t len);
ison_value_t ison_ref(const ison_reference_t *ref);

/* ==================== Value Accessors ==================== */

bool ison_value_is_null(const ison_value_t *value);
bool ison_value_as_bool(const ison_value_t *value, bool *out);
bool ison_value_as_int(const ison_value_t *value, int64_t *out);
bool ison_value_as_float(const ison_value_t *value, double *out);
bool ison_value_as_string(const ison_value_t *value, const char **out);
bool ison_value_as_ref(const ison_value_t *value, ison_reference_t *out);

/* ==================== Value Operations ==================== */

char *ison_value_to_ison(const ison_value_t *value);
char *ison_value_to_json(const ison_value_t *value);
void ison_value_free(ison_value_t *value);

/* ==================== Reference Operations ==================== */

ison_reference_t ison_reference_make(const char *id, const char *ns, const char *relationship);
char *ison_reference_to_ison(const ison_reference_t *ref);
bool ison_reference_is_relationship(const ison_reference_t *ref);
char *ison_reference_get_ns(const ison_reference_t *ref);
void ison_reference_free(ison_reference_t *ref);

/* ==================== Row Operations ==================== */

ison_row_t *ison_row_create(void);
void ison_row_set(ison_row_t *row, const char *key, const ison_value_t *value);
bool ison_row_get(const ison_row_t *row, const char *key, ison_value_t *out);
ison_value_t *ison_row_get_ptr(const ison_row_t *row, const char *key);
void ison_row_free(ison_row_t *row);

/* ==================== Block Operations ==================== */

ison_block_t *ison_block_create(const char *kind, const char *name);
void ison_block_add_field(ison_block_t *block, const char *name, const char *type_hint);
void ison_block_add_row(ison_block_t *block, const ison_row_t *row);
void ison_block_set_summary(ison_block_t *block, const ison_row_t *row);
char **ison_block_get_field_names(const ison_block_t *block, size_t *count);
void ison_block_free(ison_block_t *block);

/* ==================== Document Operations ==================== */

ison_document_t *ison_document_create(void);
void ison_document_add_block(ison_document_t *doc, ison_block_t *block);
ison_block_t *ison_document_get(const ison_document_t *doc, const char *name);
const char **ison_document_get_order(const ison_document_t *doc, size_t *count);
void ison_document_free(ison_document_t *doc);

/* ==================== Parsing ==================== */

ison_document_t *ison_parse(const char *text, ison_error_t *error);
ison_document_t *ison_parse_isonl(const char *text, ison_error_t *error);

/* ==================== Serialization ==================== */

char *ison_dumps(const ison_document_t *doc);
char *ison_dumps_with_options(const ison_document_t *doc, const ison_dumps_options_t *options);
char *ison_dumps_isonl(const ison_document_t *doc);

/* ==================== File I/O ==================== */

ison_document_t *ison_load(const char *path, ison_error_t *error);
ison_error_t ison_dump(const ison_document_t *doc, const char *path);
ison_document_t *ison_load_isonl(const char *path, ison_error_t *error);
ison_error_t ison_dump_isonl(const ison_document_t *doc, const char *path);

/* ==================== Format Conversion ==================== */

char *ison_to_isonl(const char *ison_text, ison_error_t *error);
char *isonl_to_ison(const char *isonl_text, ison_error_t *error);
char *ison_to_json(const char *ison_text, ison_error_t *error);
ison_document_t *ison_from_json(const char *json_text, ison_error_t *error);

/* ==================== Streaming ==================== */

ison_error_t isonl_stream_file(const char *path, isonl_callback_t callback, void *userdata);
ison_error_t isonl_stream_buffer(const char *buffer, size_t len, isonl_callback_t callback, void *userdata);

/* ==================== Utility ==================== */

char *ison_read_file(const char *path, size_t *out_len);
ison_error_t ison_write_file(const char *path, const char *content);

/* Default options */
ison_dumps_options_t ison_default_dumps_options(void);
ison_fromdict_options_t ison_default_fromdict_options(void);

/* Error string */
const char *ison_error_string(ison_error_t error);

#ifdef __cplusplus
}
#endif

#endif /* ISON_H */
