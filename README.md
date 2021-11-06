# RAK4631-ublox-Commanderr

This is a tokenizer and parser for raw NMEA sentences. This is not intended (yet anyway) for production, but as an exercice in re-inventing the wheel. There are other reasons for me to do this, including a smaller footprint (I need that code to be as small as possible). Suffice to say, I had good enough reasons to do this, and am happy to put it up there for others to enjoy. Or not. Many verbs are recognized, withe the Talker ID (the first 2 letters) ignored. Information that repeats previously identical information (like same location, etc) is not re-displayed in the Terminal.

I added code to send commands to the ublox receiver, to set it up with more functionalities, and have a look at existing settings. This is very much a work in progress. This part stems from my need to enable Beidou (I live in HK and there are lots of Beidou satellites above my head).

TODO

* Many more UBX commands
* A couple of extra NMEA verbs
* BLE UART functionality.
* OLED display

