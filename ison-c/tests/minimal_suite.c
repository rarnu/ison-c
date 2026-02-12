#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ison.h"

int main(void) {
    printf("Test 1: Version check... ");
    fflush(stdout);
    assert(strcmp(ISON_VERSION, "1.0.0") == 0);
    printf("PASS\n");
    
    printf("Test 2: Value null... ");
    fflush(stdout);
    ison_value_t v = ison_null();
    assert(v.type == ISON_TYPE_NULL);
    printf("PASS\n");
    
    printf("Test 3: Value bool... ");
    fflush(stdout);
    v = ison_bool(1);
    assert(v.type == ISON_TYPE_BOOL);
    printf("PASS\n");
    
    printf("Test 4: Value int... ");
    fflush(stdout);
    v = ison_int(42);
    assert(v.type == ISON_TYPE_INT);
    printf("PASS\n");
    
    printf("Test 5: Value float... ");
    fflush(stdout);
    v = ison_float(3.14);
    assert(v.type == ISON_TYPE_FLOAT);
    printf("PASS\n");
    
    printf("Test 6: Value string... ");
    fflush(stdout);
    v = ison_string("hello");
    assert(v.type == ISON_TYPE_STRING);
    ison_value_free(&v);
    printf("PASS\n");
    
    printf("Test 7: Reference... ");
    fflush(stdout);
    ison_reference_t ref = ison_reference_make("1", NULL, NULL);
    char *s = ison_reference_to_ison(&ref);
    assert(strcmp(s, ":1") == 0);
    free(s);
    ison_reference_free(&ref);
    printf("PASS\n");
    
    printf("Test 8: Document create... ");
    fflush(stdout);
    ison_document_t *doc = ison_document_create();
    assert(doc != NULL);
    printf("PASS\n");
    
    printf("Test 9: Block create... ");
    fflush(stdout);
    ison_block_t *block = ison_block_create("table", "users");
    assert(block != NULL);
    printf("PASS\n");
    
    printf("Test 10: Add field... ");
    fflush(stdout);
    ison_block_add_field(block, "id", "int");
    assert(block->field_count == 1);
    printf("PASS\n");
    
    printf("Test 11: Add row... ");
    fflush(stdout);
    ison_row_t *row = ison_row_create();
    ison_value_t val = ison_int(1);
    ison_row_set(row, "id", &val);
    ison_block_add_row(block, row);
    ison_row_free(row);
    assert(block->row_count == 1);
    printf("PASS\n");
    
    printf("Test 12: Add block to doc... ");
    fflush(stdout);
    ison_document_add_block(doc, block);
    assert(doc->block_count == 1);
    printf("PASS\n");
    
    printf("Test 13: Get block from doc... ");
    fflush(stdout);
    ison_block_t *retrieved = ison_document_get(doc, "users");
    assert(retrieved != NULL);
    assert(retrieved->field_count == 1);
    printf("PASS\n");
    
    printf("Test 14: Free doc... ");
    fflush(stdout);
    ison_document_free(doc);
    printf("PASS\n");
    
    printf("\nAll tests passed!\n");
    return 0;
}
