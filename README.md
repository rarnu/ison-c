# ISON-C

A minimal, token-efficient data format optimized for LLMs and Agentic AI workflows.

The origin project and why ([HERE](https://github.com/ISON-format/ison))

## Installation

### C

#### build & install

```shell
$ make clean
$ make
$ sudo make install
```

## Usage Examples

### C

```c++
#include <ison.h>

const char * input = "table.users\n"
                     "id:int name:string active:bool\n"
                     "1 Alice true\n"
                     "2 Bob false";
ison_error_t err;
ison_document_t* doc = ison_parse(input, &err);

if (!doc) {
    printf("ison_parse error: %s\n", ison_error_string(err));
    return 1;
}

ison_block_t* block = ison_document_get(doc, "users");
if (!block) {
    printf("ison_document_get error: block not found\n");
    return 1;
}

for (size_t i = 0; i < block->row_count; i++) {
    const ison_row_t* row = block->rows[i];
    ison_value_t v;
    if (ison_row_get(row, "name", &v)) {
        printf("%s\n", v.data.string_val);
    }
}

ison_document_free(doc);
```
