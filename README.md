# Arduino-Master-Radio
This is the master node arduino sketch linked to the Observatory Dome driver. It Transmits to the Arduino Encoder board to get the Dome Azimuth related data, 
and to the Arduino Shutter board for shutter related operations.

It uses an arduino uno board with an NRF24L01+ radio transceiver. It uses acknowledgepayload to return data from the two other nodes.
It was difficult to get working until you realise that the acknowledgepayload feature is not under software control. The data is returned
from the slave nodes to the master node by the NRF24l01+ hardware and the send is triggered by a software radio.read(....).
So the process order for successful data return to the master transmitter has to be

1 - Populate the acknowledge buffer (take care as there are only three and if you write to the buffer more than 3 times before executing a
radio.read(....) the sketch is likely to hang.
2 - Issue a radio.read(...) command to receive incoming data - this triggers the shocburst feature of the chip to return the buffer contents
to the transmitter chip.

So it is possible to get two way comms going with a pair of NRF24l01+ devices without the hassle of changing roles.

The sketch transmits to two nodes - one node is a shaft encoder sketch and the other controls two small DC motors which pull a string to open a
door in the observatory.
