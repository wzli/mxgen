#pragma once
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define GEN_STRUCT(STRUCT)         \
    GEN_STRUCT_DEFINITION(STRUCT); \
    GEN_TO_JSON(STRUCT);           \
    GEN_SERIALIZE(STRUCT);         \
    GEN_DESERIALIZE(STRUCT);

#define GEN_STRUCT_FIELD(TYPE, NAME, ARRAY) TYPE NAME ARRAY;
#define GEN_STRUCT_DEFINITION(STRUCT) typedef struct { STRUCT_##STRUCT(GEN_STRUCT_FIELD) } STRUCT

#define GEN_SERIALIZE_FIELD(TYPE, NAME, ARRAY)                       \
    for (uint32_t i = 0; i < sizeof(TYPE ARRAY) / sizeof(TYPE); ++i) \
        len += TYPE##_serialize((TYPE*) &struc->NAME + i, buf + len);

#define GEN_DESERIALIZE_FIELD(TYPE, NAME, ARRAY)                     \
    for (uint32_t i = 0; i < sizeof(TYPE ARRAY) / sizeof(TYPE); ++i) \
        len += TYPE##_deserialize((TYPE*) &struc->NAME + i, buf + len);

#define GEN_SERIALIZE(STRUCT)                                                 \
    static inline int STRUCT##_serialize(const STRUCT* struc, uint8_t* buf) { \
        int len = 0;                                                          \
        STRUCT_##STRUCT(GEN_SERIALIZE_FIELD);                                 \
        return len;                                                           \
    }

#define GEN_DESERIALIZE(STRUCT)                                                 \
    static inline int STRUCT##_deserialize(STRUCT* struc, const uint8_t* buf) { \
        int len = 0;                                                            \
        STRUCT_##STRUCT(GEN_DESERIALIZE_FIELD);                                 \
        return len;                                                             \
    }

#define GEN_JSON_STRING(NAME)                \
    buf[len++] = '"';                        \
    strcpy(buf + len, (char*) &struc->NAME); \
    len += strlen((char*) &struc->NAME);     \
    buf[len++] = '"';

#define GEN_JSON_LIST(TYPE, NAME, ARRAY)                               \
    buf[len++] = '[';                                                  \
    for (uint32_t i = 0; i < sizeof(TYPE ARRAY) / sizeof(TYPE); ++i) { \
        len += TYPE##_to_json((TYPE*) &struc->NAME + i, buf + len);    \
        buf[len++] = ',';                                              \
        buf[len++] = ' ';                                              \
    }                                                                  \
    buf[--len - 1] = ']';

#define GEN_JSON_FIELD(TYPE, NAME, ARRAY)                       \
    len += sprintf(buf + len, "\"%s\":", #NAME);                \
    if (!*#ARRAY) {                                             \
        len += TYPE##_to_json((TYPE*) &struc->NAME, buf + len); \
    } else if (!strcmp(#TYPE, "char")) {                        \
        GEN_JSON_STRING(NAME);                                  \
    } else {                                                    \
        GEN_JSON_LIST(TYPE, NAME, ARRAY);                       \
    }                                                           \
    buf[len++] = ',';                                           \
    buf[len++] = ' ';

#define GEN_TO_JSON(STRUCT)                                              \
    static inline int STRUCT##_to_json(const STRUCT* struc, char* buf) { \
        int len = 0;                                                     \
        buf[len++] = '{';                                                \
        STRUCT_##STRUCT(GEN_JSON_FIELD);                                 \
        buf[len - 2] = '}';                                              \
        buf[--len] = '\0';                                               \
        return len;                                                      \
    }

#define IS_LITTLE_ENDIAN ((union {uint32_t x; uint8_t c;}){1}.c)
#define SERIALIZE_MEMCPY(X, Y, N) IS_LITTLE_ENDIAN ? memcpy(X, Y, N) : reverse_memcpy(X, Y, N)

#define GEN_PRIMITIVE(TYPE, FORMAT)                                         \
    static inline int TYPE##_serialize(const TYPE* struc, uint8_t* buf) {   \
        SERIALIZE_MEMCPY(buf, struc, sizeof(TYPE));                         \
        return sizeof(TYPE);                                                \
    }                                                                       \
    static inline int TYPE##_deserialize(TYPE* struc, const uint8_t* buf) { \
        SERIALIZE_MEMCPY(struc, buf, sizeof(TYPE));                         \
        return sizeof(TYPE);                                                \
    }                                                                       \
    static inline int TYPE##_to_json(const TYPE* struc, char* buf) {        \
        return sprintf(buf, "\"" FORMAT "\"", *struc);                      \
    }

static inline void* reverse_memcpy(void* dst, const void* src, size_t n) {
    for (size_t i = 0; i < n; ++i)
        ((uint8_t*) dst)[n - 1 - i] = ((uint8_t*) src)[i];
    return dst;
}

GEN_PRIMITIVE(char, "%c")
GEN_PRIMITIVE(uint8_t, "%u")
GEN_PRIMITIVE(uint16_t, "%u")
GEN_PRIMITIVE(uint32_t, "%u")
GEN_PRIMITIVE(uint64_t, "%lu")
GEN_PRIMITIVE(int8_t, "%d")
GEN_PRIMITIVE(int16_t, "%d")
GEN_PRIMITIVE(int32_t, "%d")
GEN_PRIMITIVE(int64_t, "%ld")
GEN_PRIMITIVE(float, "%f")
GEN_PRIMITIVE(double, "%f")