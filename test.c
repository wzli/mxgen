#include "mxgen.h"
#include "assert.h"

/* Struct With Arrays
typedef struct {
    uint8_t a;
    double b[5];
    char c[10]
} StructA;
*/
#define TYPEDEF_StructA(FIELD, IGNORE) \
    FIELD(uint8_t, i, )                \
    FIELD(char, c, )                   \
    IGNORE(void*, ignore, [5])         \
    FIELD(double, d, [3])              \
    FIELD(char, s, [10])               \
    FIELD(bool, b, )
MXGEN(struct, StructA)

/* Nested Struct
typedef struct {
    int32_t x;
    StructA y;
    StructA z[2];
} StructB;
*/
#define TYPEDEF_StructB(FIELD, _) \
    FIELD(int32_t, x, )           \
    FIELD(StructA, y, )           \
    FIELD(StructA, z, [2])
MXGEN(struct, StructB)

/* Double Nested Struct
typedef struct {
    StructA a;
    StructB b[2];
} StructC;
*/
#define TYPEDEF_StructC(FIELD, _) \
    FIELD(StructA, a, )           \
    FIELD(StructB, b, [2])
MXGEN(struct, StructC)

int main() {
    // initialize variables
    StructC struct_c = {0};
    struct_c.a = (StructA){1, 'K', {0}, {2.5, -0.3, 99.9}, "hello", true};
    struct_c.b[0] = (StructB){-555555, struct_c.a, {{0}, struct_c.a}};
    char buf[600] = {0};

    // test serialize and deserialize
    StructB_serialize(&struct_c.b[0], (uint8_t*) buf);
    StructB_deserialize(&struct_c.b[1], (uint8_t*) buf);

    // test comparison
    assert(!StructB_compare(&struct_c.b[0], &struct_c.b[1]));

    // test json string
    puts("StructA:");
    StructA_to_json(&struct_c.a, buf);
    puts(buf);
    puts("");

    puts("StructB:");
    StructB_to_json(&struct_c.b[0], buf);
    puts(buf);
    puts("");

    puts("StructC:");
    StructC_to_json(&struct_c, buf);
    puts(buf);
    puts("");

    // test csv string
    puts("CSV Header A:");
    StructA_to_csv_header(0, buf);
    puts(buf);
    puts("CSV Entry A:");
    StructA_to_csv_entry(&struct_c.a, buf);
    puts(buf);
    puts("");

    puts("CSV Header B:");
    StructB_to_csv_header(0, buf);
    puts(buf);
    puts("CSV Entry B:");
    StructB_to_csv_entry(&struct_c.b[0], buf);
    puts(buf);
    puts("");

    puts("CSV Header C:");
    StructC_to_csv_header(0, buf);
    puts(buf);
    puts("CSV Entry C:");
    StructC_to_csv_entry(&struct_c, buf);
    puts(buf);

    return 0;
}
