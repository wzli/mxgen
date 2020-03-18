#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define GEN_STRUCT(STRUCT)            \
    GEN_STRUCT_DEFINITION(STRUCT);    \
    GEN_STRUCT_COMPARE(STRUCT);       \
    GEN_STRUCT_SERIALIZE(STRUCT);     \
    GEN_STRUCT_DESERIALIZE(STRUCT);   \
    GEN_STRUCT_TO_CSV_HEADER(STRUCT); \
    GEN_STRUCT_TO_CSV_ENTRY(STRUCT);  \
    GEN_STRUCT_TO_JSON(STRUCT);

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

#define GEN_STRUCT_FIELD_STRING(NAME)        \
    buf[len++] = '"';                        \
    strcpy(buf + len, (char*) &struc->NAME); \
    len += strlen((char*) &struc->NAME);     \
    buf[len++] = '"';

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
    if (is_char(#TYPE)) {                                       \
        GEN_STRUCT_FIELD_STRING(NAME);                          \
    } else if (sizeof(TYPE ARRAY) == sizeof(TYPE)) {            \
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
        memcpy(buf + len, prefix, prefix_len);                                                \
        prefix = buf + len;                                                                   \
        len += prefix_len;                                                                    \
        int name_len = sizeof(TYPE ARRAY) == sizeof(TYPE) || is_char(#TYPE)                   \
                               ? sprintf(buf + len, #NAME "/")                                \
                               : sprintf(buf + len, #NAME "/%u/", i);                         \
        len += TYPE##_to_csv_header(buf + len, prefix, prefix_len + name_len);                \
    }

#define GEN_STRUCT_TO_CSV_HEADER(STRUCT)                                                      \
    static inline int STRUCT##_to_csv_header(char* buf, const char* prefix, int prefix_len) { \
        int len = 0;                                                                          \
        STRUCT_##STRUCT(GEN_STRUCT_CSV_HEADER_FIELD);                                         \
        return len;                                                                           \
    }

#define GEN_STRUCT_CSV_ENTRY_FIELD(TYPE, NAME, ARRAY)                        \
    if (is_char(#TYPE)) {                                                    \
        GEN_STRUCT_FIELD_STRING(NAME);                                       \
        buf[len++] = ',';                                                    \
        buf[len] = '\0';                                                     \
    } else {                                                                 \
        for (size_t i = 0; i < sizeof(TYPE ARRAY) / sizeof(TYPE); ++i) {     \
            len += TYPE##_to_csv_entry((TYPE*) &struc->NAME + i, buf + len); \
        }                                                                    \
    }

#define GEN_STRUCT_TO_CSV_ENTRY(STRUCT)                                       \
    static inline int STRUCT##_to_csv_entry(const STRUCT* struc, char* buf) { \
        int len = 0;                                                          \
        STRUCT_##STRUCT(GEN_STRUCT_CSV_ENTRY_FIELD);                          \
        return len;                                                           \
    }

#define GEN_STRUCT_PRIMITIVE(TYPE, FORMAT)                                                  \
    static inline int TYPE##_serialize(const TYPE* struc, uint8_t* buf) {                   \
        SERIALIZE_MEMCPY(buf, struc, sizeof(TYPE));                                         \
        return sizeof(TYPE);                                                                \
    }                                                                                       \
    static inline int TYPE##_deserialize(TYPE* struc, const uint8_t* buf) {                 \
        SERIALIZE_MEMCPY(struc, buf, sizeof(TYPE));                                         \
        return sizeof(TYPE);                                                                \
    }                                                                                       \
    static inline int TYPE##_to_json(const TYPE* struc, char* buf) {                        \
        return sprintf(buf, FORMAT, *struc);                                                \
    }                                                                                       \
    static inline int TYPE##_to_csv_header(char* buf, const char* prefix, int prefix_len) { \
        int len = prefix_len + (prefix - buf);                                              \
        buf[len - 1] = ',';                                                                 \
        buf[len] = '\0';                                                                    \
        return len;                                                                         \
    }                                                                                       \
    static inline int TYPE##_to_csv_entry(const TYPE* struc, char* buf) {                   \
        int len = sprintf(buf, FORMAT, *struc);                                             \
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

GEN_STRUCT_PRIMITIVE(bool, (*struc) ? "true%c" : "false")
GEN_STRUCT_PRIMITIVE(char, (*struc) ? "\"%c\"" : "null")
GEN_STRUCT_PRIMITIVE(uint8_t, "%u")
GEN_STRUCT_PRIMITIVE(uint16_t, "%u")
GEN_STRUCT_PRIMITIVE(uint32_t, "%u")
GEN_STRUCT_PRIMITIVE(uint64_t, "%lu")
GEN_STRUCT_PRIMITIVE(int8_t, "%d")
GEN_STRUCT_PRIMITIVE(int16_t, "%d")
GEN_STRUCT_PRIMITIVE(int32_t, "%d")
GEN_STRUCT_PRIMITIVE(int64_t, "%ld")
GEN_STRUCT_PRIMITIVE(float, "%g")
GEN_STRUCT_PRIMITIVE(double, "%g")
