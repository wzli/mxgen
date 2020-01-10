# Minimal X-Macro Generator

Extremely minimal [X-Macros](https://en.wikipedia.org/wiki/X_Macro) to extend C structs with basic cross-platform serialization and JSON string conversion. 

# Use Case
- Originally part of an embedded ecosystem, to facilitate message passing between microcontrollers on top of byte transmission protocols.
- JSON string generation is primarily intended for automated debug prints and logging, with the added benefit of WebAPI compatibility.
- Combine message definition and code generation into concise header files that are portable and easily shared in cross-platform application.

## Features

- Cross-platform binary serialization.
- Introspective JSON string conversion.
- Comparison operator generation.
- Supports structs, nested structs, and array members.
- Header only, just include [mxgen.h](./mxgen.h).
- Bare minimal, 120 lines of code total.
- No external dependencies.
- No dynamic memory usage.
- C99 compliant.


## Usage

See [Example](./test.c):
```C
#include "mxgen.h"

/* For the equivalent struct definition
typedef struct {
    int32_t int_member;
    float float_array[5];
    StructB nested_struct;
}   StructA;
*/
#define STRUCT_StructA(X)      \
    X(int32_t, int_member, )   \
    X(float, float_array, [5]) \
    X(StructB, nested_struct, )
GEN_STRUCT(StructA);

// the following functions are automatically generated 
int StructA_compare(const StructA* a, const StructA* b);
int StructA_serialize(const StructA* a, uint8_t* buf);
int StructA_deserialize(StructA* a, const uint8_t* buf);
int StructA_to_json(const StructA* a, char*);
```

## Demo

Test using any C compiler, for example:

```bash
gcc test.c
./a.out
```
Output:
```json
{"i":"1", "d":["2.5", "0.3", "99.9"], "c":"hello"}
```

## Limitations

- Only structs are supported.
- Nested structs must be generated prior to parent struct.
- Pointers members are not supported.
- Variable Length Arrays are not supported.
- Use fixed-sized char arrays to store strings.
- Primitive fields use stdint.h aliases only (+char). Eg: use **int32_t** instead of **int**
