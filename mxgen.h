#pragma once
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define GEN_STRUCT(STRUCT)           \
    GEN_STRUCT_DEFINITION(STRUCT);   \
    GEN_STRUCT_COMPARE(STRUCT)       \
    GEN_STRUCT_SERIALIZE(STRUCT)     \
    GEN_STRUCT_DESERIALIZE(STRUCT)   \
    GEN_STRUCT_TO_CSV_HEADER(STRUCT) \
    GEN_STRUCT_TO_CSV_ENTRY(STRUCT)  \
    GEN_STRUCT_TO_JSON(STRUCT)

#define GEN_STRUCT_FIELD(TYPE, NAME, ARRAY) TYPE NAME ARRAY;
#define GEN_STRUCT_DEFINITION(STRUCT)     \
    typedef struct {                      \
        STRUCT_##STRUCT(GEN_STRUCT_FIELD) \
    } STRUCT

#define GEN_STRUCT_COMPARE_FIELD(TYPE, NAME, ARRAY)                           \
    for (size_t i = 0; !result && i < sizeof(TYPE ARRAY) / sizeof(TYPE); ++i) \
        result = TYPE##_compare((TYPE*) &a->NAME + i, (TYPE*) &b->NAME + i);

#define GEN_STRUCT_SERIALIZE_FIELD(TYPE, NAME, ARRAY)              \
    for (size_t i = 0; i < sizeof(TYPE ARRAY) / sizeof(TYPE); ++i) \
        len += TYPE##_serialize((TYPE*) &struc->NAME + i, buf + len);

#define GEN_STRUCT_DESERIALIZE_FIELD(TYPE, NAME, ARRAY)            \
    for (size_t i = 0; i < sizeof(TYPE ARRAY) / sizeof(TYPE); ++i) \
        len += TYPE##_deserialize((TYPE*) &struc->NAME + i, buf + len);

#define GEN_STRUCT_COMPARE(STRUCT)                                         \
    static inline int STRUCT##_compare(const STRUCT* a, const STRUCT* b) { \
        int result = 0;                                                    \
        STRUCT_##STRUCT(GEN_STRUCT_COMPARE_FIELD);                         \
        return result;                                                     \
    }

#define GEN_STRUCT_SERIALIZE(STRUCT)                                          \
    static inline int STRUCT##_serialize(const STRUCT* struc, uint8_t* buf) { \
        int len = 0;                                                          \
        STRUCT_##STRUCT(GEN_STRUCT_SERIALIZE_FIELD);                          \
        return len;                                                           \
    }

#define GEN_STRUCT_DESERIALIZE(STRUCT)                                          \
    static inline int STRUCT##_deserialize(STRUCT* struc, const uint8_t* buf) { \
        int len = 0;                                                            \
        STRUCT_##STRUCT(GEN_STRUCT_DESERIALIZE_FIELD);                          \
        return len;                                                             \
    }

#define GEN_STRUCT_JSON_FIELD_LIST(TYPE, NAME, ARRAY)                \
    buf[len++] = '[';                                                \
    for (size_t i = 0; i < sizeof(TYPE ARRAY) / sizeof(TYPE); ++i) { \
        len += TYPE##_to_json((TYPE*) &struc->NAME + i, buf + len);  \
        buf[len++] = ',';                                            \
        buf[len++] = ' ';                                            \
    }                                                                \
    buf[--len - 1] = ']';

