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

#ifndef CJSON_NESTING_LIMIT
#define CJSON_NESTING_LIMIT 1000
#endif

    CJSON_PUBLIC(cJSON*) cJSON_Parse(const char* value);
    CJSON_PUBLIC(cJSON*) cJSON_ParseWithLength(const char* value, size_t buffer_length);
    CJSON_PUBLIC(cJSON*) cJSON_ParseWithOpts(const char* value, const char** return_parse_end, bool require_null_terminated);
    CJSON_PUBLIC(cJSON*) cJSON_ParseWithLengthOpts(const char* value, size_t buffer_length, const char** return_parse_end, bool require_null_terminated);

    CJSON_PUBLIC(char*) cJSON_Print(const cJSON* item);
    CJSON_PUBLIC(char*) cJSON_PrintUnformatted(const cJSON* item);
    CJSON_PUBLIC(char*) cJSON_PrintBuffered(const cJSON* item, int prebuffer, bool fmt);
    CJSON_PUBLIC(bool) cJSON_PrintPreallocated(cJSON* item, char* buffer, const int length, const bool format);
    CJSON_PUBLIC(void) cJSON_Delete(cJSON* item);

    CJSON_PUBLIC(int) cJSON_GetArraySize(const cJSON* array);
    CJSON_PUBLIC(cJSON*) cJSON_GetArrayItem(const cJSON* array, int index);
    CJSON_PUBLIC(cJSON*) cJSON_GetObjectItem(const cJSON* const object, const char* const string);
    CJSON_PUBLIC(cJSON*) cJSON_GetObjectItemCaseSensitive(const cJSON* const object, const char* const string);
    CJSON_PUBLIC(bool) cJSON_HasObjectItem(const cJSON* object, const char* string);
    CJSON_PUBLIC(const char*) cJSON_GetErrorPtr(void);

    CJSON_PUBLIC(cJSON*) cJSON_CreateNull(void);
    CJSON_PUBLIC(cJSON*) cJSON_CreateBool(bool boolean);
    CJSON_PUBLIC(cJSON*) cJSON_CreateNumber(double num);
    CJSON_PUBLIC(cJSON*) cJSON_CreateString(const char* string);
    CJSON_PUBLIC(cJSON*) cJSON_CreateRaw(const char* raw);
    CJSON_PUBLIC(cJSON*) cJSON_CreateArray(void);
    CJSON_PUBLIC(cJSON*) cJSON_CreateObject(void);

    CJSON_PUBLIC(bool) cJSON_AddItemToArray(cJSON* array, cJSON* item);
    CJSON_PUBLIC(bool) cJSON_AddItemToObject(cJSON* object, const char* string, cJSON* item);
    CJSON_PUBLIC(bool) cJSON_AddItemToObjectCS(cJSON* object, const char* string, cJSON* item);
    CJSON_PUBLIC(bool) cJSON_AddItemReferenceToArray(cJSON* array, cJSON* item);
    CJSON_PUBLIC(bool) cJSON_AddItemReferenceToObject(cJSON* object, const char* string, cJSON* item);

    CJSON_PUBLIC(cJSON*) cJSON_AddNullToObject(cJSON* const object, const char* const name);
    CJSON_PUBLIC(cJSON*) cJSON_AddBoolToObject(cJSON* const object, const char* const name, const bool boolean);
    CJSON_PUBLIC(cJSON*) cJSON_AddNumberToObject(cJSON* const object, const char* const name, const double number);
    CJSON_PUBLIC(cJSON*) cJSON_AddStringToObject(cJSON* const object, const char* const name, const char* const string);
    CJSON_PUBLIC(cJSON*) cJSON_AddRawToObject(cJSON* const object, const char* const name, const char* const raw);
    CJSON_PUBLIC(cJSON*) cJSON_AddObjectToObject(cJSON* const object, const char* const name);
    CJSON_PUBLIC(cJSON*) cJSON_AddArrayToObject(cJSON* const object, const char* const name);

#define cJSON_ArrayForEach(element, array) for(element = (array != NULL) ? (array)->child : NULL; element != NULL; element = element->next)

#ifdef __cplusplus
}
#endif

#endif

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

#define static_strlen(string_literal) (sizeof(string_literal) - sizeof(""))

