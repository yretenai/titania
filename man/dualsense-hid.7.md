% daualsense-hid(7) Version 1 | DualSense Report Descriptors

NAME
====

**daualsense-hid** - *unofficial* Sony Playstation DualSense controller HID report structures

DESCRIPTION
===========

This page describes the various reports used by the DualSense controller.

FORMATTING
==========

The format of this page describes binary structures in a meta-syntax.

**Endianness** is defined in the report description.

**Field types** are denoted by bracketing the name
 ~ e.g. `**\[int\]**

**Field names** are *snake_cased*.

**Bit fields** are denoted in a table, described below.

**Unions** are denoted by a **\[union(n)\]** keyword, where *n* is the union size.
 ~ e.g. **\[uint32\]\[union(4)\]** value **\[uint8(4)\]\[union(4)\]** values

**Alignment** is denoted by a **\[align(n)\]** keyword, where *n* is the alignment value.
 ~ e.g. **\[align(1)\]** denoting an alignment of 1 (tightly packed.)

**Normalization** is denoted by a **\[norm(n)\]** keyword, where *n* is the divisor.
 ~ e.g. **\[norm(255)\]** denoting a normalization of 255 (value / 255.)

**Signed Normalization** is denoted by a **\[snorm(n)\]** keyword, where *n* is the divisor.
 ~ e.g. **\[snorm(128)\]** denoting a normalization of 128 (value / 128.)

**Hexadecimal** (base-16) numbers are prefixed with *0x*.
 ~ e.g. *0xA* indicating *10* in base-10.

**Binary** (base-2) numbers are prefixed with *0b*.
 ~ e.g. *0b110* indicating *6* in base 10.

BIT FIELDS
----------

Bit fields are defined as follows:

| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|---|---|---|---|---|---|---|---|
| x | x | x | x | y | y | y | y |

Where 0 is the least significant bit and 7 is the most significant bit.

The bit field table will also accompany magic numbers used to access the values.

e.g.

**x: \[uint8\]** first
 ~ value 1
 ~ **value & 0xF**

**y: \[uint8\]** second
 ~ value 2
 ~ **(value >> 4) & 0xF**

BUILT-IN TYPES
--------------

These types are intrinsic and built-into most languages.

**\[bool\]**
 ~ a 1-bit value.

**\[uint8\]**
 ~ an unsigned 8-bit value.

**\[uint16\]**
 ~ an unsigned 16-bit value.

**\[uint32\]**
 ~ an unsigned 32-bit value.

**\[uint64\]**
 ~ an unsigned 64-bit value.

**\[int8\]**
 ~ a signed 8-bit value.

**\[int16\]**
 ~ a signed 16-bit value.

**\[int32\]**
 ~ a signed 32-bit value.

**\[int64\]**
 ~ a signed 64-bit value.

**\[char\]**
 ~ an unsigned 8-bit value, denoting a UTF-8 byte.

**\[wchar\]**
 ~ an unsigned 16-bit value, denoting a UTF-16 word.

**\[t(n)\]**
 ~ an array, where *t* is the given type, and *n* is the count.
 ~ e.g. **\[char(8)\]** meaning 8 **\[char\]** values in sequence.


REPORTS
=======

REPORT 0x1 INPUT
----------------

this *report* is an *INPUT DATA* report with *Report ID 1*.

this *report* is *64 bytes*.

*endianness* is *LITTLE*.

**\[uint8\]** report_id
 ~ this will be 1.

**\[uint8(2)(2)\]\[norm(255)\]** sticks
 ~ left x, left y, right x, right y
 ~ directional data for the analog sticks.

**\[uint8(2)\]\[norm(255)\]** triggers
 ~ left, right
 ~ pressure level for each of the triggers

**\[uint8\]** usb sequence
 ~ always increments by 1 each time data is sent through the USB cable

**\[bitset_buttons\]** buttons
 ~ buttons bitset, see below

**\[int16(3)\]\[snorm(32767)\]** accelerometer
 ~ accelerometer force

**\[int16(3)\]\[snorm(32767)\]** gyroscope
 ~ gyroscope orientation

**\[uint32\]** hr_time
 ~ high resolution time

**\[uint8\]** temperature
 ~ processor temperature in celsius

**\[touchpad(2)\]** touch_points
 ~ left and right touch points, see below

**\[uint8\]** last_touch_id
 ~ increments by 1 each time a touchpad is touched

**\[adaptive_trigger(2)\]** adaptive_triggers
 ~ right and left triggers, see below

**\[uint32\]** state_id
 ~ state_id provided by the output report

**\[device_state\]** state
 ~ device state flags, see below

**\[checksum\]** checksum
 ~ secure signed checksum of the entire message (excluding this field.)

REPORT 0x2 OUTPUT
-----------------

this *report* is an *OUTPUT DATA* report with *Report ID 2*.

this *report* is *64 bytes* for *DualSense Edge, and **48 bytes** for DualSense.

*endianness* is *LITTLE*.

REPORT 0x5 CALIBRATION
----------------------

REPORT 0x8 CONNECT
------------------

REPORT 0x9 SERIAL
-----------------

REPORT 0xA PAIR
---------------

REPORT 0x20 FIRMWARE
--------------------

REPORT 0x21 AUDIO
-----------------

REPORT 0x31 BLUETOOTH
---------------------

REPORT 0x60, 0x61, 0x63 UPDATE EDGE PROFILE
--------------------------------------

REPORT 0x63, 0x70, 0x73, 0x76, 0x79 EDGE PROFILE PART 1
-------------------------------------------------------

REPORT 0x64, 0x71, 0x74, 0x77, 0x7A EDGE PROFILE PART 2
-------------------------------------------------------

REPORT 0x65, 0x72, 0x75, 0x78, 0x7B EDGE PROFILE PART 3
-------------------------------------------------------

CHECKSUM
--------

Reports **must have** a valid checksum if transmitted with Bluetooth.

:   The checksum algorithm is CRC32

:   The initial state is *0xFFFFFFFF*

:   The output state is XORed with `0xFFFFFFFF` (or otherwise: `~value`)

:   The polynomial is 0xEDB88320

:   Input Report buffers are prefixed with *0xA1*
~   e.g 0xA1 0x01 0x80 0x80...

:   Output Report buffers are prefixed with *0xA2*
~   e.g 0xA2 0x02 0xFF 0xFF...

:   Feature Report buffers are prefixed with *0xA3*
~   e.g 0xA3 0x05 0x30 0x30...

:   DualSense Edge Feature Report buffers are prefixed with *0x53*
~   e.g 0x53 0x05 0x30 0x30...

STRUCTURES
==========

touchpad
--------

this *bitset* is *4 bytes*.

defines a touch point on the touchpad.

| 0. | 1. | 2. | 3. | 4. | 5. | 6. | 7. | 8. | 9. | 10 | 11 | 12 | 13 | 14 | 15 |
|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
| i  | i  | i  | i  | i  | i  | i  | t  | x  | x  | x  | x  | x  | x  | x  | x  |

| 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 |
|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
| x  | x  | x  | x  | y  | y  | y  | y  | y  | y  | y  | y  | y  | y  | y  | y  |

**i: \[uint8\]** id
 ~ increments by one each time this touch point is activated
 ~ **value & 0x7F**

**t: \[bool\]** touched
 ~ set when the touch point is activated
 ~ **(value >> 7) & 1**

**x: \[uint16\]** x position
 ~ x coordinate, max is 1920
 ~ **(value >> 8) & 0xFFF**

**y: \[uint16\]** y position
 ~ y coordinate, max is 1080
 ~ **(value >> 20) & 0xFFF**

adaptive_trigger
----------------

this *bitset* is *1 byte*.

defines attributes a given trigger effect is currently in

| 0. | 1. | 2. | 3. | 4. | 5. | 6. | 7. |
|----|----|----|----|----|----|----|----|
| s  | s  | s  | s  | t  | t  | t  | t  |

**s: \[uint8\]** section
 ~ determines the section the trigger effect is currently in
 ~ **value & 0xF**

**t: \[uint8\]** state
 ~ determines the active state of the trigger effect
 ~ **(value >> 4) & 0xF**

device_state
------------

this *struct* is *4 bytes*.

defines device state flags


**\[device_trigger_state\]** trigger
 ~ trigger effect state

**\[dualsense_edge_state\]\[union(4)\]** edge_state
 ~ edge device specific state flags, not present on regular controllers.

**\[uint32\]\[union(4)\]** battery_time
 ~ battery time, not present on edge controllers.

**\[device_battery_state\]** battery_state
 ~ battery error state and battery level

**\[device_controller_state\]** controller_state
 ~ controller state flags

device_controller_state
-----------------

this *bitset* is *2 bytes*.

| 0. | 1. | 2. | 3. | 4. | 5. | 6. | 7. | 8. | 9. | 10 | 11 | 12 | 13 | 14 | 15 |
|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
| a0 | a1 | a2 | d0 | d1 | r0 | r0 | r0 | a3 | d2 | r1 | r1 | r1 | r1 | r1 | r1 |

**a0: \[bool\]** headphones_connected
 ~ headphone jack is plugged in
 ~ **value & 1**

**a1: \[bool\]** headset_connected
 ~ headphone jack is plugged in with line in audio
 ~ **(value >> 1) & 1**

**a2: \[bool\]** mic_muted
 ~ device microphone is muted
 ~ **(value >> 2) & 1**

**d0: \[bool\]** usb_data
 ~ usb cable is providing data
 ~ **(value >> 3) & 1**

**d1: \[bool\]** usb_power
 ~ usb cable is plugged in and providing power
 ~ **(value >> 4) & 1**

**r0: \[uint8\]** reserved
 ~ reserved bits
 ~ **(value >> 5) & 7**

**a3: \[bool\]** external_mic
 ~ external mic is used
 ~ **(value >> 3) & 1**

**d2: \[bool\]** haptic_filter
 ~ haptic filter is enabled
 ~ **(value >> 4) & 1**

**r1: \[uint8\]** reserved
 ~ reserved bits
 ~ **(value >> 5) & 0x3f**

device_edge_state
-----------------

this *bitset* is *4 bytes*.

| 0. | 1. | 2. | 3. | 4. | 5. | 6. | 7. | 8. | 9. | 10 | 11 | 12 | 13 | 14 | 15 |
|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
| p0 | p1 | p2 | p3 | p4 | p4 | p4 | p5 | s0 | s1 | s2 | s3 | l  | l  | r  | r  |

| 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 |
|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
| d  | d  | d  | d  | b0 | b1 | b2 | b3 | e  | o  | o  | b4 | b5 | b6 | b7 | b8 |

**p0: \[bool\]** unknown1
 ~ todo
 ~ **value & 1**

**p1: \[bool\]** unknown2
 ~ todo
 ~ **(value >> 1) & 1**

**p2: \[bool\]** led_indicator_enabled
 ~ set when LEDs are enabled for profile switching indicators
 ~ **(value >> 2) & 1**

**p3: \[bool\]** vibration_indicator_enabled
 ~ set when vibration is enabled for profile switching indicators
 ~ **(value >> 3) & 1**

**p4: \[enum_profile\]** profile_id
 ~ current profile id
 ~ **(value >> 4) & 0x7**

**p5: \[bool\]** profiles_disabled
 ~ set profiles are disabled
 ~ **(value >> 7) & 1**

**s0: \[bool\]** stick_disconnected
 ~ set when one of the sticks has been disconnected
 ~ **(value >> 8) & 1**

**s1: \[bool\]** stick_error
 ~ set when one of the sticks failed
 ~ **(value >> 9) & 1**

**s2: \[bool\]** stick_calibrating
 ~ set when one of the sticks is initializing and calibrating
 ~ **(value >> 10) & 1**

**s3: \[bool\]** stick_unknown
 ~ todo
 ~ **(value >> 11) & 1**

**l: \[uint8\]** left_trigger_level
 ~ left trigger level state
 ~ **(value >> 12) & 3**

**r: \[uint8\]** right_trigger_level
 ~ right trigger level state
 ~ **(value >> 14) & 3**

**d: \[enum_dpad\]** dpad
 ~ dpad state (bypasses mapped buttons)
 ~ **(value >> 16) & 0xF**

**b0: \[bool\]** square
 ~ is square pressed (bypasses mapped buttons)
 ~ **(value >> 20) & 1**

**b1: \[bool\]** cross
 ~ is cross pressed (bypasses mapped buttons)
 ~ **(value >> 21) & 1**

**b2: \[bool\]** circle
 ~ is circle pressed (bypasses mapped buttons)
 ~ **(value >> 22) & 1**

**b3: \[bool\]** triangle
 ~ is triangle pressed (bypasses mapped buttons)
 ~ **(value >> 23) & 1**

**e: \[bool\]** emulating_rumble
 ~ is emulating rumble? will be set when rumble has been performed this connection
 ~ **(value >> 24) & 1**

**o: \[uint8\]** brightness_override
 ~ LED brightness override value
 ~ **(value >> 25) & 3**

**b4: \[bool\]** unknown
 ~ todo
 ~ **(value >> 27) & 1**

**b5: \[bool\]** mute
 ~ is mute pressed (bypasses mapped buttons)
 ~ **(value >> 28) & 1**

**b6: \[bool\]** ps
 ~ is ps pressed (bypasses mapped buttons)
 ~ **(value >> 29) & 1**

**b7: \[bool\]** create
 ~ is create pressed (bypasses mapped buttons)
 ~ **(value >> 30) & 1**

**b8: \[bool\]** option
 ~ is option pressed (bypasses mapped buttons)
 ~ **(value >> 31) & 1**

device_battery_state
--------------------

this *bitset* is *1 byte*.

defines battery error state and battery level

| 0. | 1. | 2. | 3. | 4. | 5. | 6. | 7. |
|----|----|----|----|----|----|----|----|
| l  | l  | l  | l  | s  | s  | s  | s  |

**l: \[uint8\]** right
 ~ battery level
 ~ **value & 0xF**

**s: \[uint8\]** state
 ~ status state of the battery
 ~ **(value >> 4) & 0xF**

device_trigger_state
--------------------

this *bitset* is *1 byte*.

defines what effect is applied to each trigger

| 0. | 1. | 2. | 3. | 4. | 5. | 6. | 7. |
|----|----|----|----|----|----|----|----|
| r  | r  | r  | r  | l  | l  | l  | l  |

**r: \[uint8\]** right
 ~ effect id of the right trigger
 ~ **value & 0xF**

**l: \[uint8\]** left
 ~ effect id of the left trigger
 ~ **(value >> 4) & 0xF**

bitset_buttons
--------------

this *bitset* is *4 bytes*.

| 0. | 1. | 2. | 3. | 4. | 5. | 6. | 7. | 8. | 9. | 10 | 11 | 12 | 13 | 14 | 15 |
|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
| a  | a  | a  | a  | b  | c  | d  | e  | f  | g  | h  | i  | j  | k  | l  | m  |

| 16 | 17 | 18 | 19 | 20 | 21 | 22 | 23 | 24 | 25 | 26 | 27 | 28 | 29 | 30 | 31 |
|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
| n  | o  | p  | q  | r  | s  | t  | u  | v  | v  | v  | v  | v  | v  | v  | v  |

**a: \[enum_dpad\]** dpad
 ~ dpad state
 ~ **value & 0xF**

**b: \[bool\]** square
 ~ is square pressed
 ~ **(value >> 4) & 1**

**c: \[bool\]** cross
 ~ is cross pressed
 ~ **(value >> 5) & 1**

**d: \[bool\]** circle
 ~ is circle pressed
 ~ **(value >> 6) & 1**

**e: \[bool\]** triangle
 ~ is triangle pressed
 ~ **(value >> 7) & 1**

**f: \[bool\]** l1 :
 ~ is l1 : pressed
 ~ **(value >> 8) & 1**

**g: \[bool\]** r1
 ~ is r1 pressed
 ~ **(value >> 9) & 1**

**h: \[bool\]** l2
 ~ is l2 pressed
 ~ **(value >> 10) & 1**

**i: \[bool\]** r2
 ~ is r2 pressed
 ~ **(value >> 11) & 1**

**j: \[bool\]** create
 ~ is create pressed
 ~ **(value >> 12) & 1**

**k: \[bool\]** option
 ~ is option pressed
 ~ **(value >> 13) & 1**

**l: \[bool\]** l3
 ~ is l3 pressed
 ~ **(value >> 14) & 1**

**m: \[bool\]** r3
 ~ is r3 pressed
 ~ **(value >> 15) & 1**

**n: \[bool\]** ps
 ~ is ps pressed
 ~ **(value >> 16) & 1**

**o: \[bool\]** touch
 ~ is touch pressed
 ~ **(value >> 17) & 1**

**p: \[bool\]** mute
 ~ is mute pressed
 ~ **(value >> 18) & 1**

**q: \[bool\]** reserved
 ~ reserved value
 ~ **(value >> 19) & 1**

**r: \[bool\]** edge_f1
 ~ is edge f1 pressed
 ~ **(value >> 20) & 1**

**s: \[bool\]** edge_f2
 ~ is edge f2 pressed
 ~ **(value >> 21) & 1**

**t: \[bool\]** edge_lb
 ~ is edge left paddle pressed
 ~ **(value >> 22) & 1**

**u: \[bool\]** edge_rb
 ~ is edge right paddle pressed
 ~ **(value >> 23) & 1**

**v: \[uint8\]** reserved2
 ~ reserved values
 ~ **(value >> 24) & 0xFF**

enum_dpad
---------

this *enum* is *1 byte*.

defines dpad position

**0** up
 ~ up is pressed

**1** up right
 ~ up and right are pressed

**2** right
 ~ right is pressed

**3** down right
 ~ down and right are pressed

**4** down
 ~ down is pressed

**5** down left
 ~ down and left are pressed

**6** left
 ~ left is pressed

**7** up left
 ~ up and left are pressed

**8** clear
 ~ no buttons are pressed


enum_profile
------------

this *enum* is *1 byte*.

defines profile id

**0** none
 ~ no profile is selected

**1** triangle
 ~ profile 1 (triangle)

**2** square
 ~ profile 2 (square)

**3** cross
 ~ profile 3 (cross)

**4** circle
 ~ profile 4 (circle)
