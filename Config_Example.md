## Config Example

### Path 
```
/etc/moxa-configs/moxa-led-control.json
```

### Description

* `CONFIG_VERSION`: The version of config file
* `LED_TYPES`: The type of LED
  * `TYPE`: Type name
  * `NUM_OF_GROUPS`: The number of groups in this type
  * `NUM_OF_LEDS_PER_GROUP`: The number of LEDs per group in this type
  * `PATHS`: The filepath of LED in nested structure

### Example: UC-5111-LX

```
{
	"CONFIG_VERSION": "1.1.0",

	"LED_TYPES": [
		{
			"TYPE": "SIGNAL",
			"NUM_OF_GROUPS": 1,
			"NUM_OF_LEDS_PER_GROUP": 3,
			"PATHS": [
				[
					"uc5100:SGN1",
					"uc5100:SGN2",
					"uc5100:SGN3"
				]
			]
		},
		{
			"TYPE": "PROGRAMMABLE",
			"NUM_OF_GROUPS": 1,
			"NUM_OF_LEDS_PER_GROUP": 1,
			"PATHS": [
				[
					"uc5100:SYS"
				]
			]
		}
	]
}
```
