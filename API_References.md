# API References

---
### int mx_led_init(void)

Initialize Moxa LED control library.

#### Return value
* 0 on success.
* negative numbers on error.

---
### int mx_led_set_brightness(int led_type, int group, int index, int state)

Set LED state on, off, blink.

#### Parameters
* led_type: LED_TYPE_SIGNAL or LED_TYPE_PROGRAMMABLE
* group: the group number
* index: the LED index
* state: LED_STATE_OFF or LED_STATE_ON or LED_STATE_BLINK

#### Return value
* 0 on success.
* negative numbers on error.

---
### int mx_led_set_all_off(void)

Set all LED off.

#### Return value
* 0 on success.
* negative numbers on error.

---
### int mx_led_set_all_on(void)

Set all LED on.

#### Return value
* 0 on success.
* negative numbers on error.

---