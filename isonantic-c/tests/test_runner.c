/**
 * @file test_runner.c
 * @brief Test runner for isonantic-c.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "isonantic.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT_TRUE(cond) do { \
    if (cond) { \
        tests_passed++; \
        printf("  [PASS] %s\n", #cond); \
    } else { \
        tests_failed++; \
        printf("  [FAIL] %s (line %d)\n", #cond, __LINE__); \
    } \
} while(0)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))
#define ASSERT_NOT_NULL(ptr) ASSERT_TRUE((ptr) != NULL)
#define ASSERT_NULL(ptr) ASSERT_TRUE((ptr) == NULL)
#define ASSERT_EQUAL_INT(a, b) ASSERT_TRUE((a) == (b)))
#define ASSERT_CONTAINS(str, substr) ASSERT_TRUE(strstr(str, substr) != NULL)
#define ASSERT_EQUAL_STRING(a, b) ASSERT_TRUE(strcmp(a, b) == 0)

/* String Schema Tests */

static void test_string_required(void) {
    printf("Test: String Required\n");
    IsonanticSchema* schema = isonantic_string_create();

    IsonanticValue* val = isonantic_value_create_string("hello");
    IsonanticValidationErrors* err = schema->validate(schema, val);
    ASSERT_NULL(err);
    if (err) isonantic_validation_errors_free(err);
    isonantic_value_free(val);

    IsonanticValue* nil_val = NULL;
    err = schema->validate(schema, nil_val);
    ASSERT_NOT_NULL(err);
    if (err) isonantic_validation_errors_free(err);

    isonantic_schema_free(schema);
}

static void test_string_optional(void) {
    printf("Test: String Optional\n");
    IsonanticSchema* schema = isonantic_string_create();
    isonantic_string_optional(schema);

    IsonanticValue* nil_val = NULL;
    IsonanticValidationErrors* err = schema->validate(schema, nil_val);
    ASSERT_NULL(err);
    if (err) isonantic_validation_errors_free(err);

    isonantic_value_free(NULL);
    isonantic_schema_free(schema);
}

static void test_string_min_length(void) {
    printf("Test: String Min Length\n");
    IsonanticSchema* schema = isonantic_string_create();
    isonantic_string_min(schema, 5);

    IsonanticValue* val = isonantic_value_create_string("hello");
    IsonanticValidationErrors* err = schema->validate(schema, val);
    ASSERT_NULL(err);
    if (err) isonantic_validation_errors_free(err);
    isonantic_value_free(val);

    IsonanticValue* short_val = isonantic_value_create_string("hi");
    err = schema->validate(schema, short_val);
    ASSERT_NOT_NULL(err);
    if (err) {
        char* msg = isonantic_validation_errors_to_string(err);
        ASSERT_CONTAINS(msg, "at least 5");
        free(msg);
        isonantic_validation_errors_free(err);
    }
    isonantic_value_free(short_val);

    isonantic_schema_free(schema);
}

static void test_string_email(void) {
    printf("Test: String Email\n");
    IsonanticSchema* schema = isonantic_string_create();
    isonantic_string_email(schema);

    IsonanticValue* valid = isonantic_value_create_string("test@example.com");
    IsonanticValidationErrors* err = schema->validate(schema, valid);
    ASSERT_NULL(err);
    if (err) isonantic_validation_errors_free(err);
    isonantic_value_free(valid);

    IsonanticValue* invalid = isonantic_value_create_string("invalid-email");
    err = schema->validate(schema, invalid);
    ASSERT_NOT_NULL(err);
    if (err) isonantic_validation_errors_free(err);
    isonantic_value_free(invalid);

    isonantic_schema_free(schema);
}

/* Number Schema Tests */

