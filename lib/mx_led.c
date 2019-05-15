/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Name:
 *	MOXA LED Library
 *
 * Description:
 *	Library for controling LED to off, on, or blink.
 *
 * Authors:
 *	2014	SZ Lin		<sz.lin@moxa.com>
 *	2017	Harry YJ Jhou	<HarryYJ.Jhou@moxa.com>
 *	2018	Ken CJ Chou	<KenCJ.Chou@moxa.com>
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <json-c/json.h>
#include <mx_led.h>

#define CONF_FILE "/etc/moxa-configs/moxa-led-control.json"
#define CONF_VER_SUPPORTED "1.1.*"

#define LED_BASEPATH "/sys/class/leds"
#define MAX_FILEPATH_LEN 256	/* reserved length for file path */

struct led_state_struct {
	const char *file_written;
	const char *data_written;
};

static int lib_initialized;
static struct json_object *config;
static struct led_state_struct led_states[3] = {
	{
		.file_written = "brightness",
		.data_written = "0"
	},
	{
		.file_written = "brightness",
		.data_written = "1"
	},
	{
		.file_written = "trigger",
		.data_written = "heartbeat"
	}
};
extern char mx_errmsg[256];

/*
 * json-c utilities
 */

static inline int obj_get_obj(struct json_object *obj, char *key, struct json_object **val)
{
	if (!json_object_object_get_ex(obj, key, val)) {
		sprintf(mx_errmsg, "json-c: can\'t get key: \"%s\"", key);
		return -1;
	}
	return 0;
}

static int obj_get_int(struct json_object *obj, char *key, int *val)
{
	struct json_object *tmp;

	if (obj_get_obj(obj, key, &tmp) < 0)
		return -1;

	*val = json_object_get_int(tmp);
	return 0;
}

static int obj_get_str(struct json_object *obj, char *key, const char **val)
{
	struct json_object *tmp;

	if (obj_get_obj(obj, key, &tmp) < 0)
		return -1;

	*val = json_object_get_string(tmp);
	return 0;
}

static int obj_get_arr(struct json_object *obj, char *key, struct array_list **val)
{
	struct json_object *tmp;

	if (obj_get_obj(obj, key, &tmp) < 0)
		return -1;

	*val = json_object_get_array(tmp);
	return 0;
}

static int arr_get_obj(struct array_list *arr, int idx, struct json_object **val)
{
	if (arr == NULL || idx >= arr->length) {
		sprintf(mx_errmsg, "json-c: can\'t get index: %d", idx);
		return -1;
	}

	*val = array_list_get_idx(arr, idx);
	return 0;
}

static int arr_get_int(struct array_list *arr, int idx, int *val)
{
	struct json_object *tmp;

	if (arr_get_obj(arr, idx, &tmp) < 0)
		return -1;

	*val = json_object_get_int(tmp);
	return 0;
}

static int arr_get_str(struct array_list *arr, int idx, const char **val)
{
	struct json_object *tmp;

	if (arr_get_obj(arr, idx, &tmp) < 0)
		return -1;

	*val = json_object_get_string(tmp);
	return 0;
}

static int arr_get_arr(struct array_list *arr, int idx, struct array_list **val)
{
	struct json_object *tmp;

	if (arr_get_obj(arr, idx, &tmp) < 0)
		return -1;

	*val = json_object_get_array(tmp);
	return 0;
}

/*
 * static functions
 */

static int check_config_version_supported(const char *conf_ver)
{
	int cv[2], sv[2];

	if (sscanf(conf_ver, "%d.%d.%*s", &cv[0], &cv[1]) < 0) {
		sprintf(mx_errmsg, "sscanf: %s: %s", conf_ver, strerror(errno));
		return -1; /* E_SYSFUNCERR */
	}

	if (sscanf(CONF_VER_SUPPORTED, "%d.%d.%*s", &sv[0], &sv[1]) < 0) {
		sprintf(mx_errmsg, "sscanf: %s: %s", CONF_VER_SUPPORTED, strerror(errno));
		return -1; /* E_SYSFUNCERR */
	}

	if (cv[0] != sv[0] || cv[1] != sv[1]) {
		sprintf(mx_errmsg, "Config version not supported, need to be %s", CONF_VER_SUPPORTED);
		return -4; /* E_UNSUPCONFVER */
	}
	return 0;
}

