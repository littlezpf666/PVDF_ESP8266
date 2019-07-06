/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "mem.h"

#include "user_json.h"

LOCAL char *json_buf;
LOCAL int pos;
LOCAL int size;

/******************************************************************************
 * FunctionName : find_json_path
 * Description  : find the JSON format tree's path
 * Parameters   : json -- A pointer to a JSON set up
 *                path -- A pointer to the JSON format tree's path
 * Returns      : A pointer to the JSON format tree
*******************************************************************************/
struct jsontree_value *ICACHE_FLASH_ATTR
find_json_path(struct jsontree_context *json, const char *path)
{
    struct jsontree_value *v;
    const char *start;
    const char *end;
    int len;

    v = json->values[0];
    start = path;

    do {
        end = (const char *)os_strstr(start, "/");

        if (end == start) {
            break;
        }

        if (end != NULL) {
            len = end - start;
            end++;
        } else {
            len = os_strlen(start);
        }

        if (v->type != JSON_TYPE_OBJECT) {
            v = NULL;
        } else {
            struct jsontree_object *o;
            int i;

            o = (struct jsontree_object *)v;
            v = NULL;

            for (i = 0; i < o->count; i++) {
                if (os_strncmp(start, o->pairs[i].name, len) == 0) {
                    v = o->pairs[i].value;
                    json->index[json->depth] = i;
                    json->depth++;
                    json->values[json->depth] = v;
                    json->index[json->depth] = 0;
                    break;
                }
            }
        }

        start = end;
    } while (end != NULL && *end != '\0' && v != NULL);

    json->callback_state = 0;
    return v;
}

/******************************************************************************
 * FunctionName : json_putchar
 * Description  : write the value to the JSON  format tree
 * Parameters   : c -- the value which write the JSON format tree
 * Returns      : result
*******************************************************************************/
int ICACHE_FLASH_ATTR
json_putchar(int c)
{
    if (json_buf != NULL && pos <= size) {
        json_buf[pos++] = c;
        return c;
    }

    return 0;
}

/******************************************************************************
 * FunctionName : json_ws_send
 * Description  : set up the JSON format tree for string
 * Parameters   : tree -- A pointer to the JSON format tree
 *                path -- A pointer to the JSON format tree's path
 *                pbuf -- A pointer for the data sent
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
json_ws_send(struct jsontree_value *tree, const char *path, char *pbuf)
{
    struct jsontree_context json;
    /* maxsize = 128 bytes */
    json_buf = (char *)os_malloc(jsonSize);

    /* reset state and set max-size */
    /* NOTE: packet will be truncated at 512 bytes */
    pos = 0;
    size = jsonSize;

    json.values[0] = (struct jsontree_value *)tree;
    jsontree_reset(&json);
    find_json_path(&json, path);
    json.path = json.depth;
    json.putchar = json_putchar;

    while (jsontree_print_next(&json) && json.path <= json.depth);

    json_buf[pos] = 0;
    os_memcpy(pbuf, json_buf, pos);
    os_free(json_buf);
}

/******************************************************************************
 * FunctionName : json_parse
 * Description  : parse the data as a JSON format
 * Parameters   : js_ctx -- A pointer to a JSON set up
 *                ptrJSONMessage -- A pointer to the data
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
json_parse(struct jsontree_context *json, char *ptrJSONMessage)
{
    /* Set value */
    struct jsontree_value *v;
    struct jsontree_callback *c;
    struct jsontree_callback *c_bak = NULL;

    while ((v = jsontree_find_next(json, JSON_TYPE_CALLBACK)) != NULL) {
        c = (struct jsontree_callback *)v;

        if (c == c_bak) {
            continue;
        }

        c_bak = c;

        if (c->set != NULL) {
            struct jsonparse_state js;

            jsonparse_setup(&js, ptrJSONMessage, os_strlen(ptrJSONMessage));
            c->set(json, &js);
        }
    }
}
/*****************************自编字符转数字函数*******************************/
int ICACHE_FLASH_ATTR value_as_int(char *key_value,char length)
{
	uint8 i,j;
	uint16_t order=1;
	uint16_t key_integer=0;
	for(i=0;i<length;i++)
	{
		/*将字符转化为整形*/
		for(j=0;j<(length-i-1);j++)//根据位数求出每位应当乘10的几次幂
		{
			order=order*10;
		}
		key_integer+=((*(key_value+i)-48)*order) ;
		order=1;
	}

	return key_integer;
}
/*****************************自编解析单层键值函数*******************************/
uint16_t ICACHE_FLASH_ATTR comtype_parse(char* pdata,char* key)
{
	uint8 length,i;
	uint16_t key_integer=0;

	char *parse_begin,*parse_end;
	/*不要再此定义数组因为除出了函数内存就释放掉了，也可定义字符串但字符串是常量，不会被释放掉，
	 而且定义空间大小很麻烦，因此选用动态产生释放内存的方法*/
	char *key_value = NULL;
	key_value = (char *) os_zalloc(5);
	parse_begin=(char *)os_strstr(pdata,key);

	if (parse_begin != NULL){
		parse_begin+=(strlen(key)+2);
		parse_end=os_strchr(parse_begin,',');
		length=parse_end-parse_begin;

		//os_printf("length:%d\r\n",length);

		os_printf("length:%d\r\n",length);

		for(i=0;i<length;i++)
		{
		key_value[i]=*(parse_begin+i);
		}
		key_integer=value_as_int(key_value,length);
		os_free(key_value);
		os_printf("comm_type:%d\r\n",key_integer);
	}
	return key_integer;
}
