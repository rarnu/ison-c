#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ison.h"

int main(void) {
    printf("Test: ISON Parse Simple Table... ");
    fflush(stdout);
    
    const char *input = 
        "table.users\n"
        "id name email\n"
        "1 Alice alice@example.com\n"
        "2 Bob bob@example.com\n";
    
    ison_error_t err;
    ison_document_t *doc = ison_parse(input, &err);
    assert(doc != NULL);
    assert(err == ISON_OK);
    
    ison_block_t *block = ison_document_get(doc, "users");
    assert(block != NULL);
    assert(strcmp(block->kind, "table") == 0);
    assert(block->field_count == 3);
    assert(block->row_count == 2);
    
    ison_document_free(doc);
    printf("PASS\n");
    
    printf("Test: ISON Parse Typed Fields... ");
    fflush(stdout);
    
    const char *input2 = 
        "table.users\n"
        "id:int name:string active:bool\n"
        "1 Alice true\n"
        "2 Bob false\n";
    
    doc = ison_parse(input2, &err);
    assert(doc != NULL);
    
    block = ison_document_get(doc, "users");
    assert(strcmp(block->fields[0].type_hint, "int") == 0);
    assert(strcmp(block->fields[1].type_hint, "string") == 0);
    assert(strcmp(block->fields[2].type_hint, "bool") == 0);
    
    ison_document_free(doc);
    printf("PASS\n");
    
    printf("Test: ISON Dumps... ");
    fflush(stdout);
    
    doc = ison_document_create();
    block = ison_block_create("table", "users");
    ison_block_add_field(block, "id", "int");
    ison_block_add_field(block, "name", "string");
    
    ison_row_t *row = ison_row_create();
    ison_value_t val = ison_int(1);
    ison_row_set(row, "id", &val);
    val = ison_string("Alice");
    ison_row_set(row, "name", &val);
    ison_block_add_row(block, row);
    
    ison_document_add_block(doc, block);
    
    char *output = ison_dumps(doc);
    assert(strstr(output, "table.users") != NULL);
    assert(strstr(output, "id:int") != NULL);
    assert(strstr(output, "name:string") != NULL);
    free(output);
    
    ison_document_free(doc);
    printf("PASS\n");
    
    printf("Test: ISONL Parse... ");
    fflush(stdout);
    
    const char *isonl_input = 
        "table.users|id:int name:string|1 Alice\n"
        "table.users|id:int name:string|2 Bob\n";
    
    doc = ison_parse_isonl(isonl_input, &err);
    assert(doc != NULL);
    
    ison_block_t *users = ison_document_get(doc, "users");
    assert(users != NULL);
    assert(users->row_count == 2);
    
    ison_document_free(doc);
    printf("PASS\n");
    
    printf("Test: ISON to JSON... ");
    fflush(stdout);
    
    const char *ison_input = 
        "table.users\n"
        "id:int name:string\n"
        "1 Alice\n"
        "2 Bob\n";
    
    char *json = ison_to_json(ison_input, &err);
    assert(json != NULL);
    assert(strstr(json, "\"users\"") != NULL);
    assert(strstr(json, "\"id\"") != NULL);
    assert(strstr(json, "\"name\"") != NULL);
    free(json);
    printf("PASS\n");
    
    printf("Test: References... ");
    fflush(stdout);
    
    const char *ref_input = 
        "table.orders\n"
        "id user_id product\n"
        "1 :1 Widget\n"
        "2 :user:42 Gadget\n"
        "3 :OWNS:5 Gizmo\n";
    
    doc = ison_parse(ref_input, &err);
    assert(doc != NULL);
    
    ison_block_t *orders = ison_document_get(doc, "orders");
    assert(orders != NULL);
    
    ison_value_t *ref_val = ison_row_get_ptr(orders->rows[0], "user_id");
    assert(ref_val != NULL);
    assert(ref_val->type == ISON_TYPE_REFERENCE);
    assert(strcmp(ref_val->data.ref_val.id, "1") == 0);
    
    ref_val = ison_row_get_ptr(orders->rows[1], "user_id");
    assert(strcmp(ref_val->data.ref_val.ns, "user") == 0);
    assert(strcmp(ref_val->data.ref_val.id, "42") == 0);
    
    ref_val = ison_row_get_ptr(orders->rows[2], "user_id");
    assert(strcmp(ref_val->data.ref_val.relationship, "OWNS") == 0);
    assert(strcmp(ref_val->data.ref_val.id, "5") == 0);
    
    ison_document_free(doc);
    printf("PASS\n");
    
    printf("\nAll advanced tests passed!\n");
    return 0;
}
