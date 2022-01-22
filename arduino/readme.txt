SynthEyes V3 does not currently drive the status lights on a Synth helmet,
owing to a lack of available pins and the fact that the Neopixels are
expecting to receive 5V logic instead of the 3V that the Pi uses.

Instead, here is a simple Arduino program to ramp the status lights.
It can also flash them in time to an audio signal from a microphone board
if you want the status lights to react to the wearers voice.

Finally, SynthEyes V3 can be configured to set and clear GPIO lines when
a given expression starts and stops.  If this GPIO line is connected to
FAULT_PIN on the Arduino (making sure you have a ground connection too!),
the Arduino software will change the status light colour for the duration
of the expression on the eye display.

See the provided config file for Xerian as an example.

