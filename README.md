# Minimal X-Macro Generator

Extremely minimal [X-Macros](https://en.wikipedia.org/wiki/X_Macro) to extend C structs with basic cross-platform serialization and string conversion.

## Use Case
- Originally part of an embedded ecosystem, to facilitate message passing between microcontrollers on top of byte transmission protocols.
- JSON and CSV string generation is primarily intended for automated debug prints and data logging.
- Combine message definition and code generation into concise header files that are portable and easily shared in cross-platform application.

## Features

- Cross-platform binary serialization.
- Introspective JSON and CSV string conversion.
- Supports nested structs, unions, and arrays.
- Ignored fields enable integration with internal data structures.
- Header only, just include [mxgen.h](./mxgen.h).
- Bare minimal, under 200 lines of code total.
- No external dependencies.
- No dynamic memory allocation.
- C99 compliant.


## Usage

See [Example](./test.c):
```C
#include "mxgen.h"

/* For the equivalent struct definition
typedef struct {
    int32_t int_member;
    bool bool_member;
    void* ignored_member; <-- this field will not be serialized
    float float_array[5];
    StructB nested_struct;
} StructA;
*/
#define TYPEDEF_StructA(X, _)  \
    X(int32_t, int_member, )   \
    X(bool, bool_member, )     \
    _(void*, ignored_member, ) \
    X(float, float_array, [5]) \
    X(StructB, nested_struct, )
GEN_STRUCT(StructA)

// the following functions are automatically generated 
int StructA_compare(const StructA* a, const StructA* b);
int StructA_serialize(const StructA* a, uint8_t* buf);
int StructA_deserialize(StructA* a, const uint8_t* buf);
int StructA_to_json(const StructA* a, char* buf);
int StructA_to_csv_header(int prefix_offset, char* buf);
int StructA_to_csv_entry(const StructA* a, char* buf);
```

## Demo

Test using any C compiler, for example:

```bash
gcc test.c
./a.out
```
Output:
```json
{"i":"1", "d":["2.5", "0.3", "99.9"], "s":"hello", "b":true}
...
```

## Limitations (ignored fields are exempt)

- Nested structs/unions must be generated prior to parent struct/union.
- Complex floats are not supported.
- Pointers members are not supported.
- Strings as fixed-sized char arrays only.
- Integer fields use *stdint.h* aliases only. Eg: use **int32_t** instead of **int**
