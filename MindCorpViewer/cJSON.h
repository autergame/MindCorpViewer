/*
  Copyright (c) 2009-2017 Dave Gamble and cJSON contributors
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#ifdef _WIN64
    typedef unsigned __int64 sizem_t;
    #define SIZEM_MAX _I64_MAX
    #define SIZEM_MIN _I64_MIN
#else
    typedef unsigned __int32 sizem_t;
    #define SIZEM_MAX INT_MAX
    #define SIZEM_MIN INT_MIN
#endif

#if !defined(_CRT_SECURE_NO_DEPRECATE) && defined(_MSC_VER)
#define _CRT_SECURE_NO_DEPRECATE
#endif

#ifdef __GNUC__
#pragma GCC visibility push(default)
#endif
#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning (disable : 4001)
#endif

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <float.h>

#if defined(_MSC_VER)
#pragma warning (pop)
#endif
#ifdef __GNUC__
#pragma GCC visibility pop
#endif

#ifndef cJSON__h
#define cJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

#if !defined(__WINDOWS__) && (defined(WIN32) || defined(WIN64) || defined(_MSC_VER) || defined(_WIN32))
#define __WINDOWS__
#endif

#ifdef __WINDOWS__

#define CJSON_CDECL __cdecl
#define CJSON_STDCALL __stdcall

#if !defined(CJSON_HIDE_SYMBOLS) && !defined(CJSON_IMPORT_SYMBOLS) && !defined(CJSON_EXPORT_SYMBOLS)
#define CJSON_EXPORT_SYMBOLS
#endif

#if defined(CJSON_HIDE_SYMBOLS)
#define CJSON_PUBLIC(type)   type CJSON_STDCALL
#elif defined(CJSON_EXPORT_SYMBOLS)
#define CJSON_PUBLIC(type)   __declspec(dllexport) type CJSON_STDCALL
#elif defined(CJSON_IMPORT_SYMBOLS)
#define CJSON_PUBLIC(type)   __declspec(dllimport) type CJSON_STDCALL
#endif
#else   
#define CJSON_CDECL
#define CJSON_STDCALL

#if (defined(__GNUC__) || defined(__SUNPRO_CC) || defined (__SUNPRO_C)) && defined(CJSON_API_VISIBILITY)
#define CJSON_PUBLIC(type)   __attribute__((visibility("default"))) type
#else
#define CJSON_PUBLIC(type) type
#endif
#endif

#define CJSON_VERSION_MAJOR 1
#define CJSON_VERSION_MINOR 7
#define CJSON_VERSION_PATCH 13

#include <stddef.h>

#define cJSON_Invalid (0)
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
#define cJSON_Object (1 << 6)
#define cJSON_Raw    (1 << 7)    

#define cJSON_IsReference 256
#define cJSON_StringIsConst 512

    typedef struct cJSON
    {
        struct cJSON* next;
        struct cJSON* prev;
        struct cJSON* child;

        int type;

        char* valuestring;
        sizem_t valueint;
        double valuedouble;

        char* string;
    } cJSON;

    typedef struct cJSON_Hooks
    {
        void* (CJSON_CDECL* malloc_fn)(size_t sz);
        void (CJSON_CDECL* free_fn)(void* ptr);
    } cJSON_Hooks;

    typedef int cJSON_bool;

#ifndef CJSON_NESTING_LIMIT
#define CJSON_NESTING_LIMIT 1000
#endif

    CJSON_PUBLIC(const char*) cJSON_Version(void);

    CJSON_PUBLIC(void) cJSON_InitHooks(cJSON_Hooks* hooks);

    CJSON_PUBLIC(cJSON*) cJSON_Parse(const char* value);
    CJSON_PUBLIC(cJSON*) cJSON_ParseWithLength(const char* value, size_t buffer_length);
    CJSON_PUBLIC(cJSON*) cJSON_ParseWithOpts(const char* value, const char** return_parse_end, cJSON_bool require_null_terminated);
    CJSON_PUBLIC(cJSON*) cJSON_ParseWithLengthOpts(const char* value, size_t buffer_length, const char** return_parse_end, cJSON_bool require_null_terminated);

    CJSON_PUBLIC(char*) cJSON_Print(const cJSON* item);
    CJSON_PUBLIC(char*) cJSON_PrintUnformatted(const cJSON* item);
    CJSON_PUBLIC(char*) cJSON_PrintBuffered(const cJSON* item, int prebuffer, cJSON_bool fmt);
    CJSON_PUBLIC(cJSON_bool) cJSON_PrintPreallocated(cJSON* item, char* buffer, const int length, const cJSON_bool format);
    CJSON_PUBLIC(void) cJSON_Delete(cJSON* item);

    CJSON_PUBLIC(int) cJSON_GetArraySize(const cJSON* array);
    CJSON_PUBLIC(cJSON*) cJSON_GetArrayItem(const cJSON* array, int index);
    CJSON_PUBLIC(cJSON*) cJSON_GetObjectItem(const cJSON* const object, const char* const string);
    CJSON_PUBLIC(cJSON*) cJSON_GetObjectItemCaseSensitive(const cJSON* const object, const char* const string);
    CJSON_PUBLIC(cJSON_bool) cJSON_HasObjectItem(const cJSON* object, const char* string);
    CJSON_PUBLIC(const char*) cJSON_GetErrorPtr(void);

    CJSON_PUBLIC(char*) cJSON_GetStringValue(const cJSON* const item);
    CJSON_PUBLIC(double) cJSON_GetNumberValue(const cJSON* const item);

    CJSON_PUBLIC(cJSON_bool) cJSON_IsInvalid(const cJSON* const item);
    CJSON_PUBLIC(cJSON_bool) cJSON_IsFalse(const cJSON* const item);
    CJSON_PUBLIC(cJSON_bool) cJSON_IsTrue(const cJSON* const item);
    CJSON_PUBLIC(cJSON_bool) cJSON_IsBool(const cJSON* const item);
    CJSON_PUBLIC(cJSON_bool) cJSON_IsNull(const cJSON* const item);
    CJSON_PUBLIC(cJSON_bool) cJSON_IsNumber(const cJSON* const item);
    CJSON_PUBLIC(cJSON_bool) cJSON_IsString(const cJSON* const item);
    CJSON_PUBLIC(cJSON_bool) cJSON_IsArray(const cJSON* const item);
    CJSON_PUBLIC(cJSON_bool) cJSON_IsObject(const cJSON* const item);
    CJSON_PUBLIC(cJSON_bool) cJSON_IsRaw(const cJSON* const item);

    CJSON_PUBLIC(cJSON*) cJSON_CreateNull(void);
    CJSON_PUBLIC(cJSON*) cJSON_CreateTrue(void);
    CJSON_PUBLIC(cJSON*) cJSON_CreateFalse(void);
    CJSON_PUBLIC(cJSON*) cJSON_CreateBool(cJSON_bool boolean);
    CJSON_PUBLIC(cJSON*) cJSON_CreateNumber(double num);
    CJSON_PUBLIC(cJSON*) cJSON_CreateString(const char* string);
    CJSON_PUBLIC(cJSON*) cJSON_CreateRaw(const char* raw);
    CJSON_PUBLIC(cJSON*) cJSON_CreateArray(void);
    CJSON_PUBLIC(cJSON*) cJSON_CreateObject(void);

    CJSON_PUBLIC(cJSON*) cJSON_CreateStringReference(const char* string);
    CJSON_PUBLIC(cJSON*) cJSON_CreateObjectReference(const cJSON* child);
    CJSON_PUBLIC(cJSON*) cJSON_CreateArrayReference(const cJSON* child);

    CJSON_PUBLIC(cJSON*) cJSON_CreateIntArray(int* numbers, int count);
    CJSON_PUBLIC(cJSON*) cJSON_CreateFloatArray(float* numbers, int count);
    CJSON_PUBLIC(cJSON*) cJSON_CreateDoubleArray(double* numbers, int count);
    CJSON_PUBLIC(cJSON*) cJSON_CreateStringArray(char* const* strings, int count);

    CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToArray(cJSON* array, cJSON* item);
    CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToObject(cJSON* object, const char* string, cJSON* item);
    CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToObjectCS(cJSON* object, const char* string, cJSON* item);
    CJSON_PUBLIC(cJSON_bool) cJSON_AddItemReferenceToArray(cJSON* array, cJSON* item);
    CJSON_PUBLIC(cJSON_bool) cJSON_AddItemReferenceToObject(cJSON* object, const char* string, cJSON* item);

    CJSON_PUBLIC(cJSON*) cJSON_DetachItemViaPointer(cJSON* parent, cJSON* const item);
    CJSON_PUBLIC(cJSON*) cJSON_DetachItemFromArray(cJSON* array, int which);
    CJSON_PUBLIC(void) cJSON_DeleteItemFromArray(cJSON* array, int which);
    CJSON_PUBLIC(cJSON*) cJSON_DetachItemFromObject(cJSON* object, const char* string);
    CJSON_PUBLIC(cJSON*) cJSON_DetachItemFromObjectCaseSensitive(cJSON* object, const char* string);
    CJSON_PUBLIC(void) cJSON_DeleteItemFromObject(cJSON* object, const char* string);
    CJSON_PUBLIC(void) cJSON_DeleteItemFromObjectCaseSensitive(cJSON* object, const char* string);

    CJSON_PUBLIC(cJSON_bool) cJSON_InsertItemInArray(cJSON* array, int which, cJSON* newitem);        
    CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemViaPointer(cJSON* const parent, cJSON* const item, cJSON* replacement);
    CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInArray(cJSON* array, int which, cJSON* newitem);
    CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInObject(cJSON* object, const char* string, cJSON* newitem);
    CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInObjectCaseSensitive(cJSON* object, const char* string, cJSON* newitem);

    CJSON_PUBLIC(cJSON*) cJSON_Duplicate(const cJSON* item, cJSON_bool recurse);
    CJSON_PUBLIC(cJSON_bool) cJSON_Compare(const cJSON* const a, const cJSON* const b, const cJSON_bool case_sensitive);

    CJSON_PUBLIC(void) cJSON_Minify(char* json);

    CJSON_PUBLIC(cJSON*) cJSON_AddNullToObject(cJSON* const object, const char* const name);
    CJSON_PUBLIC(cJSON*) cJSON_AddTrueToObject(cJSON* const object, const char* const name);
    CJSON_PUBLIC(cJSON*) cJSON_AddFalseToObject(cJSON* const object, const char* const name);
    CJSON_PUBLIC(cJSON*) cJSON_AddBoolToObject(cJSON* const object, const char* const name, const cJSON_bool boolean);
    CJSON_PUBLIC(cJSON*) cJSON_AddNumberToObject(cJSON* const object, const char* const name, const double number);
    CJSON_PUBLIC(cJSON*) cJSON_AddStringToObject(cJSON* const object, const char* const name, const char* const string);
    CJSON_PUBLIC(cJSON*) cJSON_AddRawToObject(cJSON* const object, const char* const name, const char* const raw);
    CJSON_PUBLIC(cJSON*) cJSON_AddObjectToObject(cJSON* const object, const char* const name);
    CJSON_PUBLIC(cJSON*) cJSON_AddArrayToObject(cJSON* const object, const char* const name);

#define cJSON_SetIntValue(object, number) ((object) ? (object)->valueint = (object)->valuedouble = (number) : (number))
    CJSON_PUBLIC(double) cJSON_SetNumberHelper(cJSON* object, double number);
#define cJSON_SetNumberValue(object, number) ((object != NULL) ? cJSON_SetNumberHelper(object, (double)number) : (number))
    CJSON_PUBLIC(char*) cJSON_SetValuestring(cJSON* object, const char* valuestring);

#define cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

    CJSON_PUBLIC(void*) cJSON_malloc(size_t size);
    CJSON_PUBLIC(void) cJSON_free(void* object);

#ifdef __cplusplus
}
#endif

#endif

#ifdef true
#undef true
#endif
#define true ((cJSON_bool)1)

#ifdef false
#undef false
#endif
#define false ((cJSON_bool)0)

#ifndef isinf
#define isinf(d) (isnan((d - d)) && !isnan(d))
#endif
#ifndef isnan
#define isnan(d) (d != d)
#endif

#ifndef NAN
#define NAN 0.0/0.0
#endif

typedef struct {
    const unsigned char* json;
    size_t position;
} error;
static error global_error = { NULL, 0 };

CJSON_PUBLIC(const char*) cJSON_GetErrorPtr(void)
{
    return (const char*)(global_error.json + global_error.position);
}

CJSON_PUBLIC(char*) cJSON_GetStringValue(const cJSON* const item)
{
    if (!cJSON_IsString(item))
    {
        return NULL;
    }

    return item->valuestring;
}

CJSON_PUBLIC(double) cJSON_GetNumberValue(const cJSON* const item)
{
    if (!cJSON_IsNumber(item))
    {
        return NAN;
    }

    return item->valuedouble;
}

#if (CJSON_VERSION_MAJOR != 1) || (CJSON_VERSION_MINOR != 7) || (CJSON_VERSION_PATCH != 13)
#error cJSON.h and cJSON.c have different versions. Make sure that both have the same.
#endif

CJSON_PUBLIC(const char*) cJSON_Version(void)
{
    static char version[15];
    sprintf(version, "%i.%i.%i", CJSON_VERSION_MAJOR, CJSON_VERSION_MINOR, CJSON_VERSION_PATCH);

    return version;
}

static int case_insensitive_strcmp(const unsigned char* string1, const unsigned char* string2)
{
    if ((string1 == NULL) || (string2 == NULL))
    {
        return 1;
    }

    if (string1 == string2)
    {
        return 0;
    }

    for (; tolower(*string1) == tolower(*string2); (void)string1++, string2++)
    {
        if (*string1 == '\0')
        {
            return 0;
        }
    }

    return tolower(*string1) - tolower(*string2);
}

typedef struct internal_hooks
{
    void* (CJSON_CDECL* allocate)(size_t size);
    void (CJSON_CDECL* deallocate)(void* pointer);
    void* (CJSON_CDECL* reallocate)(void* pointer, size_t size);
} internal_hooks;

#if defined(_MSC_VER)
static void* CJSON_CDECL internal_malloc(size_t size)
{
    return malloc(size);
}
static void CJSON_CDECL internal_free(void* pointer)
{
    free(pointer);
}
static void* CJSON_CDECL internal_realloc(void* pointer, size_t size)
{
    return realloc(pointer, size);
}
#else
#define internal_malloc malloc
#define internal_free free
#define internal_realloc realloc
#endif

#define static_strlen(string_literal) (sizeof(string_literal) - sizeof(""))

static internal_hooks global_hooks = { internal_malloc, internal_free, internal_realloc };

static unsigned char* cJSON_strdup(const unsigned char* string, const internal_hooks* const hooks)
{
    size_t length = 0;
    unsigned char* copy = NULL;

    if (string == NULL)
    {
        return NULL;
    }

    length = strlen((const char*)string) + sizeof("");
    copy = (unsigned char*)hooks->allocate(length);
    if (copy == NULL)
    {
        return NULL;
    }
    memcpy(copy, string, length);

    return copy;
}

CJSON_PUBLIC(void) cJSON_InitHooks(cJSON_Hooks* hooks)
{
    if (hooks == NULL)
    {
        global_hooks.allocate = malloc;
        global_hooks.deallocate = free;
        global_hooks.reallocate = realloc;
        return;
    }

    global_hooks.allocate = malloc;
    if (hooks->malloc_fn != NULL)
    {
        global_hooks.allocate = hooks->malloc_fn;
    }

    global_hooks.deallocate = free;
    if (hooks->free_fn != NULL)
    {
        global_hooks.deallocate = hooks->free_fn;
    }

    global_hooks.reallocate = NULL;
    if ((global_hooks.allocate == malloc) && (global_hooks.deallocate == free))
    {
        global_hooks.reallocate = realloc;
    }
}

static cJSON* cJSON_New_Item(const internal_hooks* const hooks)
{
    cJSON* node = (cJSON*)hooks->allocate(sizeof(cJSON));
    if (node)
    {
        memset(node, '\0', sizeof(cJSON));
    }

    return node;
}

CJSON_PUBLIC(void) cJSON_Delete(cJSON* item)
{
    cJSON* next = NULL;
    while (item != NULL)
    {
        next = item->next;
        if (!(item->type & cJSON_IsReference) && (item->child != NULL))
        {
            cJSON_Delete(item->child);
        }
        if (!(item->type & cJSON_IsReference) && (item->valuestring != NULL))
        {
            global_hooks.deallocate(item->valuestring);
        }
        if (!(item->type & cJSON_StringIsConst) && (item->string != NULL))
        {
            global_hooks.deallocate(item->string);
        }
        global_hooks.deallocate(item);
        item = next;
    }
}

static unsigned char get_decimal_point(void)
{
#ifdef ENABLE_LOCALES
    struct lconv* lconv = localeconv();
    return (unsigned char)lconv->decimal_point[0];
#else
    return '.';
#endif
}

typedef struct
{
    const unsigned char* content;
    size_t length;
    size_t offset;
    size_t depth;              
    internal_hooks hooks;
} parse_buffer;

#define can_read(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
#define can_access_at_index(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define cannot_access_at_index(buffer, index) (!can_access_at_index(buffer, index))
#define buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)

static cJSON_bool parse_number(cJSON* const item, parse_buffer* const input_buffer)
{
    double number = 0;
    unsigned char* after_end = NULL;
    unsigned char number_c_string[64];
    unsigned char decimal_point = get_decimal_point();
    size_t i = 0;

    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return false;
    }

    for (i = 0; (i < (sizeof(number_c_string) - 1)) && can_access_at_index(input_buffer, i); i++)
    {
        switch (buffer_at_offset(input_buffer)[i])
        {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '+':
        case '-':
        case 'e':
        case 'E':
            number_c_string[i] = buffer_at_offset(input_buffer)[i];
            break;

        case '.':
            number_c_string[i] = decimal_point;
            break;

        default:
            goto loop_end;
        }
    }
loop_end:
    number_c_string[i] = '\0';

    number = strtod((const char*)number_c_string, (char**)&after_end);
    if (number_c_string == after_end)
    {
        return false;   
    }

    item->valuedouble = number;

    if (number >= SIZEM_MAX)
    {
        item->valueint = SIZEM_MAX;
    }
    else if (number <= (double)SIZEM_MIN)
    {
        item->valueint = SIZEM_MIN;
    }
    else
    {
        item->valueint = (size_t)number;
    }

    item->type = cJSON_Number;

    input_buffer->offset += (size_t)(after_end - number_c_string);
    return true;
}

CJSON_PUBLIC(double) cJSON_SetNumberHelper(cJSON* object, double number)
{
    if (number >= SIZEM_MAX)
    {
        object->valueint = SIZEM_MAX;
    }
    else if (number <= (double)SIZEM_MIN)
    {
        object->valueint = SIZEM_MIN;
    }
    else
    {
        object->valueint = (size_t)number;
    }

    return object->valuedouble = number;
}

CJSON_PUBLIC(char*) cJSON_SetValuestring(cJSON* object, const char* valuestring)
{
    char* copy = NULL;
    if (!(object->type & cJSON_String) || (object->type & cJSON_IsReference))
    {
        return NULL;
    }
    if (strlen(valuestring) <= strlen(object->valuestring))
    {
        strcpy(object->valuestring, valuestring);
        return object->valuestring;
    }
    copy = (char*)cJSON_strdup((const unsigned char*)valuestring, &global_hooks);
    if (copy == NULL)
    {
        return NULL;
    }
    if (object->valuestring != NULL)
    {
        cJSON_free(object->valuestring);
    }
    object->valuestring = copy;

    return copy;
}

typedef struct
{
    unsigned char* buffer;
    size_t length;
    size_t offset;
    size_t depth;        
    cJSON_bool noalloc;
    cJSON_bool format;        
    internal_hooks hooks;
} printbuffer;

static unsigned char* ensure(printbuffer* const p, size_t needed)
{
    unsigned char* newbuffer = NULL;
    size_t newsize = 0;

    if ((p == NULL) || (p->buffer == NULL))
    {
        return NULL;
    }

    if ((p->length > 0) && (p->offset >= p->length))
    {
        return NULL;
    }

    if (needed > SIZEM_MAX)
    {
        return NULL;
    }

    needed += p->offset + 1;
    if (needed <= p->length)
    {
        return p->buffer + p->offset;
    }

    if (p->noalloc) {
        return NULL;
    }

    if (needed > (SIZEM_MAX / 2))
    {
        if (needed <= SIZEM_MAX)
        {
            newsize = SIZEM_MAX;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        newsize = needed * 2;
    }

    if (p->hooks.reallocate != NULL)
    {
        newbuffer = (unsigned char*)p->hooks.reallocate(p->buffer, newsize);
        if (newbuffer == NULL)
        {
            p->hooks.deallocate(p->buffer);
            p->length = 0;
            p->buffer = NULL;

            return NULL;
        }
    }
    else
    {
        newbuffer = (unsigned char*)p->hooks.allocate(newsize);
        if (!newbuffer)
        {
            p->hooks.deallocate(p->buffer);
            p->length = 0;
            p->buffer = NULL;

            return NULL;
        }
        if (newbuffer)
        {
            memcpy(newbuffer, p->buffer, p->offset + 1);
        }
        p->hooks.deallocate(p->buffer);
    }
    p->length = newsize;
    p->buffer = newbuffer;

    return newbuffer + p->offset;
}

static void update_offset(printbuffer* const buffer)
{
    const unsigned char* buffer_pointer = NULL;
    if ((buffer == NULL) || (buffer->buffer == NULL))
    {
        return;
    }
    buffer_pointer = buffer->buffer + buffer->offset;

    buffer->offset += strlen((const char*)buffer_pointer);
}

static cJSON_bool compare_double(double a, double b)
{
    double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

static cJSON_bool print_number(const cJSON* const item, printbuffer* const output_buffer)
{
    unsigned char* output_pointer = NULL;
    double d = item->valuedouble;
    int length = 0;
    size_t i = 0;
    unsigned char number_buffer[26] = { 0 };         
    unsigned char decimal_point = get_decimal_point();
    double test = 0.0;

    if (output_buffer == NULL)
    {
        return false;
    }

    if (isnan(d) || isinf(d))
    {
        length = sprintf((char*)number_buffer, "null");
    }
    else
    {
        length = sprintf((char*)number_buffer, "%1.15g", d);

        if ((sscanf((char*)number_buffer, "%lg", &test) != 1) || !compare_double((double)test, d))
        {
            length = sprintf((char*)number_buffer, "%1.17g", d);
        }
    }

    if ((length < 0) || (length > (int)(sizeof(number_buffer) - 1)))
    {
        return false;
    }

    output_pointer = ensure(output_buffer, (size_t)length + sizeof(""));
    if (output_pointer == NULL)
    {
        return false;
    }

    for (i = 0; i < ((size_t)length); i++)
    {
        if (number_buffer[i] == decimal_point)
        {
            output_pointer[i] = '.';
            continue;
        }

        output_pointer[i] = number_buffer[i];
    }
    output_pointer[i] = '\0';

    output_buffer->offset += (size_t)length;

    return true;
}

static unsigned parse_hex4(const unsigned char* const input)
{
    unsigned int h = 0;
    size_t i = 0;

    for (i = 0; i < 4; i++)
    {
        if ((input[i] >= '0') && (input[i] <= '9'))
        {
            h += (unsigned int)input[i] - '0';
        }
        else if ((input[i] >= 'A') && (input[i] <= 'F'))
        {
            h += (unsigned int)10 + input[i] - 'A';
        }
        else if ((input[i] >= 'a') && (input[i] <= 'f'))
        {
            h += (unsigned int)10 + input[i] - 'a';
        }
        else   
        {
            return 0;
        }

        if (i < 3)
        {
            h = h << 4;
        }
    }

    return h;
}

static unsigned char utf16_literal_to_utf8(const unsigned char* const input_pointer, const unsigned char* const input_end, unsigned char** output_pointer)
{
    long unsigned int codepoint = 0;
    unsigned int first_code = 0;
    const unsigned char* first_sequence = input_pointer;
    unsigned char utf8_length = 0;
    unsigned char utf8_position = 0;
    unsigned char sequence_length = 0;
    unsigned char first_byte_mark = 0;

    if ((input_end - first_sequence) < 6)
    {
        goto fail;
    }

    first_code = parse_hex4(first_sequence + 2);

    if (((first_code >= 0xDC00) && (first_code <= 0xDFFF)))
    {
        goto fail;
    }

    if ((first_code >= 0xD800) && (first_code <= 0xDBFF))
    {
        const unsigned char* second_sequence = first_sequence + 6;
        unsigned int second_code = 0;
        sequence_length = 12;   

        if ((input_end - second_sequence) < 6)
        {
            goto fail;
        }

        if ((second_sequence[0] != '\\') || (second_sequence[1] != 'u'))
        {
            goto fail;
        }

        second_code = parse_hex4(second_sequence + 2);
        if ((second_code < 0xDC00) || (second_code > 0xDFFF))
        {
            goto fail;
        }


        codepoint = 0x10000 + (((first_code & 0x3FF) << 10) | (second_code & 0x3FF));
    }
    else
    {
        sequence_length = 6;   
        codepoint = first_code;
    }

    if (codepoint < 0x80)
    {
        utf8_length = 1;
    }
    else if (codepoint < 0x800)
    {
        utf8_length = 2;
        first_byte_mark = 0xC0;   
    }
    else if (codepoint < 0x10000)
    {
        utf8_length = 3;
        first_byte_mark = 0xE0;   
    }
    else if (codepoint <= 0x10FFFF)
    {
        utf8_length = 4;
        first_byte_mark = 0xF0;   
    }
    else
    {
        goto fail;
    }

    for (utf8_position = (unsigned char)(utf8_length - 1); utf8_position > 0; utf8_position--)
    {
        (*output_pointer)[utf8_position] = (unsigned char)((codepoint | 0x80) & 0xBF);
        codepoint >>= 6;
    }
    if (utf8_length > 1)
    {
        (*output_pointer)[0] = (unsigned char)((codepoint | first_byte_mark) & 0xFF);
    }
    else
    {
        (*output_pointer)[0] = (unsigned char)(codepoint & 0x7F);
    }

    *output_pointer += utf8_length;

    return sequence_length;

fail:
    return 0;
}

static cJSON_bool parse_string(cJSON* const item, parse_buffer* const input_buffer)
{
    const unsigned char* input_pointer = buffer_at_offset(input_buffer) + 1;
    const unsigned char* input_end = buffer_at_offset(input_buffer) + 1;
    unsigned char* output_pointer = NULL;
    unsigned char* output = NULL;

    if (buffer_at_offset(input_buffer)[0] != '\"')
    {
        goto fail;
    }

    {
        size_t allocation_length = 0;
        size_t skipped_bytes = 0;
        while (((size_t)(input_end - input_buffer->content) < input_buffer->length) && (*input_end != '\"'))
        {
            if (input_end[0] == '\\')
            {
                if ((size_t)(input_end + 1 - input_buffer->content) >= input_buffer->length)
                {
                    goto fail;
                }
                skipped_bytes++;
                input_end++;
            }
            input_end++;
        }
        if (((size_t)(input_end - input_buffer->content) >= input_buffer->length) || (*input_end != '\"'))
        {
            goto fail;     
        }

        allocation_length = (size_t)(input_end - buffer_at_offset(input_buffer)) - skipped_bytes;
        output = (unsigned char*)input_buffer->hooks.allocate(allocation_length + sizeof(""));
        if (output == NULL)
        {
            goto fail;    
        }
    }

    output_pointer = output;
    while (input_pointer < input_end)
    {
        if (*input_pointer != '\\')
        {
            *output_pointer++ = *input_pointer++;
        }
        else
        {
            unsigned char sequence_length = 2;
            if ((input_end - input_pointer) < 1)
            {
                goto fail;
            }

            switch (input_pointer[1])
            {
            case 'b':
                *output_pointer++ = '\b';
                break;
            case 'f':
                *output_pointer++ = '\f';
                break;
            case 'n':
                *output_pointer++ = '\n';
                break;
            case 'r':
                *output_pointer++ = '\r';
                break;
            case 't':
                *output_pointer++ = '\t';
                break;
            case '\"':
            case '\\':
            case '/':
                *output_pointer++ = input_pointer[1];
                break;

            case 'u':
                sequence_length = utf16_literal_to_utf8(input_pointer, input_end, &output_pointer);
                if (sequence_length == 0)
                {
                    goto fail;
                }
                break;

            default:
                goto fail;
            }
            input_pointer += sequence_length;
        }
    }

    *output_pointer = '\0';

    item->type = cJSON_String;
    item->valuestring = (char*)output;

    input_buffer->offset = (size_t)(input_end - input_buffer->content);
    input_buffer->offset++;

    return true;

fail:
    if (output != NULL)
    {
        input_buffer->hooks.deallocate(output);
    }

    if (input_pointer != NULL)
    {
        input_buffer->offset = (size_t)(input_pointer - input_buffer->content);
    }

    return false;
}

static cJSON_bool print_string_ptr(const unsigned char* const input, printbuffer* const output_buffer)
{
    const unsigned char* input_pointer = NULL;
    unsigned char* output = NULL;
    unsigned char* output_pointer = NULL;
    size_t output_length = 0;
    size_t escape_characters = 0;

    if (output_buffer == NULL)
    {
        return false;
    }

    if (input == NULL)
    {
        output = ensure(output_buffer, sizeof("\"\""));
        if (output == NULL)
        {
            return false;
        }
        strcpy((char*)output, "\"\"");

        return true;
    }

    for (input_pointer = input; *input_pointer; input_pointer++)
    {
        switch (*input_pointer)
        {
        case '\"':
        case '\\':
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
            escape_characters++;
            break;
        default:
            if (*input_pointer < 32)
            {
                escape_characters += 5;
            }
            break;
        }
    }
    output_length = (size_t)(input_pointer - input) + escape_characters;

    output = ensure(output_buffer, output_length + sizeof("\"\""));
    if (output == NULL)
    {
        return false;
    }

    if (escape_characters == 0)
    {
        output[0] = '\"';
        memcpy(output + 1, input, output_length);
        output[output_length + 1] = '\"';
        output[output_length + 2] = '\0';

        return true;
    }

    output[0] = '\"';
    output_pointer = output + 1;
    for (input_pointer = input; *input_pointer != '\0'; (void)input_pointer++, output_pointer++)
    {
        if ((*input_pointer > 31) && (*input_pointer != '\"') && (*input_pointer != '\\'))
        {
            *output_pointer = *input_pointer;
        }
        else
        {
            *output_pointer++ = '\\';
            switch (*input_pointer)
            {
            case '\\':
                *output_pointer = '\\';
                break;
            case '\"':
                *output_pointer = '\"';
                break;
            case '\b':
                *output_pointer = 'b';
                break;
            case '\f':
                *output_pointer = 'f';
                break;
            case '\n':
                *output_pointer = 'n';
                break;
            case '\r':
                *output_pointer = 'r';
                break;
            case '\t':
                *output_pointer = 't';
                break;
            default:
                sprintf((char*)output_pointer, "u%04x", *input_pointer);
                output_pointer += 4;
                break;
            }
        }
    }
    output[output_length + 1] = '\"';
    output[output_length + 2] = '\0';

    return true;
}

static cJSON_bool print_string(const cJSON* const item, printbuffer* const p)
{
    return print_string_ptr((unsigned char*)item->valuestring, p);
}

static cJSON_bool parse_value(cJSON* const item, parse_buffer* const input_buffer);
static cJSON_bool print_value(const cJSON* const item, printbuffer* const output_buffer);
static cJSON_bool parse_array(cJSON* const item, parse_buffer* const input_buffer);
static cJSON_bool print_array(const cJSON* const item, printbuffer* const output_buffer);
static cJSON_bool parse_object(cJSON* const item, parse_buffer* const input_buffer);
static cJSON_bool print_object(const cJSON* const item, printbuffer* const output_buffer);

static parse_buffer* buffer_skip_whitespace(parse_buffer* const buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL))
    {
        return NULL;
    }

    if (cannot_access_at_index(buffer, 0))
    {
        return buffer;
    }

    while (can_access_at_index(buffer, 0) && (buffer_at_offset(buffer)[0] <= 32))
    {
        buffer->offset++;
    }

    if (buffer->offset == buffer->length)
    {
        buffer->offset--;
    }

    return buffer;
}

static parse_buffer* skip_utf8_bom(parse_buffer* const buffer)
{
    if ((buffer == NULL) || (buffer->content == NULL) || (buffer->offset != 0))
    {
        return NULL;
    }

    if (can_access_at_index(buffer, 4) && (strncmp((const char*)buffer_at_offset(buffer), "\xEF\xBB\xBF", 3) == 0))
    {
        buffer->offset += 3;
    }

    return buffer;
}

CJSON_PUBLIC(cJSON*) cJSON_ParseWithOpts(const char* value, const char** return_parse_end, cJSON_bool require_null_terminated)
{
    size_t buffer_length;

    if (NULL == value)
    {
        return NULL;
    }

    buffer_length = strlen(value) + sizeof("");

    return cJSON_ParseWithLengthOpts(value, buffer_length, return_parse_end, require_null_terminated);
}

CJSON_PUBLIC(cJSON*) cJSON_ParseWithLengthOpts(const char* value, size_t buffer_length, const char** return_parse_end, cJSON_bool require_null_terminated)
{
    parse_buffer buffer = { 0, 0, 0, 0, { 0, 0, 0 } };
    cJSON* item = NULL;

    global_error.json = NULL;
    global_error.position = 0;

    if (value == NULL || 0 == buffer_length)
    {
        goto fail;
    }

    buffer.content = (const unsigned char*)value;
    buffer.length = buffer_length;
    buffer.offset = 0;
    buffer.hooks = global_hooks;

    item = cJSON_New_Item(&global_hooks);
    if (item == NULL)    
    {
        goto fail;
    }

    if (!parse_value(item, buffer_skip_whitespace(skip_utf8_bom(&buffer))))
    {
        goto fail;
    }

    if (require_null_terminated)
    {
        buffer_skip_whitespace(&buffer);
        if ((buffer.offset >= buffer.length) || buffer_at_offset(&buffer)[0] != '\0')
        {
            goto fail;
        }
    }
    if (return_parse_end)
    {
        *return_parse_end = (const char*)buffer_at_offset(&buffer);
    }

    return item;

fail:
    if (item != NULL)
    {
        cJSON_Delete(item);
    }

    if (value != NULL)
    {
        error local_error;
        local_error.json = (const unsigned char*)value;
        local_error.position = 0;

        if (buffer.offset < buffer.length)
        {
            local_error.position = buffer.offset;
        }
        else if (buffer.length > 0)
        {
            local_error.position = buffer.length - 1;
        }

        if (return_parse_end != NULL)
        {
            *return_parse_end = (const char*)local_error.json + local_error.position;
        }

        global_error = local_error;
    }

    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_Parse(const char* value)
{
    return cJSON_ParseWithOpts(value, 0, 0);
}

CJSON_PUBLIC(cJSON*) cJSON_ParseWithLength(const char* value, size_t buffer_length)
{
    return cJSON_ParseWithLengthOpts(value, buffer_length, 0, 0);
}

#define cjson_min(a, b) (((a) < (b)) ? (a) : (b))

static unsigned char* print(const cJSON* const item, cJSON_bool format, const internal_hooks* const hooks)
{
    static const size_t default_buffer_size = 256;
    printbuffer buffer[1];
    unsigned char* printed = NULL;

    memset(buffer, 0, sizeof(buffer));

    buffer->buffer = (unsigned char*)hooks->allocate(default_buffer_size);
    buffer->length = default_buffer_size;
    buffer->format = format;
    buffer->hooks = *hooks;
    if (buffer->buffer == NULL)
    {
        goto fail;
    }

    if (!print_value(item, buffer))
    {
        goto fail;
    }
    update_offset(buffer);

    if (hooks->reallocate != NULL)
    {
        printed = (unsigned char*)hooks->reallocate(buffer->buffer, buffer->offset + 1);
        if (printed == NULL) {
            goto fail;
        }
        buffer->buffer = NULL;
    }
    else           
    {
        printed = (unsigned char*)hooks->allocate(buffer->offset + 1);
        if (printed == NULL)
        {
            goto fail;
        }
        memcpy(printed, buffer->buffer, cjson_min(buffer->length, buffer->offset + 1));
        printed[buffer->offset] = '\0';      

        hooks->deallocate(buffer->buffer);
    }

    return printed;

fail:
    if (buffer->buffer != NULL)
    {
        hooks->deallocate(buffer->buffer);
    }

    if (printed != NULL)
    {
        hooks->deallocate(printed);
    }

    return NULL;
}

CJSON_PUBLIC(char*) cJSON_Print(const cJSON* item)
{
    return (char*)print(item, true, &global_hooks);
}

CJSON_PUBLIC(char*) cJSON_PrintUnformatted(const cJSON* item)
{
    return (char*)print(item, false, &global_hooks);
}

CJSON_PUBLIC(char*) cJSON_PrintBuffered(const cJSON* item, int prebuffer, cJSON_bool fmt)
{
    printbuffer p = { 0, 0, 0, 0, 0, 0, { 0, 0, 0 } };

    if (prebuffer < 0)
    {
        return NULL;
    }

    p.buffer = (unsigned char*)global_hooks.allocate((size_t)prebuffer);
    if (!p.buffer)
    {
        return NULL;
    }

    p.length = (size_t)prebuffer;
    p.offset = 0;
    p.noalloc = false;
    p.format = fmt;
    p.hooks = global_hooks;

    if (!print_value(item, &p))
    {
        global_hooks.deallocate(p.buffer);
        return NULL;
    }

    return (char*)p.buffer;
}

CJSON_PUBLIC(cJSON_bool) cJSON_PrintPreallocated(cJSON* item, char* buffer, const int length, const cJSON_bool format)
{
    printbuffer p = { 0, 0, 0, 0, 0, 0, { 0, 0, 0 } };

    if ((length < 0) || (buffer == NULL))
    {
        return false;
    }

    p.buffer = (unsigned char*)buffer;
    p.length = (size_t)length;
    p.offset = 0;
    p.noalloc = true;
    p.format = format;
    p.hooks = global_hooks;

    return print_value(item, &p);
}

static cJSON_bool parse_value(cJSON* const item, parse_buffer* const input_buffer)
{
    if ((input_buffer == NULL) || (input_buffer->content == NULL))
    {
        return false;    
    }

    if (can_read(input_buffer, 4) && (strncmp((const char*)buffer_at_offset(input_buffer), "null", 4) == 0))
    {
        item->type = cJSON_NULL;
        input_buffer->offset += 4;
        return true;
    }
    if (can_read(input_buffer, 5) && (strncmp((const char*)buffer_at_offset(input_buffer), "false", 5) == 0))
    {
        item->type = cJSON_False;
        input_buffer->offset += 5;
        return true;
    }
    if (can_read(input_buffer, 4) && (strncmp((const char*)buffer_at_offset(input_buffer), "true", 4) == 0))
    {
        item->type = cJSON_True;
        item->valueint = 1;
        input_buffer->offset += 4;
        return true;
    }
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '\"'))
    {
        return parse_string(item, input_buffer);
    }
    if (can_access_at_index(input_buffer, 0) && ((buffer_at_offset(input_buffer)[0] == '-') || ((buffer_at_offset(input_buffer)[0] >= '0') && (buffer_at_offset(input_buffer)[0] <= '9'))))
    {
        return parse_number(item, input_buffer);
    }
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '['))
    {
        return parse_array(item, input_buffer);
    }
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '{'))
    {
        return parse_object(item, input_buffer);
    }

    return false;
}

static cJSON_bool print_value(const cJSON* const item, printbuffer* const output_buffer)
{
    unsigned char* output = NULL;

    if ((item == NULL) || (output_buffer == NULL))
    {
        return false;
    }

    switch ((item->type) & 0xFF)
    {
    case cJSON_NULL:
        output = ensure(output_buffer, 5);
        if (output == NULL)
        {
            return false;
        }
        strcpy((char*)output, "null");
        return true;

    case cJSON_False:
        output = ensure(output_buffer, 6);
        if (output == NULL)
        {
            return false;
        }
        strcpy((char*)output, "false");
        return true;

    case cJSON_True:
        output = ensure(output_buffer, 5);
        if (output == NULL)
        {
            return false;
        }
        strcpy((char*)output, "true");
        return true;

    case cJSON_Number:
        return print_number(item, output_buffer);

    case cJSON_Raw:
    {
        size_t raw_length = 0;
        if (item->valuestring == NULL)
        {
            return false;
        }

        raw_length = strlen(item->valuestring) + sizeof("");
        output = ensure(output_buffer, raw_length);
        if (output == NULL)
        {
            return false;
        }
        memcpy(output, item->valuestring, raw_length);
        return true;
    }

    case cJSON_String:
        return print_string(item, output_buffer);

    case cJSON_Array:
        return print_array(item, output_buffer);

    case cJSON_Object:
        return print_object(item, output_buffer);

    default:
        return false;
    }
}

static cJSON_bool parse_array(cJSON* const item, parse_buffer* const input_buffer)
{
    cJSON* head = NULL;       
    cJSON* current_item = NULL;

    if (input_buffer->depth >= CJSON_NESTING_LIMIT)
    {
        return false;     
    }
    input_buffer->depth++;

    if (buffer_at_offset(input_buffer)[0] != '[')
    {
        goto fail;
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ']'))
    {
        goto success;
    }

    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    input_buffer->offset--;
    do
    {
        cJSON* new_item = cJSON_New_Item(&(input_buffer->hooks));
        if (new_item == NULL)
        {
            goto fail;    
        }

        if (head == NULL)
        {
            current_item = head = new_item;
        }
        else
        {
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_value(current_item, input_buffer))
        {
            goto fail;      
        }
        buffer_skip_whitespace(input_buffer);
    } while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) || buffer_at_offset(input_buffer)[0] != ']')
    {
        goto fail;      
    }

success:
    input_buffer->depth--;

    if (head != NULL) {
        head->prev = current_item;
    }

    item->type = cJSON_Array;
    item->child = head;

    input_buffer->offset++;

    return true;

fail:
    if (head != NULL)
    {
        cJSON_Delete(head);
    }

    return false;
}

static cJSON_bool print_array(const cJSON* const item, printbuffer* const output_buffer)
{
    unsigned char* output_pointer = NULL;
    size_t length = 0;
    cJSON* current_element = item->child;
    size_t i;

    if (output_buffer == NULL)
    {
        return false;
    }

    output_pointer = ensure(output_buffer, 1);
    if (output_pointer == NULL)
    {
        return false;
    }

    *output_pointer = '[';
    output_buffer->offset++;
    output_buffer->depth++;

    if (current_element != NULL)
    {
        if (output_buffer->format && current_element->type != cJSON_Object)
        {
            output_pointer = ensure(output_buffer, output_buffer->depth + 1);
            if (output_pointer == NULL)
            {
                return false;
            }

            *output_pointer++ = '\n';

            for (i = 0; i < output_buffer->depth; i++)
            {
                *output_pointer++ = '\t';
            }

            output_buffer->offset += output_buffer->depth + 1;
        }
    }

    while (current_element != NULL)
    {
        if (!print_value(current_element, output_buffer))
        {
            return false;
        }
        update_offset(output_buffer);
        if (current_element->next)
        {
            length = (size_t)(output_buffer->format ? current_element->type == cJSON_Object || current_element->type == cJSON_Number ? 2 : 2 + output_buffer->depth : 1);
            output_pointer = ensure(output_buffer, length);
            if (output_pointer == NULL)
            {
                return false;
            }
            *output_pointer++ = ',';
            if (output_buffer->format)
            {
                if (current_element->type == cJSON_Object || current_element->type == cJSON_Number)
                {
                    *output_pointer++ = ' ';
                }
                else
                {
                    *output_pointer++ = '\n';
                    for (i = 0; i < output_buffer->depth; i++)
                    {
                        *output_pointer++ = '\t';
                    }
                }
            }
            *output_pointer = '\0';
            output_buffer->offset += length;
        }
        current_element = current_element->next;
    }

    if (output_buffer->format)
    {
        output_pointer = ensure(output_buffer, output_buffer->depth);
        if (output_pointer == NULL)
        {
            return false;
        }

        *output_pointer++ = '\n';

        for (i = 0; i < output_buffer->depth - 1; i++)
        {
            *output_pointer++ = '\t';
        }

        output_buffer->offset += output_buffer->depth;
    }

    output_pointer = ensure(output_buffer, 2);
    if (output_pointer == NULL)
    {
        return false;
    }
    *output_pointer++ = ']';
    *output_pointer = '\0';
    output_buffer->depth--;

    return true;
}

static cJSON_bool parse_object(cJSON* const item, parse_buffer* const input_buffer)
{
    cJSON* head = NULL;     
    cJSON* current_item = NULL;

    if (input_buffer->depth >= CJSON_NESTING_LIMIT)
    {
        return false;     
    }
    input_buffer->depth++;

    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '{'))
    {
        goto fail;     
    }

    input_buffer->offset++;
    buffer_skip_whitespace(input_buffer);
    if (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == '}'))
    {
        goto success;    
    }

    if (cannot_access_at_index(input_buffer, 0))
    {
        input_buffer->offset--;
        goto fail;
    }

    input_buffer->offset--;
    do
    {
        cJSON* new_item = cJSON_New_Item(&(input_buffer->hooks));
        if (new_item == NULL)
        {
            goto fail;    
        }

        if (head == NULL)
        {
            current_item = head = new_item;
        }
        else
        {
            current_item->next = new_item;
            new_item->prev = current_item;
            current_item = new_item;
        }

        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_string(current_item, input_buffer))
        {
            goto fail;      
        }
        buffer_skip_whitespace(input_buffer);

        current_item->string = current_item->valuestring;
        current_item->valuestring = NULL;

        if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != ':'))
        {
            goto fail;    
        }

        input_buffer->offset++;
        buffer_skip_whitespace(input_buffer);
        if (!parse_value(current_item, input_buffer))
        {
            goto fail;      
        }
        buffer_skip_whitespace(input_buffer);
    } while (can_access_at_index(input_buffer, 0) && (buffer_at_offset(input_buffer)[0] == ','));

    if (cannot_access_at_index(input_buffer, 0) || (buffer_at_offset(input_buffer)[0] != '}'))
    {
        goto fail;      
    }

success:
    input_buffer->depth--;

    if (head != NULL) {
        head->prev = current_item;
    }

    item->type = cJSON_Object;
    item->child = head;

    input_buffer->offset++;
    return true;

fail:
    if (head != NULL)
    {
        cJSON_Delete(head);
    }

    return false;
}

static cJSON_bool print_object(const cJSON* const item, printbuffer* const output_buffer)
{
    unsigned char* output_pointer = NULL;
    size_t length = 0;
    size_t i;
    cJSON* current_item = item->child;

    if (output_buffer == NULL)
    {
        return false;
    }

    if (output_buffer->format)
    {
        length = (size_t)(output_buffer->depth ? output_buffer->depth + 1 : 0) + 2;       
        output_pointer = ensure(output_buffer, length + 1);
        if (output_pointer == NULL)
        {
            return false;
        }

        if (output_buffer->depth)
        {
            *output_pointer++ = '\n';
        }

        for (i = 0; i < output_buffer->depth; i++)
        {
            *output_pointer++ = '\t';
        }

        *output_pointer++ = '{';
        *output_pointer++ = '\n';

        output_buffer->depth++;
        output_buffer->offset += length;

    }
    else
    {
        length = (size_t)1;    
        output_pointer = ensure(output_buffer, length + 1);
        if (output_pointer == NULL)
        {
            return false;
        }

        *output_pointer++ = '{';

        output_buffer->depth++;
        output_buffer->offset += length;
    }

    while (current_item)
    {
        if (output_buffer->format)
        {
            size_t i;
            output_pointer = ensure(output_buffer, output_buffer->depth);
            if (output_pointer == NULL)
            {
                return false;
            }
            for (i = 0; i < output_buffer->depth; i++)
            {
                *output_pointer++ = '\t';
            }
            output_buffer->offset += output_buffer->depth;
        }

        if (!print_string_ptr((unsigned char*)current_item->string, output_buffer))
        {
            return false;
        }
        update_offset(output_buffer);

        length = (size_t)(output_buffer->format ? 2 : 1);
        output_pointer = ensure(output_buffer, length);
        if (output_pointer == NULL)
        {
            return false;
        }
        *output_pointer++ = ':';
        if (output_buffer->format)
        {
            *output_pointer++ = ' ';
        }
        output_buffer->offset += length;

        if (!print_value(current_item, output_buffer))
        {
            return false;
        }
        update_offset(output_buffer);

        length = ((size_t)(output_buffer->format ? 1 : 0) + (size_t)(current_item->next ? 1 : 0));
        output_pointer = ensure(output_buffer, length + 1);
        if (output_pointer == NULL)
        {
            return false;
        }
        if (current_item->next)
        {
            *output_pointer++ = ',';
        }

        if (output_buffer->format)
        {
            *output_pointer++ = '\n';
        }
        *output_pointer = '\0';
        output_buffer->offset += length;

        current_item = current_item->next;
    }

    output_pointer = ensure(output_buffer, output_buffer->format ? (output_buffer->depth + 1) : 2);
    if (output_pointer == NULL)
    {
        return false;
    }
    if (output_buffer->format)
    {
        size_t i;
        for (i = 0; i < (output_buffer->depth - 1); i++)
        {
            *output_pointer++ = '\t';
        }
    }
    *output_pointer++ = '}';
    *output_pointer = '\0';
    output_buffer->depth--;

    return true;
}


CJSON_PUBLIC(int) cJSON_GetArraySize(const cJSON* array)
{
    cJSON* child = NULL;
    size_t size = 0;

    if (array == NULL)
    {
        return 0;
    }

    child = array->child;

    while (child != NULL)
    {
        size++;
        child = child->next;
    }

    return (int)size;
}

static cJSON* get_array_item(const cJSON* array, size_t index)
{
    cJSON* current_child = NULL;

    if (array == NULL)
    {
        return NULL;
    }

    current_child = array->child;
    while ((current_child != NULL) && (index > 0))
    {
        index--;
        current_child = current_child->next;
    }

    return current_child;
}

CJSON_PUBLIC(cJSON*) cJSON_GetArrayItem(const cJSON* array, int index)
{
    if (index < 0)
    {
        return NULL;
    }

    return get_array_item(array, (size_t)index);
}

static cJSON* get_object_item(const cJSON* const object, const char* const name, const cJSON_bool case_sensitive)
{
    cJSON* current_element = NULL;

    if ((object == NULL) || (name == NULL))
    {
        return NULL;
    }

    current_element = object->child;
    if (case_sensitive)
    {
        while ((current_element != NULL) && (current_element->string != NULL) && (strcmp(name, current_element->string) != 0))
        {
            current_element = current_element->next;
        }
    }
    else
    {
        while ((current_element != NULL) && (case_insensitive_strcmp((const unsigned char*)name, (const unsigned char*)(current_element->string)) != 0))
        {
            current_element = current_element->next;
        }
    }

    if ((current_element == NULL) || (current_element->string == NULL)) {
        return NULL;
    }

    return current_element;
}

CJSON_PUBLIC(cJSON*) cJSON_GetObjectItem(const cJSON* const object, const char* const string)
{
    return get_object_item(object, string, false);
}

CJSON_PUBLIC(cJSON*) cJSON_GetObjectItemCaseSensitive(const cJSON* const object, const char* const string)
{
    return get_object_item(object, string, true);
}

CJSON_PUBLIC(cJSON_bool) cJSON_HasObjectItem(const cJSON* object, const char* string)
{
    return cJSON_GetObjectItem(object, string) ? 1 : 0;
}

static void suffix_object(cJSON* prev, cJSON* item)
{
    prev->next = item;
    item->prev = prev;
}

static cJSON* create_reference(const cJSON* item, const internal_hooks* const hooks)
{
    cJSON* reference = NULL;
    if (item == NULL)
    {
        return NULL;
    }

    reference = cJSON_New_Item(hooks);
    if (reference == NULL)
    {
        return NULL;
    }

    memcpy(reference, item, sizeof(cJSON));
    reference->string = NULL;
    reference->type |= cJSON_IsReference;
    reference->next = reference->prev = NULL;
    return reference;
}

static cJSON_bool add_item_to_array(cJSON* array, cJSON* item)
{
    cJSON* child = NULL;

    if ((item == NULL) || (array == NULL) || (array == item))
    {
        return false;
    }

    child = array->child;
    if (child == NULL)
    {
        array->child = item;
        item->prev = item;
        item->next = NULL;
    }
    else
    {
        if (child->prev)
        {
            suffix_object(child->prev, item);
            array->child->prev = item;
        }
        else
        {
            while (child->next)
            {
                child = child->next;
            }
            suffix_object(child, item);
            array->child->prev = item;
        }
    }

    return true;
}

CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToArray(cJSON* array, cJSON* item)
{
    return add_item_to_array(array, item);
}

#if defined(__clang__) || (defined(__GNUC__)  && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
#pragma GCC diagnostic push
#endif
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
static void* cast_away_const(const void* string)
{
    return (void*)string;
}
#if defined(__clang__) || (defined(__GNUC__)  && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ > 5))))
#pragma GCC diagnostic pop
#endif


static cJSON_bool add_item_to_object(cJSON* const object, const char* const string, cJSON* const item, const internal_hooks* const hooks, const cJSON_bool constant_key)
{
    char* new_key = NULL;
    int new_type = cJSON_Invalid;

    if ((object == NULL) || (string == NULL) || (item == NULL) || (object == item))
    {
        return false;
    }

    if (constant_key)
    {
        new_key = (char*)cast_away_const(string);
        new_type = item->type | cJSON_StringIsConst;
    }
    else
    {
        new_key = (char*)cJSON_strdup((const unsigned char*)string, hooks);
        if (new_key == NULL)
        {
            return false;
        }

        new_type = item->type & ~cJSON_StringIsConst;
    }

    if (!(item->type & cJSON_StringIsConst) && (item->string != NULL))
    {
        hooks->deallocate(item->string);
    }

    item->string = new_key;
    item->type = new_type;

    return add_item_to_array(object, item);
}

CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToObject(cJSON* object, const char* string, cJSON* item)
{
    return add_item_to_object(object, string, item, &global_hooks, false);
}

CJSON_PUBLIC(cJSON_bool) cJSON_AddItemToObjectCS(cJSON* object, const char* string, cJSON* item)
{
    return add_item_to_object(object, string, item, &global_hooks, true);
}

CJSON_PUBLIC(cJSON_bool) cJSON_AddItemReferenceToArray(cJSON* array, cJSON* item)
{
    if (array == NULL)
    {
        return false;
    }

    return add_item_to_array(array, create_reference(item, &global_hooks));
}

CJSON_PUBLIC(cJSON_bool) cJSON_AddItemReferenceToObject(cJSON* object, const char* string, cJSON* item)
{
    if ((object == NULL) || (string == NULL))
    {
        return false;
    }

    return add_item_to_object(object, string, create_reference(item, &global_hooks), &global_hooks, false);
}

CJSON_PUBLIC(cJSON*) cJSON_AddNullToObject(cJSON* const object, const char* const name)
{
    cJSON* null = cJSON_CreateNull();
    if (add_item_to_object(object, name, null, &global_hooks, false))
    {
        return null;
    }

    cJSON_Delete(null);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddTrueToObject(cJSON* const object, const char* const name)
{
    cJSON* true_item = cJSON_CreateTrue();
    if (add_item_to_object(object, name, true_item, &global_hooks, false))
    {
        return true_item;
    }

    cJSON_Delete(true_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddFalseToObject(cJSON* const object, const char* const name)
{
    cJSON* false_item = cJSON_CreateFalse();
    if (add_item_to_object(object, name, false_item, &global_hooks, false))
    {
        return false_item;
    }

    cJSON_Delete(false_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddBoolToObject(cJSON* const object, const char* const name, const cJSON_bool boolean)
{
    cJSON* bool_item = cJSON_CreateBool(boolean);
    if (add_item_to_object(object, name, bool_item, &global_hooks, false))
    {
        return bool_item;
    }

    cJSON_Delete(bool_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddNumberToObject(cJSON* const object, const char* const name, const double number)
{
    cJSON* number_item = cJSON_CreateNumber(number);
    if (add_item_to_object(object, name, number_item, &global_hooks, false))
    {
        return number_item;
    }

    cJSON_Delete(number_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddStringToObject(cJSON* const object, const char* const name, const char* const string)
{
    cJSON* string_item = cJSON_CreateString(string);
    if (add_item_to_object(object, name, string_item, &global_hooks, false))
    {
        return string_item;
    }

    cJSON_Delete(string_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddRawToObject(cJSON* const object, const char* const name, const char* const raw)
{
    cJSON* raw_item = cJSON_CreateRaw(raw);
    if (add_item_to_object(object, name, raw_item, &global_hooks, false))
    {
        return raw_item;
    }

    cJSON_Delete(raw_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddObjectToObject(cJSON* const object, const char* const name)
{
    cJSON* object_item = cJSON_CreateObject();
    if (add_item_to_object(object, name, object_item, &global_hooks, false))
    {
        return object_item;
    }

    cJSON_Delete(object_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddArrayToObject(cJSON* const object, const char* const name)
{
    cJSON* array = cJSON_CreateArray();
    if (add_item_to_object(object, name, array, &global_hooks, false))
    {
        return array;
    }

    cJSON_Delete(array);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_DetachItemViaPointer(cJSON* parent, cJSON* const item)
{
    if ((parent == NULL) || (item == NULL))
    {
        return NULL;
    }

    if (item != parent->child)
    {
        item->prev->next = item->next;
    }
    if (item->next != NULL)
    {
        item->next->prev = item->prev;
    }

    if (item == parent->child)
    {
        parent->child = item->next;
    }
    else if (item->next == NULL)
    {
        parent->child->prev = item->prev;
    }

    item->prev = NULL;
    item->next = NULL;

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_DetachItemFromArray(cJSON* array, int which)
{
    if (which < 0)
    {
        return NULL;
    }

    return cJSON_DetachItemViaPointer(array, get_array_item(array, (size_t)which));
}

CJSON_PUBLIC(void) cJSON_DeleteItemFromArray(cJSON* array, int which)
{
    cJSON_Delete(cJSON_DetachItemFromArray(array, which));
}

CJSON_PUBLIC(cJSON*) cJSON_DetachItemFromObject(cJSON* object, const char* string)
{
    cJSON* to_detach = cJSON_GetObjectItem(object, string);

    return cJSON_DetachItemViaPointer(object, to_detach);
}

CJSON_PUBLIC(cJSON*) cJSON_DetachItemFromObjectCaseSensitive(cJSON* object, const char* string)
{
    cJSON* to_detach = cJSON_GetObjectItemCaseSensitive(object, string);

    return cJSON_DetachItemViaPointer(object, to_detach);
}

CJSON_PUBLIC(void) cJSON_DeleteItemFromObject(cJSON* object, const char* string)
{
    cJSON_Delete(cJSON_DetachItemFromObject(object, string));
}

CJSON_PUBLIC(void) cJSON_DeleteItemFromObjectCaseSensitive(cJSON* object, const char* string)
{
    cJSON_Delete(cJSON_DetachItemFromObjectCaseSensitive(object, string));
}

CJSON_PUBLIC(cJSON_bool) cJSON_InsertItemInArray(cJSON* array, int which, cJSON* newitem)
{
    cJSON* after_inserted = NULL;

    if (which < 0)
    {
        return false;
    }

    after_inserted = get_array_item(array, (size_t)which);
    if (after_inserted == NULL)
    {
        return add_item_to_array(array, newitem);
    }

    newitem->next = after_inserted;
    newitem->prev = after_inserted->prev;
    after_inserted->prev = newitem;
    if (after_inserted == array->child)
    {
        array->child = newitem;
    }
    else
    {
        newitem->prev->next = newitem;
    }
    return true;
}

CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemViaPointer(cJSON* const parent, cJSON* const item, cJSON* replacement)
{
    if ((parent == NULL) || (replacement == NULL) || (item == NULL))
    {
        return false;
    }

    if (replacement == item)
    {
        return true;
    }

    replacement->next = item->next;
    replacement->prev = item->prev;

    if (replacement->next != NULL)
    {
        replacement->next->prev = replacement;
    }
    if (parent->child == item)
    {
        if (parent->child->prev == parent->child)
        {
            replacement->prev = replacement;
        }
        parent->child = replacement;
    }
    else
    {   
        if (replacement->prev != NULL)
        {
            replacement->prev->next = replacement;
        }
        if (replacement->next == NULL)
        {
            parent->child->prev = replacement;
        }
    }

    item->next = NULL;
    item->prev = NULL;
    cJSON_Delete(item);

    return true;
}

CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInArray(cJSON* array, int which, cJSON* newitem)
{
    if (which < 0)
    {
        return false;
    }

    return cJSON_ReplaceItemViaPointer(array, get_array_item(array, (size_t)which), newitem);
}

static cJSON_bool replace_item_in_object(cJSON* object, const char* string, cJSON* replacement, cJSON_bool case_sensitive)
{
    if ((replacement == NULL) || (string == NULL))
    {
        return false;
    }

    if (!(replacement->type & cJSON_StringIsConst) && (replacement->string != NULL))
    {
        cJSON_free(replacement->string);
    }
    replacement->string = (char*)cJSON_strdup((const unsigned char*)string, &global_hooks);
    replacement->type &= ~cJSON_StringIsConst;

    return cJSON_ReplaceItemViaPointer(object, get_object_item(object, string, case_sensitive), replacement);
}

CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInObject(cJSON* object, const char* string, cJSON* newitem)
{
    return replace_item_in_object(object, string, newitem, false);
}

CJSON_PUBLIC(cJSON_bool) cJSON_ReplaceItemInObjectCaseSensitive(cJSON* object, const char* string, cJSON* newitem)
{
    return replace_item_in_object(object, string, newitem, true);
}

CJSON_PUBLIC(cJSON*) cJSON_CreateNull(void)
{
    cJSON* item = cJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = cJSON_NULL;
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateTrue(void)
{
    cJSON* item = cJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = cJSON_True;
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateFalse(void)
{
    cJSON* item = cJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = cJSON_False;
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateBool(cJSON_bool boolean)
{
    cJSON* item = cJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = boolean ? cJSON_True : cJSON_False;
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateNumber(double num)
{
    cJSON* item = cJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = cJSON_Number;
        item->valuedouble = num;

        if (num >= SIZEM_MAX)
        {
            item->valueint = SIZEM_MAX;
        }
        else if (num <= (double)SIZEM_MIN)
        {
            item->valueint = SIZEM_MIN;
        }
        else
        {
            item->valueint = (size_t)num;
        }
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateString(const char* string)
{
    cJSON* item = cJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = cJSON_String;
        item->valuestring = (char*)cJSON_strdup((const unsigned char*)string, &global_hooks);
        if (!item->valuestring)
        {
            cJSON_Delete(item);
            return NULL;
        }
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateStringReference(const char* string)
{
    cJSON* item = cJSON_New_Item(&global_hooks);
    if (item != NULL)
    {
        item->type = cJSON_String | cJSON_IsReference;
        item->valuestring = (char*)cast_away_const(string);
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateObjectReference(const cJSON* child)
{
    cJSON* item = cJSON_New_Item(&global_hooks);
    if (item != NULL) {
        item->type = cJSON_Object | cJSON_IsReference;
        item->child = (cJSON*)cast_away_const(child);
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateArrayReference(const cJSON* child) {
    cJSON* item = cJSON_New_Item(&global_hooks);
    if (item != NULL) {
        item->type = cJSON_Array | cJSON_IsReference;
        item->child = (cJSON*)cast_away_const(child);
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateRaw(const char* raw)
{
    cJSON* item = cJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = cJSON_Raw;
        item->valuestring = (char*)cJSON_strdup((const unsigned char*)raw, &global_hooks);
        if (!item->valuestring)
        {
            cJSON_Delete(item);
            return NULL;
        }
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateArray(void)
{
    cJSON* item = cJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = cJSON_Array;
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateObject(void)
{
    cJSON* item = cJSON_New_Item(&global_hooks);
    if (item)
    {
        item->type = cJSON_Object;
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateIntArray(int* numbers, int count)
{
    size_t i = 0;
    cJSON* n = NULL;
    cJSON* p = NULL;
    cJSON* a = NULL;

    if ((count < 0) || (numbers == NULL))
    {
        return NULL;
    }

    a = cJSON_CreateArray();
    for (i = 0; a && (i < (size_t)count); i++)
    {
        n = cJSON_CreateNumber(numbers[i]);
        if (!n)
        {
            cJSON_Delete(a);
            return NULL;
        }
        if (!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }

    return a;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateFloatArray(float* numbers, int count)
{
    size_t i = 0;
    cJSON* n = NULL;
    cJSON* p = NULL;
    cJSON* a = NULL;

    if ((count < 0) || (numbers == NULL))
    {
        return NULL;
    }

    a = cJSON_CreateArray();

    for (i = 0; a && (i < (size_t)count); i++)
    {
        n = cJSON_CreateNumber((double)numbers[i]);
        if (!n)
        {
            cJSON_Delete(a);
            return NULL;
        }
        if (!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }

    return a;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateDoubleArray(double* numbers, int count)
{
    size_t i = 0;
    cJSON* n = NULL;
    cJSON* p = NULL;
    cJSON* a = NULL;

    if ((count < 0) || (numbers == NULL))
    {
        return NULL;
    }

    a = cJSON_CreateArray();

    for (i = 0; a && (i < (size_t)count); i++)
    {
        n = cJSON_CreateNumber(numbers[i]);
        if (!n)
        {
            cJSON_Delete(a);
            return NULL;
        }
        if (!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }

    return a;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateStringArray(char* const* strings, int count)
{
    size_t i = 0;
    cJSON* n = NULL;
    cJSON* p = NULL;
    cJSON* a = NULL;

    if ((count < 0) || (strings == NULL))
    {
        return NULL;
    }

    a = cJSON_CreateArray();

    for (i = 0; a && (i < (size_t)count); i++)
    {
        n = cJSON_CreateString(strings[i]);
        if (!n)
        {
            cJSON_Delete(a);
            return NULL;
        }
        if (!i)
        {
            a->child = n;
        }
        else
        {
            suffix_object(p, n);
        }
        p = n;
    }

    return a;
}

CJSON_PUBLIC(cJSON*) cJSON_Duplicate(const cJSON* item, cJSON_bool recurse)
{
    cJSON* newitem = NULL;
    cJSON* child = NULL;
    cJSON* next = NULL;
    cJSON* newchild = NULL;

    if (!item)
    {
        goto fail;
    }
    newitem = cJSON_New_Item(&global_hooks);
    if (!newitem)
    {
        goto fail;
    }
    newitem->type = item->type & (~cJSON_IsReference);
    newitem->valueint = item->valueint;
    newitem->valuedouble = item->valuedouble;
    if (item->valuestring)
    {
        newitem->valuestring = (char*)cJSON_strdup((unsigned char*)item->valuestring, &global_hooks);
        if (!newitem->valuestring)
        {
            goto fail;
        }
    }
    if (item->string)
    {
        newitem->string = (item->type & cJSON_StringIsConst) ? item->string : (char*)cJSON_strdup((unsigned char*)item->string, &global_hooks);
        if (!newitem->string)
        {
            goto fail;
        }
    }
    if (!recurse)
    {
        return newitem;
    }
    child = item->child;
    while (child != NULL)
    {
        newchild = cJSON_Duplicate(child, true);           
        if (!newchild)
        {
            goto fail;
        }
        if (next != NULL)
        {
            next->next = newchild;
            newchild->prev = next;
            next = newchild;
        }
        else
        {
            newitem->child = newchild;
            next = newchild;
        }
        child = child->next;
    }

    return newitem;

fail:
    if (newitem != NULL)
    {
        cJSON_Delete(newitem);
    }

    return NULL;
}

static void skip_oneline_comment(char** input)
{
    *input += static_strlen("//");

    for (; (*input)[0] != '\0'; ++(*input))
    {
        if ((*input)[0] == '\n') {
            *input += static_strlen("\n");
            return;
        }
    }
}

static void skip_multiline_comment(char** input)
{
    *input += static_strlen("/*");

    for (; (*input)[0] != '\0'; ++(*input))
    {
        if (((*input)[0] == '*') && ((*input)[1] == '/'))
        {
            *input += static_strlen("*/");
            return;
        }
    }
}