static unsigned char* cJSON_strdup(const unsigned char* string)
{
    size_t length = 0;
    unsigned char* copy = NULL;

    if (string == NULL)
    {
        return NULL;
    }

    length = strlen((const char*)string) + sizeof("");
    copy = (unsigned char*)malloc(length);
    if (copy == NULL)
    {
        return NULL;
    }
    memcpy(copy, string, length);

    return copy;
}

static cJSON* cJSON_New_Item()
{
    cJSON* node = (cJSON*)malloc(sizeof(cJSON));
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
            free(item->valuestring);
        }
        if (!(item->type & cJSON_StringIsConst) && (item->string != NULL))
        {
            free(item->string);
        }
        free(item);
        item = next;
    }
}

typedef struct
{
    const unsigned char* content;
    size_t length;
    size_t offset;
    size_t depth;              
} parse_buffer;

#define can_read(buffer, size) ((buffer != NULL) && (((buffer)->offset + size) <= (buffer)->length))
#define can_access_at_index(buffer, index) ((buffer != NULL) && (((buffer)->offset + index) < (buffer)->length))
#define cannot_access_at_index(buffer, index) (!can_access_at_index(buffer, index))
#define buffer_at_offset(buffer) ((buffer)->content + (buffer)->offset)

static bool parse_number(cJSON* const item, parse_buffer* const input_buffer)
{
    double number = 0;
    unsigned char* after_end = NULL;
    unsigned char number_c_string[64];
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
            number_c_string[i] = '.';
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
    copy = (char*)cJSON_strdup((const unsigned char*)valuestring);
    if (copy == NULL)
    {
        return NULL;
    }
    if (object->valuestring != NULL)
    {
        free(object->valuestring);
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
    bool noalloc;
    bool format;        
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

    newbuffer = (unsigned char*)realloc(p->buffer, newsize);
    if (newbuffer == NULL)
    {
        free(p->buffer);
        p->length = 0;
        p->buffer = NULL;

        return NULL;
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

static bool compare_double(double a, double b)
{
    double maxVal = fabs(a) > fabs(b) ? fabs(a) : fabs(b);
    return (fabs(a - b) <= maxVal * DBL_EPSILON);
}

static bool print_number(const cJSON* const item, printbuffer* const output_buffer)
{
    unsigned char* output_pointer = NULL;
    double d = item->valuedouble;
    int length = 0;
    size_t i = 0;
    unsigned char number_buffer[26] = { 0 };         
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
        if (number_buffer[i] == '.')
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

static bool parse_string(cJSON* const item, parse_buffer* const input_buffer)
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
        output = (unsigned char*)malloc(allocation_length + sizeof(""));
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
        free(output);
    }

    if (input_pointer != NULL)
    {
        input_buffer->offset = (size_t)(input_pointer - input_buffer->content);
    }

    return false;
}

static bool print_string_ptr(const unsigned char* const input, printbuffer* const output_buffer)
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

static bool print_string(const cJSON* const item, printbuffer* const p)
{
    return print_string_ptr((unsigned char*)item->valuestring, p);
}

static bool parse_value(cJSON* const item, parse_buffer* const input_buffer);
static bool print_value(const cJSON* const item, printbuffer* const output_buffer);
static bool parse_array(cJSON* const item, parse_buffer* const input_buffer);
static bool print_array(const cJSON* const item, printbuffer* const output_buffer);
static bool parse_object(cJSON* const item, parse_buffer* const input_buffer);
static bool print_object(const cJSON* const item, printbuffer* const output_buffer);

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

CJSON_PUBLIC(cJSON*) cJSON_ParseWithOpts(const char* value, const char** return_parse_end, bool require_null_terminated)
{
    size_t buffer_length;

    if (NULL == value)
    {
        return NULL;
    }

    buffer_length = strlen(value) + sizeof("");

    return cJSON_ParseWithLengthOpts(value, buffer_length, return_parse_end, require_null_terminated);
}

CJSON_PUBLIC(cJSON*) cJSON_ParseWithLengthOpts(const char* value, size_t buffer_length, const char** return_parse_end, bool require_null_terminated)
{
    parse_buffer buffer = { 0, 0, 0, 0 };
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

    item = cJSON_New_Item();
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

static unsigned char* print(const cJSON* const item, bool format)
{
    static const size_t default_buffer_size = 256;
    printbuffer buffer[1];
    unsigned char* printed = NULL;

    memset(buffer, 0, sizeof(buffer));

    buffer->buffer = (unsigned char*)malloc(default_buffer_size);
    buffer->length = default_buffer_size;
    buffer->format = format;
    if (buffer->buffer == NULL)
    {
        goto fail;
    }

    if (!print_value(item, buffer))
    {
        goto fail;
    }
    update_offset(buffer);

    printed = (unsigned char*)realloc(buffer->buffer, buffer->offset + 1);
    if (printed == NULL) {
        goto fail;
    }
    buffer->buffer = NULL;

    return printed;

fail:
    if (buffer->buffer != NULL)
    {
        free(buffer->buffer);
    }

    if (printed != NULL)
    {
        free(printed);
    }

    return NULL;
}

CJSON_PUBLIC(char*) cJSON_Print(const cJSON* item)
{
    return (char*)print(item, true);
}

CJSON_PUBLIC(char*) cJSON_PrintUnformatted(const cJSON* item)
{
    return (char*)print(item, false);
}

CJSON_PUBLIC(char*) cJSON_PrintBuffered(const cJSON* item, int prebuffer, bool fmt)
{
    printbuffer p = { 0, 0, 0, 0, 0, 0 };

    if (prebuffer < 0)
    {
        return NULL;
    }

    p.buffer = (unsigned char*)malloc((size_t)prebuffer);
    if (!p.buffer)
    {
        return NULL;
    }

    p.length = (size_t)prebuffer;
    p.offset = 0;
    p.noalloc = false;
    p.format = fmt;

    if (!print_value(item, &p))
    {
        free(p.buffer);
        return NULL;
    }

    return (char*)p.buffer;
}

CJSON_PUBLIC(bool) cJSON_PrintPreallocated(cJSON* item, char* buffer, const int length, const bool format)
{
    printbuffer p = { 0, 0, 0, 0, 0, 0 };

    if ((length < 0) || (buffer == NULL))
    {
        return false;
    }

    p.buffer = (unsigned char*)buffer;
    p.length = (size_t)length;
    p.offset = 0;
    p.noalloc = true;
    p.format = format;

    return print_value(item, &p);
}

static bool parse_value(cJSON* const item, parse_buffer* const input_buffer)
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

static bool print_value(const cJSON* const item, printbuffer* const output_buffer)
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

static bool parse_array(cJSON* const item, parse_buffer* const input_buffer)
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
        cJSON* new_item = cJSON_New_Item();
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

static bool print_array(const cJSON* const item, printbuffer* const output_buffer)
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

static bool parse_object(cJSON* const item, parse_buffer* const input_buffer)
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
        cJSON* new_item = cJSON_New_Item();
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

static bool print_object(const cJSON* const item, printbuffer* const output_buffer)
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

static cJSON* get_object_item(const cJSON* const object, const char* const name, const bool case_sensitive)
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

CJSON_PUBLIC(bool) cJSON_HasObjectItem(const cJSON* object, const char* string)
{
    return cJSON_GetObjectItem(object, string) ? 1 : 0;
}

static void suffix_object(cJSON* prev, cJSON* item)
{
    prev->next = item;
    item->prev = prev;
}

static cJSON* create_reference(const cJSON* item)
{
    cJSON* reference = NULL;
    if (item == NULL)
    {
        return NULL;
    }

    reference = cJSON_New_Item();
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

static bool add_item_to_array(cJSON* array, cJSON* item)
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

CJSON_PUBLIC(bool) cJSON_AddItemToArray(cJSON* array, cJSON* item)
{
    return add_item_to_array(array, item);
}

bool add_item_to_object(cJSON* const object, const char* const string, cJSON* const item, const bool constant_key)
{
    char* new_key = NULL;
    int new_type = cJSON_Invalid;

    if ((object == NULL) || (string == NULL) || (item == NULL) || (object == item))
    {
        return false;
    }

    if (constant_key)
    {
        new_key = (char*)string;
        new_type = item->type | cJSON_StringIsConst;
    }
    else
    {
        new_key = (char*)cJSON_strdup((const unsigned char*)string);
        if (new_key == NULL)
        {
            return false;
        }

        new_type = item->type & ~cJSON_StringIsConst;
    }

    if (!(item->type & cJSON_StringIsConst) && (item->string != NULL))
    {
        free(item->string);
    }

    item->string = new_key;
    item->type = new_type;

    return add_item_to_array(object, item);
}

CJSON_PUBLIC(bool) cJSON_AddItemToObject(cJSON* object, const char* string, cJSON* item)
{
    return add_item_to_object(object, string, item, false);
}

CJSON_PUBLIC(bool) cJSON_AddItemToObjectCS(cJSON* object, const char* string, cJSON* item)
{
    return add_item_to_object(object, string, item, true);
}

CJSON_PUBLIC(bool) cJSON_AddItemReferenceToArray(cJSON* array, cJSON* item)
{
    if (array == NULL)
    {
        return false;
    }

    return add_item_to_array(array, create_reference(item));
}

CJSON_PUBLIC(bool) cJSON_AddItemReferenceToObject(cJSON* object, const char* string, cJSON* item)
{
    if ((object == NULL) || (string == NULL))
    {
        return false;
    }

    return add_item_to_object(object, string, create_reference(item), false);
}

CJSON_PUBLIC(cJSON*) cJSON_AddNullToObject(cJSON* const object, const char* const name)
{
    cJSON* null = cJSON_CreateNull();
    if (add_item_to_object(object, name, null, false))
    {
        return null;
    }

    cJSON_Delete(null);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddBoolToObject(cJSON* const object, const char* const name, const bool boolean)
{
    cJSON* bool_item = cJSON_CreateBool(boolean);
    if (add_item_to_object(object, name, bool_item, false))
    {
        return bool_item;
    }

    cJSON_Delete(bool_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddNumberToObject(cJSON* const object, const char* const name, const double number)
{
    cJSON* number_item = cJSON_CreateNumber(number);
    if (add_item_to_object(object, name, number_item, false))
    {
        return number_item;
    }

    cJSON_Delete(number_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddStringToObject(cJSON* const object, const char* const name, const char* const string)
{
    cJSON* string_item = cJSON_CreateString(string);
    if (add_item_to_object(object, name, string_item, false))
    {
        return string_item;
    }

    cJSON_Delete(string_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddRawToObject(cJSON* const object, const char* const name, const char* const raw)
{
    cJSON* raw_item = cJSON_CreateRaw(raw);
    if (add_item_to_object(object, name, raw_item, false))
    {
        return raw_item;
    }

    cJSON_Delete(raw_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddObjectToObject(cJSON* const object, const char* const name)
{
    cJSON* object_item = cJSON_CreateObject();
    if (add_item_to_object(object, name, object_item, false))
    {
        return object_item;
    }

    cJSON_Delete(object_item);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_AddArrayToObject(cJSON* const object, const char* const name)
{
    cJSON* array = cJSON_CreateArray();
    if (add_item_to_object(object, name, array, false))
    {
        return array;
    }

    cJSON_Delete(array);
    return NULL;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateNull(void)
{
    cJSON* item = cJSON_New_Item();
    if (item)
    {
        item->type = cJSON_NULL;
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateBool(bool boolean)
{
    cJSON* item = cJSON_New_Item();
    if (item)
    {
        item->type = boolean ? cJSON_True : cJSON_False;
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateNumber(double num)
{
    cJSON* item = cJSON_New_Item();
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
    cJSON* item = cJSON_New_Item();
    if (item)
    {
        item->type = cJSON_String;
        item->valuestring = (char*)cJSON_strdup((const unsigned char*)string);
        if (!item->valuestring)
        {
            cJSON_Delete(item);
            return NULL;
        }
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateRaw(const char* raw)
{
    cJSON* item = cJSON_New_Item();
    if (item)
    {
        item->type = cJSON_Raw;
        item->valuestring = (char*)cJSON_strdup((const unsigned char*)raw);
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
    cJSON* item = cJSON_New_Item();
    if (item)
    {
        item->type = cJSON_Array;
    }

    return item;
}

CJSON_PUBLIC(cJSON*) cJSON_CreateObject(void)
{
    cJSON* item = cJSON_New_Item();
    if (item)
    {
        item->type = cJSON_Object;
    }

    return item;
}