#include "json.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

static const char *SkipWhitespace(const char *p) {
    while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) {
        p++;
    }
    return p;
}

static JsonValue *ParseValue(const char **p);

static char *ParseString(const char **p) {
    if (**p != '"') return NULL;
    (*p)++; // Skip opening quote

    const char *start = *p;
    size_t len = 0;
    while (**p && **p != '"') {
        if (**p == '\\' && *(*p + 1)) {
            (*p) += 2;
            len++;
        } else {
            (*p)++;
            len++;
        }
    }

    if (**p != '"') return NULL; // Missing closing quote

    char *out = (char *)malloc(len + 1);
    if (!out) return NULL;

    const char *src = start;
    char *dst = out;
    while (*src && *src != '"') {
        if (*src == '\\') {
            src++;
            switch (*src) {
                case 'n': *dst++ = '\n'; break;
                case 't': *dst++ = '\t'; break;
                case 'r': *dst++ = '\r'; break;
                case '"': *dst++ = '"'; break;
                case '\\': *dst++ = '\\'; break;
                default: *dst++ = *src; break;
            }
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
    (*p)++; // Skip closing quote
    return out;
}

static JsonValue *ParseArray(const char **p) {
    if (**p != '[') return NULL;
    (*p)++; // Skip '['

    JsonValue *arr = (JsonValue *)calloc(1, sizeof(JsonValue));
    arr->type = JSON_ARRAY;
    JsonElement **tail = &arr->arrHead;

    *p = SkipWhitespace(*p);
    if (**p == ']') {
        (*p)++;
        return arr;
    }

    while (**p) {
        *p = SkipWhitespace(*p);
        JsonValue *elemVal = ParseValue(p);
        if (!elemVal) {
            JsonFree(arr);
            return NULL;
        }

        JsonElement *elem = (JsonElement *)malloc(sizeof(JsonElement));
        elem->value = elemVal;
        elem->next = NULL;
        *tail = elem;
        tail = &elem->next;

        *p = SkipWhitespace(*p);
        if (**p == ',') {
            (*p)++;
        } else if (**p == ']') {
            (*p)++;
            break;
        } else {
            JsonFree(arr);
            return NULL;
        }
    }
    return arr;
}

static JsonValue *ParseObject(const char **p) {
    if (**p != '{') return NULL;
    (*p)++; // Skip '{'

    JsonValue *obj = (JsonValue *)calloc(1, sizeof(JsonValue));
    obj->type = JSON_OBJECT;
    JsonMember **tail = &obj->objHead;

    *p = SkipWhitespace(*p);
    if (**p == '}') {
        (*p)++;
        return obj;
    }

    while (**p) {
        *p = SkipWhitespace(*p);
        if (**p != '"') {
            JsonFree(obj);
            return NULL;
        }

        char *key = ParseString(p);
        if (!key) {
            JsonFree(obj);
            return NULL;
        }

        *p = SkipWhitespace(*p);
        if (**p != ':') {
            free(key);
            JsonFree(obj);
            return NULL;
        }
        (*p)++; // Skip ':'

        *p = SkipWhitespace(*p);
        JsonValue *val = ParseValue(p);
        if (!val) {
            free(key);
            JsonFree(obj);
            return NULL;
        }

        JsonMember *mem = (JsonMember *)malloc(sizeof(JsonMember));
        mem->key = key;
        mem->value = val;
        mem->next = NULL;
        *tail = mem;
        tail = &mem->next;

        *p = SkipWhitespace(*p);
        if (**p == ',') {
            (*p)++;
        } else if (**p == '}') {
            (*p)++;
            break;
        } else {
            JsonFree(obj);
            return NULL;
        }
    }
    return obj;
}

static JsonValue *ParseValue(const char **p) {
    *p = SkipWhitespace(*p);
    if (!**p) return NULL;

    if (**p == '"') {
        char *str = ParseString(p);
        if (!str) return NULL;
        JsonValue *v = (JsonValue *)calloc(1, sizeof(JsonValue));
        v->type = JSON_STRING;
        v->strVal = str;
        return v;
    }

    if (**p == '[') return ParseArray(p);
    if (**p == '{') return ParseObject(p);

    if (isdigit((unsigned char)**p) || **p == '-') {
        char *end;
        double num = strtod(*p, &end);
        if (end == *p) return NULL;
        *p = end;
        JsonValue *v = (JsonValue *)calloc(1, sizeof(JsonValue));
        v->type = JSON_NUMBER;
        v->numVal = num;
        return v;
    }

    if (strncmp(*p, "true", 4) == 0) {
        *p += 4;
        JsonValue *v = (JsonValue *)calloc(1, sizeof(JsonValue));
        v->type = JSON_BOOL;
        v->boolVal = true;
        return v;
    }

    if (strncmp(*p, "false", 5) == 0) {
        *p += 5;
        JsonValue *v = (JsonValue *)calloc(1, sizeof(JsonValue));
        v->type = JSON_BOOL;
        v->boolVal = false;
        return v;
    }

    if (strncmp(*p, "null", 4) == 0) {
        *p += 4;
        JsonValue *v = (JsonValue *)calloc(1, sizeof(JsonValue));
        v->type = JSON_NULL;
        return v;
    }

    return NULL;
}

JsonValue *JsonParse(const char *text) {
    if (!text) return NULL;
    const char *p = text;
    JsonValue *root = ParseValue(&p);
    return root;
}

void JsonFree(JsonValue *val) {
    if (!val) return;

    switch (val->type) {
        case JSON_STRING:
            free(val->strVal);
            break;
        case JSON_ARRAY: {
            JsonElement *curr = val->arrHead;
            while (curr) {
                JsonElement *next = curr->next;
                JsonFree(curr->value);
                free(curr);
                curr = next;
            }
            break;
        }
        case JSON_OBJECT: {
            JsonMember *curr = val->objHead;
            while (curr) {
                JsonMember *next = curr->next;
                free(curr->key);
                JsonFree(curr->value);
                free(curr);
                curr = next;
            }
            break;
        }
        default:
            break;
    }
    free(val);
}

JsonValue *JsonObjectGet(const JsonValue *obj, const char *key) {
    if (!obj || obj->type != JSON_OBJECT || !key) return NULL;
    JsonMember *curr = obj->objHead;
    while (curr) {
        if (strcmp(curr->key, key) == 0) {
            return curr->value;
        }
        curr = curr->next;
    }
    return NULL;
}

const char *JsonObjectGetString(const JsonValue *obj, const char *key, const char *defaultVal) {
    JsonValue *v = JsonObjectGet(obj, key);
    if (v && v->type == JSON_STRING) return v->strVal;
    return defaultVal;
}

double JsonObjectGetNumber(const JsonValue *obj, const char *key, double defaultVal) {
    JsonValue *v = JsonObjectGet(obj, key);
    if (v && v->type == JSON_NUMBER) return v->numVal;
    return defaultVal;
}

bool JsonObjectGetBool(const JsonValue *obj, const char *key, bool defaultVal) {
    JsonValue *v = JsonObjectGet(obj, key);
    if (v && v->type == JSON_BOOL) return v->boolVal;
    return defaultVal;
}

JsonValue *JsonObjectGetArray(const JsonValue *obj, const char *key) {
    JsonValue *v = JsonObjectGet(obj, key);
    if (v && v->type == JSON_ARRAY) return v;
    return NULL;
}
