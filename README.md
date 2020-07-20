# Teensy_eurorack_hihat

image: https://user-images.githubusercontent.com/10212990/87932250-19c3a480-ca83-11ea-9b29-cb3052e256a8.jpg

A teensy microcontroller hihat module for eurorack. A no frills teensy 3.2 HiHat eurorack module. Using Teensy audio DSP
 * 
 * Based on 808 model of
 * six square waves with three filters and an envelope.
 * white noise added to fill
 * three filters for hat frequency narrowing, plus one for colour
 * use only one envelope for both open and closed, so choking is automatic
 * 
 * two trigger inputs, open and closed.
 * closed chokes open
 * three poteniometers
 * - length of closed
 * - length of open
 * - resonance or filter effect
 * onboard DAC gives a single mixed audio output on pin14
 * two trigger indicators leds
 * 
 * 
 * six square waves to make an 808 hihat: http://joesul.li/van/synthesizing-hi-hats/

Fundamental freq =40

Osc1 ratio:2

Osc2 ratio:3 

Osc3 ratio:4.16 

Osc4 ratio:5.43

Osc5 ratio:6.79

Osc6 ratio:8.21

other influences:
https://www.muffwiggler.com/forum/viewtopic.php?t=120280
