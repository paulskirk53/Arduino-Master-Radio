
//    Name:       Master_Radio.ino
//    Created:	11/11/2018 18:22:57
//    Author:     DESKTOP-OCFJAV9\Paul
// in boiler this board is UNo port 6
// working transmitter code
// works with Radio_Receiver1 as tested 13/11/18
//

#include <SPI.h>
#include <RF24.h>
#include <SoftwareSerial.h>

#define PIN10  10

RF24 radio(7, 8); // CE, CSN
const byte Encoder_address[6] = "00001";          //the address used to write to the encoder arduino board
const byte Shutter_address[6] = "00002";          //the address used to write to the shutter arduino board
String  ReceivedData  = "";
String Message = "";
bool tx_sent;

double ReceivedPayload = 0.0;

void setup()
{
  //pinMode(PIN10, OUTPUT);      // this is an NRF24L01 requirement if pin 10 is not used
  Serial.begin(9600);            // this module uses the serial channel to Tx/ Rx commands from the Dome driver

  // radio.setChannel(0x02); //ensure it matches the target host causes sketch to hang
  radio.begin();
  radio.enableAckPayload();

  //  radio.setDataRate(RF24_250KBPS);  // set RF datarate didn't work with the + devices either



  radio.setPALevel(RF24_PA_MIN);
  radio.setRetries(5, 10);           // time between tries and No of tries
  radio.enableDynamicPayloads();     // needs this for acknowledge to work


}

void loop()
{

  //Serial.print("serial test ");     // test used in v early stages
  if (Serial.available() > 0)         // the dome driver has sent a command
  {
    // Serial.print("Radio channel used is ");
    // Serial.println(radio.getChannel());
    ReceivedData = Serial.readStringUntil('#');


    //ReceivedData.equalsIgnoreCase("AZ")
    if ((ReceivedData.equalsIgnoreCase("AZ")) || (ReceivedData.equalsIgnoreCase("SA")) || (ReceivedData.equalsIgnoreCase("SL")))
    {
      radio.openWritingPipe(Encoder_address);
      SendTheCommand();
      ReceiveTheResponse();
      TransmitToDriver();

    }

    if (ReceivedData.startsWith("OS", 0))   // fill this area for open shutter
    {
      radio.openWritingPipe(Shutter_address);
      SendTheCommand();


    }

    if (ReceivedData.startsWith("CS", 0))   // fill this area for close shutter
    {
      radio.openWritingPipe(Shutter_address);
      SendTheCommand();


    }
    if (ReceivedData.startsWith("SS", 0))   // fill this area for shutter status
    {
      radio.openWritingPipe(Shutter_address);
      SendTheCommand();
      ReceiveTheResponse();
      // this is the only part of the shutter code which returns a value.
      // the payload will be 0 or 1, the driver requires 'open' or closed' so transpose appropriately
      if (ReceivedPayload == 1)
      {
        Message = "closed";                        //the shutter is closed
        Serial.print (Message);                    //print value to serial, for the driver
        Serial.println("#");                      // print the string terminator
      }
      else
      {
        Message = "open";       //the shutter is open
        Serial.print (Message); //print value to serial, for the driver
        Serial.println("#");                      // print the string terminator
      }

    }
  } // end if receiveddata.startswith

  //end if serial available
}

void SendTheCommand()
{

  tx_sent = radio.write(&ReceivedData, sizeof(ReceivedData));
  Serial.print("The text sent was ");
  Serial.println(ReceivedData);
  ReceivedData = "";


}

void ReceiveTheResponse()
{
  if (tx_sent)
  {
    if (radio.isAckPayloadAvailable())   // tests if the remote device has acknowledged and its
      // acknowledge payload (i.e its response )is available
    {

      // read ack payload and copy data to the remoteresponse variable
      radio.read(&ReceivedPayload, sizeof(ReceivedPayload));

    }
  }
  else
  {
    Serial.print("[-] The transmission to the selected node failed.");
  }

  Serial.println("--------------------------------------------------------");

}
void TransmitToDriver()
{
  Serial.println("[+] Successfully received data from node: ");
  Serial.print("The ReceivedPayload returned was ");

  Serial.print (String(ReceivedPayload, 2)); //print value to serial, for the driver
  Serial.println("#");                      // print the string terminator

}
