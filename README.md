# moxa-led-control

`moxa-led-control` is a C library for controlling the Linux LED
subsystem via sysfs interface.

## Build

This project use autotools as buildsystem. You can build this project by the following commands:

* If the build target architecture is x86_64

	```
	# ./autogen.sh --host=x86_64-linux-gnu --includedir=/usr/include/moxa --libdir=/usr/lib/x86_64-linux-gnu --sbindir=/sbin
	# make
	# make install
	```
* If the build target architecture is armhf

	```
	# ./autogen.sh --host=arm-linux-gnueabihf --includedir=/usr/include/moxa --libdir=/usr/lib/arm-linux-gnueabihf --sbindir=/sbin
	# make
	# make install
	```

The autogen script will execute ./configure and pass all the command-line
arguments to it.

## Usage of mx-led-ctl

```
Usage:
	mx-led-ctl -s|-p <sgn_group|prog_group> -i <led_index> off|on|blink
OPTIONS:
	-s <led_group>
		Select signal LED group
	-p <led_group>
		Select programmable LED group
	-i <led_index>
		Select LED index in group
	--all-signal
		Select all signal LEDs
	--all-programmable
		Select all programmable LEDs

Example:
	Turn on 'Signal' led GROUP 2 index 3
	# mx-led-ctl -s 2 -i 3 on

	Turn off 'Programmable' led GROUP 2 index 3
	# mx-led-ctl -p 2 -i 3 off
```

## Documentation

[Config Example](/Config_Example.md)

[API Reference](/API_References.md)