static void test_number_required(void) {
    printf("Test: Number Required\n");
    IsonanticSchema* schema = isonantic_number_create();

    IsonanticValue* val = isonantic_value_create_number(42.5);
    IsonanticValidationErrors* err = schema->validate(schema, val);
    ASSERT_NULL(err);
    if (err) isonantic_validation_errors_free(err);
    isonantic_value_free(val);

    IsonanticValue* nil_val = NULL;
    err = schema->validate(schema, nil_val);
    ASSERT_NOT_NULL(err);
    if (err) isonantic_validation_errors_free(err);

    isonantic_schema_free(schema);
}

static void test_int_schema(void) {
    printf("Test: Int Schema\n");
    IsonanticSchema* schema = isonantic_int_create();

    IsonanticValue* val = isonantic_value_create_number(42.0);
    IsonanticValidationErrors* err = schema->validate(schema, val);
    ASSERT_NULL(err);
    if (err) isonantic_validation_errors_free(err);
    isonantic_value_free(val);

    IsonanticValue* float_val = isonantic_value_create_number(42.5);
    err = schema->validate(schema, float_val);
    ASSERT_NOT_NULL(err);
    if (err) {
        char* msg = isonantic_validation_errors_to_string(err);
        ASSERT_CONTAINS(msg, "expected integer");
        free(msg);
        isonantic_validation_errors_free(err);
    }
    isonantic_value_free(float_val);

    isonantic_schema_free(schema);
}

static void test_number_positive(void) {
    printf("Test: Number Positive\n");
    IsonanticSchema* schema = isonantic_number_create();
    isonantic_number_positive(schema);

    IsonanticValue* val = isonantic_value_create_number(5.0);
    IsonanticValidationErrors* err = schema->validate(schema, val);
    ASSERT_NULL(err);
    if (err) isonantic_validation_errors_free(err);
    isonantic_value_free(val);

    IsonanticValue* zero_val = isonantic_value_create_number(0.0);
    err = schema->validate(schema, zero_val);
    ASSERT_NOT_NULL(err);
    if (err) {
        char* msg = isonantic_validation_errors_to_string(err);
        ASSERT_CONTAINS(msg, "positive");
        free(msg);
        isonantic_validation_errors_free(err);
    }
    isonantic_value_free(zero_val);

    isonantic_schema_free(schema);
}

/* Boolean Schema Tests */

static void test_boolean_required(void) {
    printf("Test: Boolean Required\n");
    IsonanticSchema* schema = isonantic_boolean_create();

    IsonanticValue* val = isonantic_value_create_boolean(true);
    IsonanticValidationErrors* err = schema->validate(schema, val);
    ASSERT_NULL(err);
    if (err) isonantic_validation_errors_free(err);
    isonantic_value_free(val);

    IsonanticValue* nil_val = NULL;
    err = schema->validate(schema, nil_val);
    ASSERT_NOT_NULL(err);
    if (err) isonantic_validation_errors_free(err);

    isonantic_schema_free(schema);
}

/* Reference Schema Tests */

static void test_ref_required(void) {
    printf("Test: Ref Required\n");
    IsonanticSchema* schema = isonantic_ref_create();

    IsonanticValue* val = isonantic_value_create_ref(":1");
    IsonanticValidationErrors* err = schema->validate(schema, val);
    ASSERT_NULL(err);
    if (err) isonantic_validation_errors_free(err);
    isonantic_value_free(val);

    IsonanticValue* nil_val = NULL;
    err = schema->validate(schema, nil_val);
    ASSERT_NOT_NULL(err);
    if (err) isonantic_validation_errors_free(err);

    isonantic_schema_free(schema);
}

/* Version Test */

static void test_version(void) {
    printf("Test: Version\n");
    ASSERT_EQUAL_STRING(ISONANTIC_VERSION, "1.0.0");
}

/* Main */

int main(void) {
    printf("Running isonantic-c tests...\n\n");

    test_version();
    test_string_required();
    test_string_optional();
    test_string_min_length();
    test_string_email();
    test_number_required();
    test_int_schema();
    test_number_positive();
    test_boolean_required();
    test_ref_required();

    printf("\n=== Results ===\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("\n");

    return tests_failed > 0 ? 1 : 0;
}
