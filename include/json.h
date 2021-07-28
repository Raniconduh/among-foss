#pragma once

#include <json-c/json.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


struct json_object *create_response(int status, char *type, json_object *arguments);

struct json_object *create_generic_response(int status, char *type);

struct json_object *create_string_argument_pair(char *key, char *value);

struct json_object *create_bool_argument_pair(char *key, int value);

struct json_object *get_argument(struct json_object *object, char *key);

char *get_type(struct json_object *object);

int is_type(struct json_object *object, char *type);

int is_valid_json(struct json_object *object);

char *convert_json_object_to_string(json_object *object);

struct json_object *convert_string_to_json_object(char *data);
