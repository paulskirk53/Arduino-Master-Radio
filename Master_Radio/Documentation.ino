// 17-11-2018
// This sketch is uploaded to an arduino board connected to the observatory PC by a USB cable.
// The arduino communicates with two other arduino boards using the NRF24l01+ transceivers
// modelled on all three boards.
// This skecth requests actions from the arduino shutter board (open, close, status)
// It reuests the azimuth angle and status from the encoder arduino.


//Take care with the transmission buffer there are only three on the NRF, so if the buffer receives more than
// three writes before a radio.receive is executed, the sketch will hang.
//The sketch is maintaine in github - https://github.com/paulskirk53/Arduino-Master-Radio
// there is inline documentation in key areas