static void minify_string(char** input, char** output) {
    (*output)[0] = (*input)[0];
    *input += static_strlen("\"");
    *output += static_strlen("\"");


    for (; (*input)[0] != '\0'; (void)++(*input), ++(*output)) {
        (*output)[0] = (*input)[0];

        if ((*input)[0] == '\"') {
            (*output)[0] = '\"';
            *input += static_strlen("\"");
            *output += static_strlen("\"");
            return;
        }
        else if (((*input)[0] == '\\') && ((*input)[1] == '\"')) {
            (*output)[1] = (*input)[1];
            *input += static_strlen("\"");
            *output += static_strlen("\"");
        }
    }
}

CJSON_PUBLIC(void) cJSON_Minify(char* json)
{
    char* into = json;

    if (json == NULL)
    {
        return;
    }

    while (json[0] != '\0')
    {
        switch (json[0])
        {
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            json++;
            break;

        case '/':
            if (json[1] == '/')
            {
                skip_oneline_comment(&json);
            }
            else if (json[1] == '*')
            {
                skip_multiline_comment(&json);
            }
            else {
                json++;
            }
            break;

        case '\"':
            minify_string(&json, (char**)&into);
            break;

        default:
            into[0] = json[0];
            json++;
            into++;
        }
    }

    *into = '\0';
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsInvalid(const cJSON* const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_Invalid;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsFalse(const cJSON* const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_False;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsTrue(const cJSON* const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xff) == cJSON_True;
}


CJSON_PUBLIC(cJSON_bool) cJSON_IsBool(const cJSON* const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & (cJSON_True | cJSON_False)) != 0;
}
CJSON_PUBLIC(cJSON_bool) cJSON_IsNull(const cJSON* const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_NULL;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsNumber(const cJSON* const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_Number;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsString(const cJSON* const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_String;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsArray(const cJSON* const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_Array;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsObject(const cJSON* const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_Object;
}

CJSON_PUBLIC(cJSON_bool) cJSON_IsRaw(const cJSON* const item)
{
    if (item == NULL)
    {
        return false;
    }

    return (item->type & 0xFF) == cJSON_Raw;
}

CJSON_PUBLIC(cJSON_bool) cJSON_Compare(const cJSON* const a, const cJSON* const b, const cJSON_bool case_sensitive)
{
    if ((a == NULL) || (b == NULL) || ((a->type & 0xFF) != (b->type & 0xFF)) || cJSON_IsInvalid(a))
    {
        return false;
    }

    switch (a->type & 0xFF)
    {
    case cJSON_False:
    case cJSON_True:
    case cJSON_NULL:
    case cJSON_Number:
    case cJSON_String:
    case cJSON_Raw:
    case cJSON_Array:
    case cJSON_Object:
        break;

    default:
        return false;
    }

    if (a == b)
    {
        return true;
    }

    switch (a->type & 0xFF)
    {
    case cJSON_False:
    case cJSON_True:
    case cJSON_NULL:
        return true;

    case cJSON_Number:
        if (compare_double(a->valuedouble, b->valuedouble))
        {
            return true;
        }
        return false;

    case cJSON_String:
    case cJSON_Raw:
        if ((a->valuestring == NULL) || (b->valuestring == NULL))
        {
            return false;
        }
        if (strcmp(a->valuestring, b->valuestring) == 0)
        {
            return true;
        }

        return false;

    case cJSON_Array:
    {
        cJSON* a_element = a->child;
        cJSON* b_element = b->child;

        for (; (a_element != NULL) && (b_element != NULL);)
        {
            if (!cJSON_Compare(a_element, b_element, case_sensitive))
            {
                return false;
            }

            a_element = a_element->next;
            b_element = b_element->next;
        }

        if (a_element != b_element) {
            return false;
        }

        return true;
    }

    case cJSON_Object:
    {
        cJSON* a_element = NULL;
        cJSON* b_element = NULL;
        cJSON_ArrayForEach(a_element, a)
        {
            b_element = get_object_item(b, a_element->string, case_sensitive);
            if (b_element == NULL)
            {
                return false;
            }

            if (!cJSON_Compare(a_element, b_element, case_sensitive))
            {
                return false;
            }
        }

        cJSON_ArrayForEach(b_element, b)
        {
            a_element = get_object_item(a, b_element->string, case_sensitive);
            if (a_element == NULL)
            {
                return false;
            }

            if (!cJSON_Compare(b_element, a_element, case_sensitive))
            {
                return false;
            }
        }

        return true;
    }

    default:
        return false;
    }
}

CJSON_PUBLIC(void*) cJSON_malloc(size_t size)
{
    return global_hooks.allocate(size);
}

CJSON_PUBLIC(void) cJSON_free(void* object)
{
    global_hooks.deallocate(object);
}