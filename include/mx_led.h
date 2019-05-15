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

#ifndef _MOXA_LED_H
#define _MOXA_LED_H

enum led_type {
	LED_TYPE_SIGNAL = 0,
	LED_TYPE_PROGRAMMABLE = 1
};

enum led_state {
	LED_STATE_OFF = 0,
	LED_STATE_ON = 1,
	LED_STATE_BLINK = 2
};

#ifdef __cplusplus
extern "C" {
#endif

extern int mx_led_init(void);
extern int mx_led_get_num_of_groups(int led_type, int *num_of_groups);
extern int mx_led_get_num_of_leds_per_group(int led_type, int *num_of_leds_per_group);
extern int mx_led_set_brightness(int led_type, int group, int index, int state);
extern int mx_led_set_type_all(int led_type, int led_state);
extern int mx_led_set_all_on(void);
extern int mx_led_set_all_off(void);

#ifdef __cplusplus
}
#endif

#endif /* _MOXA_LED_H */
