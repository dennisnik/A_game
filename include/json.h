#ifndef JSON_H
#define JSON_H

#include <stdbool.h>

typedef enum {
    JSON_NULL,
    JSON_BOOL,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} JsonType;

typedef struct JsonValue JsonValue;

typedef struct JsonMember {
    char *key;
    JsonValue *value;
    struct JsonMember *next;
} JsonMember;

typedef struct JsonElement {
    JsonValue *value;
    struct JsonElement *next;
} JsonElement;

struct JsonValue {
    JsonType type;
    union {
        bool boolVal;
        double numVal;
        char *strVal;
        JsonElement *arrHead;
        JsonMember *objHead;
    };
};

// Parser & Memory Management
JsonValue *JsonParse(const char *text);
void JsonFree(JsonValue *val);

// Helper queries
JsonValue *JsonObjectGet(const JsonValue *obj, const char *key);
const char *JsonObjectGetString(const JsonValue *obj, const char *key, const char *defaultVal);
double JsonObjectGetNumber(const JsonValue *obj, const char *key, double defaultVal);
bool JsonObjectGetBool(const JsonValue *obj, const char *key, bool defaultVal);
JsonValue *JsonObjectGetArray(const JsonValue *obj, const char *key);

#endif // JSON_H
