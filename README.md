# syntheyes3
A customisable eye display for Raspberry Pi, using animated GIF files

# New features in the 3.xx series
* Animations are loaded from GIF files instead of being hard-coded
* Can scroll text as well
* Far more scriptable - can define random or triggered animations with percentage chances
* Animations can be multicoloured (using colours from the GIF) or monochrome with a colour of your choice
* Special effects such as rainbow cycling can be enabled for specific animations
* Events can be run before and after animations, e.g. set/clear GPIO lines, or even chaining several animations together in sequence
* Desktop linux build for testing without a raspberry Pi using SDL virtual display
* Gradient colours now configurable

## Documentation

Makefiles are provided for Raspberry Pi using a Unicorn Hat HD display, and Desktop Linux (the latter uses SDL2 to emulate the display for testing).
Other display panel drivers may be added in future.

The Pi version can have the eye colour set at runtime by using an eyeconfig3.txt file.
This should be put in the /boot partition of the Raspberry Pi SD card so that it can be edited from Windows.
If the file is not present it will abort with a scrolling error message on the display panel.

The Pi version requires the WiringPi and giflib libraries, and is intended for use with two Raspberry Pi Zeroes, each driving a single Unicorn HD board.
One must be set as the controller by grounding pin 29 (BCM5), or passing in a commandline parameter if you prefer.
The two units are linked together via the TX and RX pins (TX on the controller links to RX on the receiver) and the receiver will listen
for commands via this link.  Don't forget to ensure that there's a ground connection between both units or it won't work.

It is strongly recommended to have the same version of the software on both units.

The controller should also have a few GPIO pins spare for controlling the expression.  I use pins 31-40 for this - momentarily connecting
them to ground will trigger the expression change.

Because the Unicorn is a HAT board, you will need to modify the headers so that the pins we need are sticking out the bottom of the Pi and
not blocked by the display module.
The Zero is ideal for this, not only because it's compact but also because it usually comes with the headers unpopulated.  This means that
you can use a pair of needle-nosed pliers or a similar tool to push the pins down into the plastic spacers prior to soldering so they poke
out the bottom of the board.  This trick is particularly useful for the RX/TX/GND lines needed to link the two Pi units, though it does
make the pin shorter than normal.
For pins 29-40 it is probably better to cut the header in two at that point so the GPIO pins can be installed on the rear of the board.

To install the software, it is probably best to modify /etc/rc.local and add the following line to the end so the software is run on boot:
/bin/nice -20 /boot/eyes

...this will give you a 15-30 second boot time (which I would definitely like to improve on) but also means that the executable can be
copied to the /boot partition, and therefore the binary and config can be updated directly from windows if the end-user needs to do this.

I would strongly recommend editing /ets/fstab so that the root partition is mounted read-only - this will preserve the life of the SD card
and also helps protect the OS from filesystem corruption when the power is cut to the eye module.

# Version History

V2.99 - Initial PUT, not yet at feature parity with SynthEyes2 (Pi build working, serial link WIP so you only get one eye anyway)

V3.00 - Initial release, full feature parity with SynthEyes 2 and more besides.

# TODO for 3.01
* Test release on Xerian
* Document that mirror now works on scrollers

# TODO for 3.0x
* Refactor panel driver class to reduce redundant code
* Panel driver options (e.g. brightness)
* Panel driver should register GPIO pins it uses
* Expand GPIO mapper to cover all RPI pins
* WS2812 driver for eyes and status lights
* Background comms thread to stop the timeout glitching the display
* Startup animation for user splash screens?

# TODO for 3.1x
* Winking support (probably a flag to keep displaying IDLE on one panel)
* Redraw Xerian to make the eye image bigger
* Desktop Windows build for testing?  (This looks to be a total PITA)
* MAX7912 panel driver (monochrome only, obviously - but could support both eyes from a single Pi in Transmitter mode)
* Neopixel driver (16x16 outputs and horn status lights support)

# Future projects
* Network comms for WIFI control (how did Sofox do this for his Protogen?)
* Hub75 driver
