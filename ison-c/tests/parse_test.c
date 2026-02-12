#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ison.h"

int main(void) {
    printf("Test 1: Basic parse...\n");
    fflush(stdout);
    
    const char *input = "table.users\nid name\n1 Alice\n";
    printf("Input:\n%s\n", input);
    fflush(stdout);
    
    ison_error_t err;
    printf("Calling ison_parse...\n");
    fflush(stdout);
    
    ison_document_t *doc = ison_parse(input, &err);
    printf("Parse returned: doc=%p, err=%d\n", (void*)doc, err);
    fflush(stdout);
    
    if (!doc) {
        printf("FAIL: doc is NULL\n");
        return 1;
    }
    
    printf("block_count=%zu\n", doc->block_count);
    fflush(stdout);
    
    printf("Calling ison_document_free...\n");
    fflush(stdout);
    ison_document_free(doc);
    printf("Done!\n");
    
    return 0;
}