#define GEN_STRUCT_JSON_FIELD(TYPE, NAME, ARRAY)                \
    len += sprintf(buf + len, "\"" #NAME "\":");                \
    if (sizeof(TYPE ARRAY) == sizeof(TYPE) || is_char(#TYPE)) { \
        len += TYPE##_to_json((TYPE*) &struc->NAME, buf + len); \
    } else {                                                    \
        GEN_STRUCT_JSON_FIELD_LIST(TYPE, NAME, ARRAY);          \
    }                                                           \
    buf[len++] = ',';                                           \
    buf[len++] = ' ';

#define GEN_STRUCT_TO_JSON(STRUCT)                                       \
    static inline int STRUCT##_to_json(const STRUCT* struc, char* buf) { \
        int len = 0;                                                     \
        buf[len++] = '{';                                                \
        STRUCT_##STRUCT(GEN_STRUCT_JSON_FIELD);                          \
        buf[len - 2] = '}';                                              \
        buf[--len] = '\0';                                               \
        return len;                                                      \
    }

#define GEN_STRUCT_CSV_HEADER_FIELD(TYPE, NAME, ARRAY)                                        \
    for (uint32_t i = 0; i < (is_char(#TYPE) ? 1 : sizeof(TYPE ARRAY) / sizeof(TYPE)); ++i) { \
        buf[len - prefix_len] = '/';                                                          \
        int name_len = sizeof(TYPE ARRAY) == sizeof(TYPE) || is_char(#TYPE)                   \
                               ? sprintf(buf + len, "/" #NAME)                                \
                               : sprintf(buf + len, "/" #NAME "/%u", i);                      \
        len += TYPE##_to_csv_header(buf + len + name_len, prefix, prefix_len + name_len);     \
        prefix = buf + len - prefix_len;                                                      \
    }

#define GEN_STRUCT_TO_CSV_HEADER(STRUCT)                                                      \
    static inline int STRUCT##_to_csv_header(char* buf, const char* prefix, int prefix_len) { \
        int len = 0;                                                                          \
        STRUCT_##STRUCT(GEN_STRUCT_CSV_HEADER_FIELD);                                         \
        return len;                                                                           \
    }

#define GEN_STRUCT_CSV_ENTRY_FIELD(TYPE, NAME, ARRAY)                                       \
    for (size_t i = 0; i < (is_char(#TYPE) ? 1 : sizeof(TYPE ARRAY) / sizeof(TYPE)); ++i) { \
        len += TYPE##_to_csv_entry((TYPE*) &struc->NAME + i, buf + len);                    \
    }

#define GEN_STRUCT_TO_CSV_ENTRY(STRUCT)                                       \
    static inline int STRUCT##_to_csv_entry(const STRUCT* struc, char* buf) { \
        int len = 0;                                                          \
        STRUCT_##STRUCT(GEN_STRUCT_CSV_ENTRY_FIELD);                          \
        return len;                                                           \
    }

#define GEN_STRUCT_PRIMITIVE(TYPE, FORMAT, ...)                                             \
    static inline int TYPE##_serialize(const TYPE* struc, uint8_t* buf) {                   \
        SERIALIZE_MEMCPY(buf, struc, sizeof(TYPE));                                         \
        return sizeof(TYPE);                                                                \
    }                                                                                       \
    static inline int TYPE##_deserialize(TYPE* struc, const uint8_t* buf) {                 \
        SERIALIZE_MEMCPY(struc, buf, sizeof(TYPE));                                         \
        return sizeof(TYPE);                                                                \
    }                                                                                       \
    static inline int TYPE##_to_json(const TYPE* struc, char* buf) {                        \
        return sprintf(buf, FORMAT, __VA_ARGS__);                                           \
    }                                                                                       \
    static inline int TYPE##_to_csv_header(char* buf, const char* prefix, int prefix_len) { \
        memcpy(buf + 1, prefix, prefix_len);                                                \
        buf[0] = ',';                                                                       \
        buf[1] = '\0';                                                                      \
        return prefix_len + 1;                                                              \
    }                                                                                       \
    static inline int TYPE##_to_csv_entry(const TYPE* struc, char* buf) {                   \
        int len = sprintf(buf, FORMAT, __VA_ARGS__);                                        \
        buf[len++] = ',';                                                                   \
        buf[len] = '\0';                                                                    \
        return len;                                                                         \
    }                                                                                       \
    static inline int TYPE##_compare(const TYPE* a, const TYPE* b) { return (int) (*b - *a); }

#define IS_LITTLE_ENDIAN \
    ((union {            \
        uint32_t x;      \
        uint8_t c;       \
    }){1}                \
                    .c)
#define SERIALIZE_MEMCPY(X, Y, N) IS_LITTLE_ENDIAN ? memcpy(X, Y, N) : reverse_memcpy(X, Y, N)

static inline void* reverse_memcpy(void* dst, const void* src, size_t n) {
    for (size_t i = 0; i < n; ++i)
        ((uint8_t*) dst)[n - 1 - i] = ((uint8_t*) src)[i];
    return dst;
}

static inline bool is_char(const char* type) {
    return *(int*) type == (IS_LITTLE_ENDIAN ? 0x72616863 : 0x63686172);  // 'char'
}

GEN_STRUCT_PRIMITIVE(bool, "%s", (*struc) ? "true" : "false")
GEN_STRUCT_PRIMITIVE(char, "\"%s\"", struc)
GEN_STRUCT_PRIMITIVE(uint8_t, "%" PRIu8, *struc)
GEN_STRUCT_PRIMITIVE(uint16_t, "%" PRIu16, *struc)
GEN_STRUCT_PRIMITIVE(uint32_t, "%" PRIu32, *struc)
GEN_STRUCT_PRIMITIVE(uint64_t, "%" PRIu64, *struc)
GEN_STRUCT_PRIMITIVE(int8_t, "%" PRId8, *struc)
GEN_STRUCT_PRIMITIVE(int16_t, "%" PRId16, *struc)
GEN_STRUCT_PRIMITIVE(int32_t, "%" PRId32, *struc)
GEN_STRUCT_PRIMITIVE(int64_t, "%" PRId64, *struc)
GEN_STRUCT_PRIMITIVE(float, "%g", (double) *struc)
GEN_STRUCT_PRIMITIVE(double, "%g", *struc)
