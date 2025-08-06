
As the completed IONOS development boards are sold-out, we have looked for an alternative platform​ for users of this software...
(A few bare boards are still available!)

There is an option....

Proto-supply has a development board using the Teensy 4.1, a 320x480 TFT display, and a SGTL5000 Codex.
https://protosupplies.com/product/mini-platform-teensy41/

All that is needed to run the IONOS code is a pair of encoders!,
just wire them to their I/O connector, 7 wires.
https://protosupplies.com/product/dual-rotary-encoder-module/

There is NO Audio buffering on the mini-platform, so audio in-out is connected directly to the SGTL5000 codex.

Or, we did an "encoder proto board" for another project that plugs directly into the mini-platform. 
As a minimum, you need only to install the two encoders, the 24 pin I/O connector and four jumpers.
The cost was less than $8 for 5 boards plus shipping from:
https://jlcpcb.com/

We have a version of code that runs on the board, with modification for the new TFT display.

​Design documents for the ​"encoder proto board​", including schematics, and gerber files are available on this site​.
S​ource code and binary HEX files for the updated TFT display are also available.

To use this board for IONOS, as a minimun, just install the two encoders, P1 and replace R5,R6,R9,R10 with jumpers.
Nothing else is needed...

If you build code from source, you will need Teensyduino version 1.60b4 or newer to support the ST7789 TFT display.
A .hex file is avilable:

New alternate hardware platforms/src/build/teensy.avr.teensy41/HFSim_BFD_2_03_Proto.ino.hex

https://forum.pjrc.com/index.php?forums/announcements.6/

tom@lafleur.us ka6iqa