static int get_led_type_info(int led_type, struct json_object **led_type_info)
{
	struct array_list *led_types;

	if (led_type != LED_TYPE_SIGNAL && led_type != LED_TYPE_PROGRAMMABLE) {
		sprintf(mx_errmsg, "Invalid type: %d", led_type);
		return -2; /* E_INVAL */
	}

	if (obj_get_arr(config, "LED_TYPES", &led_types) < 0)
		return -5; /* E_CONFERR */

	if (arr_get_obj(led_types, led_type, led_type_info) < 0)
		return -5; /* E_CONFERR */

	return 0;
}

static int get_led_state_info(int led_state, struct led_state_struct **state)
{
	if (led_state != LED_STATE_OFF && led_state != LED_STATE_ON &&
		led_state != LED_STATE_BLINK){
		sprintf(mx_errmsg, "Invalid state: %d", led_state);
		return -2; /* E_INVAL */
	}

	*state = &led_states[led_state];
	return 0;
}

static int get_led_path(int led_type, int group, int index, const char **path)
{
	struct json_object *led_type_info;
	struct array_list *group_paths, *led_paths;
	int ret, num_of_groups, num_of_leds_per_group;

	ret = get_led_type_info(led_type, &led_type_info);
	if (ret < 0)
		return ret;

	if (obj_get_int(led_type_info, "NUM_OF_GROUPS", &num_of_groups) < 0)
		return -5; /* E_CONFERR */

	if (obj_get_int(led_type_info, "NUM_OF_LEDS_PER_GROUP", &num_of_leds_per_group) < 0)
		return -5; /* E_CONFERR */

	if (group < 1 || group > num_of_groups) {
		sprintf(mx_errmsg, "LED group out of index: %d", group);
		return -2; /* E_INVAL */
	}

	if (index < 1 || index > num_of_leds_per_group) {
		sprintf(mx_errmsg, "LED index out of index: %d", index);
		return -2; /* E_INVAL */
	}

	if (obj_get_arr(led_type_info, "PATHS", &group_paths) < 0)
		return -5; /* E_CONFERR */

	if (arr_get_arr(group_paths, group - 1, &led_paths) < 0)
		return -5; /* E_CONFERR */

	if (arr_get_str(led_paths, index - 1, path) < 0)
		return -5; /* E_CONFERR */

	return 0;
}

static int write_file(char *filepath, const char *data)
{
	int fd;

	fd = open(filepath, O_WRONLY);
	if (fd < 0) {
		sprintf(mx_errmsg, "open %s: %s", filepath, strerror(errno));
		return -1; /* E_SYSFUNCERR */
	}

	if (write(fd, data, strlen(data)) < 0) {
		sprintf(mx_errmsg, "write %s: %s", filepath, strerror(errno));
		close(fd);
		return -1; /* E_SYSFUNCERR */
	}
	close(fd);

	return 0;
}

static int set_led(int led_type, int group, int index, int led_state)
{
	struct led_state_struct *led_state_info;
	const char *led_path;
	char filepath[MAX_FILEPATH_LEN];
	int ret;

	/* set led off first before set led on */
	if (led_state == LED_STATE_ON) {
		ret = set_led(led_type, group, index, LED_STATE_OFF);
		if (ret < 0)
			return ret;
	}

	ret = get_led_path(led_type, group, index, &led_path);
	if (ret < 0)
		return ret;

	ret = get_led_state_info(led_state, &led_state_info);
	if (ret < 0)
		return ret;

	sprintf(filepath, "%s/%s/%s", LED_BASEPATH, led_path, led_state_info->file_written);

	return write_file(filepath, led_state_info->data_written);
}

/*
 * APIs
 */

