#include "mxgen.h"
#include "assert.h"

/* Struct With Arrays
typedef struct {
    uint8_t a;
    double b[5];
    char c[10]
} StructA;
*/
#define STRUCT_StructA(FIELD) \
    FIELD(uint8_t, i, )       \
    FIELD(char, c, )          \
    FIELD(double, d, [3])     \
    FIELD(char, s, [10])      \
    FIELD(bool, b, )
GEN_STRUCT(StructA);

/* Nested Struct
typedef struct {
    int32_t x;
    StructA y;
    StructA z[2];
} StructB;
*/
#define STRUCT_StructB(FIELD) \
    FIELD(int32_t, x, )       \
    FIELD(StructA, y, )       \
    FIELD(StructA, z, [2])
GEN_STRUCT(StructB);

/* Double Nested Struct
typedef struct {
    StructA a;
    StructB b[2];
} StructC;
*/
#define STRUCT_StructC(FIELD) \
    FIELD(StructA, a, )       \
    FIELD(StructB, b, [2])
GEN_STRUCT(StructC);

int main() {
    // initialize variables
    StructC struct_c = {};
    struct_c.a = (StructA){1, 'K', {2.5, -0.3, 99.9}, "hello", true};
    struct_c.b[0] = (StructB){-555555, struct_c.a, {{}, struct_c.a}};
    char buf[600] = {};

    // test serialize and deserialize
    StructB_serialize(&struct_c.b[0], (uint8_t*)buf);
    StructB_deserialize(&struct_c.b[1], (uint8_t*)buf);

    // test comparison
    assert(!StructB_compare(&struct_c.b[0], &struct_c.b[1]));

    // test json string
    StructA_to_json(&struct_c.a, buf);
    printf("StructA: %s\n\n", buf);
    StructB_to_json(&struct_c.b[0], buf);
    printf("StructB: %s\n\n", buf);
    StructC_to_json(&struct_c, buf);
    printf("StructC: %s\n", buf);
    return 0;
}
