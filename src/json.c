#include "json.h"


/* Create a JSON response. */
struct json_object *create_response(int status, char *type, json_object *arguments) {
	struct json_object *object, *tmp;

	/* Create a new, empty JSON object. */
	object = json_object_new_object();

	/* Create the status integer. */
	tmp = json_object_new_int(status);
	json_object_object_add(object, "status", tmp);

	/* Create the type string. */
	tmp = json_object_new_string(type);
	json_object_object_add(object, "type", tmp);

	if (arguments != NULL)
		json_object_object_add(object, "arguments", arguments);

	return object;
}

/* Create JSON response with just a status code and a type. */
struct json_object *create_generic_response(int status, char *type) {
	return create_response(status, type, NULL);
}

/* Create a string argument pair. */
struct json_object *create_string_argument_pair(char *key, char *value) {
	struct json_object *arguments = json_object_new_object();
	struct json_object *object = json_object_new_string(value);

	json_object_object_add(arguments, key, object);

	return arguments;
}

/* Create a boolean argument pair. */
struct json_object *create_bool_argument_pair(char *key, int value) {
	struct json_object *arguments = json_object_new_object();
	struct json_object *object = json_object_new_boolean(value);

	json_object_object_add(arguments, key, object);

	return arguments;
}

/* Get an argument from a client response. */
struct json_object *get_argument(struct json_object *object, char *key) {
	struct json_object *temp, *temp2;

	json_object_object_get_ex(object, "arguments", &temp);
	json_object_object_get_ex(temp, key, &temp2);

	return temp2;
}

char *get_type(struct json_object *object) {
	struct json_object *type_object;
	char *type_string;

	json_object_object_get_ex(object, "type", &type_object);

	/* If the type is not valid, return. */
	if (!is_valid_json(object))
		return NULL;

	/* If the type is not a string, return. */
	if (!json_object_is_type(type_object, json_type_string))
		return NULL;

	type_string = (char *) json_object_get_string(type_object);
	return type_string;
}

int is_type(struct json_object *object, char *type) {
	char *type_string = get_type(object);

	/* If the type string matches and is not NULL, return true. */
	if (type_string != NULL && strcmp(type, type_string) == 0)
		return 1;

	return 0;
}

int is_valid_json(struct json_object *object) {
	return !json_object_is_type(object, json_type_null);
}

/* Convert a JSON object into a string. */
char *convert_json_object_to_string(json_object *object) {
	return (char *) json_object_to_json_string_ext(
			object,
			JSON_C_TO_STRING_PLAIN
	);
}

/* Convert a string into a JSON object. */
json_object *convert_string_to_json_object(char *data) {
	json_object *parsed_data = json_tokener_parse(data);
	return parsed_data;
}
