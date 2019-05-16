/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Name:
 *	MOXA LED Control Utility
 *
 * Description:
 *	Utility for controling LED to off, on, or blink.
 *
 * Authors:
 *	2017	Harry YJ Jhou	<HarryYJ.Jhou@moxa.com>
 *	2018	Ken CJ Chou	<KenCJ.Chou@moxa.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <mx_led.h>

enum setting_type {
	UNSET = -1,
	TARGET_ALL = 0,
	TARGET_INDEX = 1
};

struct action_struct {
	int led_type;
	int group_setting;
	int group_id;
	int index_setting;
	int index;
	char *state;
};

void usage(FILE *fp)
{
	fprintf(fp, "Usage:\n");
	fprintf(fp, "	mx-led-ctl -s|-p <sgn_group|prog_group> -i <led_index> off|on|blink\n");
	fprintf(fp, "OPTIONS:\n");
	fprintf(fp, "	-s <led_group>\n");
	fprintf(fp, "		Select signal LED group\n");
	fprintf(fp, "	-p <led_group>\n");
	fprintf(fp, "		Select programmable LED group\n");
	fprintf(fp, "	-i <led_index>\n");
	fprintf(fp, "		Select LED index in group\n");
	fprintf(fp, "	--all-signal\n");
	fprintf(fp, "		Select all signal LEDs\n");
	fprintf(fp, "	--all-programmable\n");
	fprintf(fp, "		Select all programmable LEDs\n");
	fprintf(fp, "\n");
	fprintf(fp, "Example:\n");
	fprintf(fp, "	Turn on 'Signal' led GROUP 2 index 3\n");
	fprintf(fp, "	# mx-led-ctl -s 2 -i 3 on\n");
	fprintf(fp, "\n");
	fprintf(fp, "	Turn off 'Programmable' led GROUP 2 index 3\n");
	fprintf(fp, "	# mx-led-ctl -p 2 -i 3 off\n");
}

void do_action(struct action_struct action)
{
	if (action.index_setting == TARGET_INDEX) {
		if (strcmp(action.state, "off") == 0) {
			if (mx_led_set_brightness(action.led_type,
				action.group_id, action.index,
				LED_STATE_OFF) < 0) {
				fprintf(stderr, "Failed to set LED state\n");
				exit(1);
			}
		} else if (strcmp(action.state, "on") == 0) {
			if (mx_led_set_brightness(action.led_type,
				action.group_id, action.index,
				LED_STATE_ON) < 0) {
				fprintf(stderr, "Failed to set LED state\n");
				exit(1);
			}
		} else if (strcmp(action.state, "blink") == 0) {
			if (mx_led_set_brightness(action.led_type,
				action.group_id, action.index,
				LED_STATE_BLINK) < 0) {
				fprintf(stderr, "Failed to set LED state\n");
				exit(1);
			}
		}  else {
			fprintf(stderr, "unknown state: %s: ", action.state);
			usage(stderr);
			exit(99);
		}
	} else if (action.group_setting == TARGET_ALL) {
		if (strcmp(action.state, "off") == 0) {
			if (mx_led_set_type_all(action.led_type,
				LED_STATE_OFF) < 0) {
				fprintf(stderr, "Failed to set LED state\n");
				exit(1);
			}
		} else if (strcmp(action.state, "on") == 0) {
			if (mx_led_set_type_all(action.led_type,
				LED_STATE_ON) < 0) {
				fprintf(stderr, "Failed to set LED state\n");
				exit(1);
			}
		} else if (strcmp(action.state, "blink") == 0) {
			if (mx_led_set_type_all(action.led_type,
				LED_STATE_BLINK) < 0) {
				fprintf(stderr, "Failed to set LED state\n");
				exit(1);
			}
		}  else {
			fprintf(stderr, "unknown state: %s: ", action.state);
			usage(stderr);
			exit(99);
		}
	} else {
		int num_of_leds_per_group, i;

		if (mx_led_get_num_of_leds_per_group(action.led_type,
			&num_of_leds_per_group) < 0) {
			fprintf(stderr, "Failed to get number of leds\n");
			exit(1);
		}

		if (strlen(action.state) != num_of_leds_per_group) {
			fprintf(stderr, "number of leds not matched\n");
			exit(1);
		}

		for (i = 1; i <= num_of_leds_per_group; i++) {
			if (mx_led_set_brightness(action.led_type,
				action.group_id, i,
				action.state[i - 1] - '0') < 0) {
				fprintf(stderr, "Failed to set LED state\n");
				exit(1);
			}
		}
	}
}

int main(int argc, char *argv[])
{
	struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"signal", required_argument, 0, 's'},
		{"programmable", required_argument, 0, 'p'},
		{"index", required_argument, 0, 'i'},
		{"raw", required_argument, 0, 'r'},
		{"all-signal", required_argument, 0, 'S'},
		{"all-programmable", required_argument, 0, 'P'},
		{0, 0, 0, 0}
	};
	struct action_struct action = {
		.led_type = LED_TYPE_SIGNAL,
		.group_setting = UNSET,
		.index_setting = UNSET,
	};
	int c;

	while (1) {
		c = getopt_long(argc, argv, "hs:p:i:r:S:P:", long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case 'h':
			usage(stdout);
			exit(0);
		case 'p':
			action.led_type = LED_TYPE_PROGRAMMABLE;
		case 's':
			if (action.group_setting != UNSET) {
				fprintf(stderr, "group is already set\n");
				usage(stderr);
				exit(99);
			}
			action.group_setting = TARGET_INDEX;
			action.group_id = atoi(argv[optind - 1]);
			break;
		case 'i':
			if (action.index_setting != UNSET) {
				fprintf(stderr, "index is already set\n");
				usage(stderr);
				exit(99);
			}
			action.index_setting = TARGET_INDEX;
			action.index = atoi(argv[optind - 1]);
			action.state = argv[optind];
			optind++;
			break;
		case 'r':
			if (action.index_setting != UNSET) {
				fprintf(stderr, "index is already set\n");
				usage(stderr);
				exit(99);
			}
			action.index_setting = TARGET_ALL;
			action.state = argv[optind - 1];
			break;
		case 'P':
			action.led_type = LED_TYPE_PROGRAMMABLE;
		case 'S':
			if (action.group_setting != UNSET ||
				action.index_setting != UNSET) {
				fprintf(stderr, "group or index is already set\n");
				usage(stderr);
				exit(99);
			}
			action.group_setting = TARGET_ALL;
			action.index_setting = TARGET_ALL;
			action.state = argv[optind - 1];
			break;
		default:
			usage(stderr);
			exit(99);
		}
	}

	if (optind < argc) {
		fprintf(stderr, "non-option arguments found: ");
		while (optind < argc)
			fprintf(stderr, "%s ", argv[optind++]);
		fprintf(stderr, "\n");

		usage(stderr);
		exit(99);
	}

	if (action.group_setting == UNSET ||
		action.index_setting == UNSET) {
		fprintf(stderr, "group or index is not yet set\n");
		usage(stderr);
		exit(99);
	}

	if (mx_led_init() < 0) {
		fprintf(stderr, "Initialize Moxa led control library failed\n");
		exit(1);
	}

	do_action(action);

	exit(0);
}

