# Arduino-Master-Radio
This is the master node arduino sketch linked to the Observatory Dome driver. It Transmits to the Arduino Encoder board to get the Dome Azimuth related data, and to the Arduino Shutter board for shutter related operations.

It uses an arduino mega 2560 board with an NRF24L01+ radio transceiver. This branch no longer uses acknowledgepayload to return data from the two other nodes.
It was difficult to get working effectively - ended up writing code to deal with the non synchronous actions of the nrf rather than the functions I needed to address, so i have now remodelled on a synchronous comms approach with the master and slaves acting as transceivers. It was much easier to program, despite what folks have said about ease of use of the AckPayload feature of the NRF24L01+ - it is very difficult to get working reliably....
The sketch transmits to two nodes - one node is a shaft encoder sketch and the other controls two small DC motors which pull a string to open a door in the observatory.