int mx_led_init(void)
{
	int ret;
	const char *conf_ver;

	if (lib_initialized)
		return 0;

	config = json_object_from_file(CONF_FILE);
	if (config == NULL) {
		sprintf(mx_errmsg, "json-c: load file %s failed", CONF_FILE);
		return -5; /* E_CONFERR */
	}

	if (obj_get_str(config, "CONFIG_VERSION", &conf_ver) < 0)
		return -5; /* E_CONFERR */

	ret = check_config_version_supported(conf_ver);
	if (ret < 0)
		return ret;

	lib_initialized = 1;
	return 0;
}

int mx_led_get_num_of_groups(int led_type, int *num_of_groups)
{
	struct json_object *led_type_info;
	int ret;

	if (!lib_initialized) {
		sprintf(mx_errmsg, "Library is not initialized");
		return -3; /* E_LIBNOTINIT */
	}

	ret = get_led_type_info(led_type, &led_type_info);
	if (ret < 0)
		return ret;

	if (obj_get_int(led_type_info, "NUM_OF_GROUPS", num_of_groups) < 0)
		return -5; /* E_CONFERR */

	return 0;
}

int mx_led_get_num_of_leds_per_group(int led_type, int *num_of_leds_per_group)
{
	struct json_object *led_type_info;
	int ret;

	if (!lib_initialized) {
		sprintf(mx_errmsg, "Library is not initialized");
		return -3; /* E_LIBNOTINIT */
	}

	ret = get_led_type_info(led_type, &led_type_info);
	if (ret < 0)
		return ret;

	if (obj_get_int(led_type_info, "NUM_OF_LEDS_PER_GROUP", num_of_leds_per_group) < 0)
		return -5; /* E_CONFERR */

	return 0;
}

int mx_led_set_brightness(int led_type, int group, int index, int state)
{
	if (!lib_initialized) {
		sprintf(mx_errmsg, "Library is not initialized");
		return -3; /* E_LIBNOTINIT */
	}

	return set_led(led_type, group, index, state);
}

int mx_led_set_type_all(int led_type, int led_state)
{
	struct json_object *led_type_info;
	int ret, num_of_groups, num_of_leds_per_group, i, j;

	if (!lib_initialized) {
		sprintf(mx_errmsg, "Library is not initialized");
		return -3; /* E_LIBNOTINIT */
	}

	ret = get_led_type_info(led_type, &led_type_info);
	if (ret < 0)
		return ret;

	if (obj_get_int(led_type_info, "NUM_OF_GROUPS", &num_of_groups) < 0)
		return -5; /* E_CONFERR */

	if (obj_get_int(led_type_info, "NUM_OF_LEDS_PER_GROUP", &num_of_leds_per_group) < 0)
		return -5; /* E_CONFERR */

	for (i = 1; i <= num_of_groups; i++) {
		for (j = 1; j <= num_of_leds_per_group; j++) {
			ret = set_led(led_type, i, j, led_state);
			if (ret < 0)
				return ret;
		}
	}

	return 0;
}

int mx_led_set_all_off(void)
{
	int ret;

	if (!lib_initialized) {
		sprintf(mx_errmsg, "Library is not initialized");
		return -3; /* E_LIBNOTINIT */
	}

	ret = mx_led_set_type_all(LED_TYPE_SIGNAL, LED_STATE_OFF);
	if (ret < 0)
		return ret;

	ret = mx_led_set_type_all(LED_TYPE_PROGRAMMABLE, LED_STATE_OFF);
	if (ret < 0)
		return ret;

	return 0;
}

int mx_led_set_all_on(void)
{
	int ret;

	if (!lib_initialized) {
		sprintf(mx_errmsg, "Library is not initialized");
		return -3; /* E_LIBNOTINIT */
	}

	ret = mx_led_set_type_all(LED_TYPE_SIGNAL, LED_STATE_ON);
	if (ret < 0)
		return ret;

	ret = mx_led_set_type_all(LED_TYPE_PROGRAMMABLE, LED_STATE_ON);
	if (ret < 0)
		return ret;

	return 0;
}
