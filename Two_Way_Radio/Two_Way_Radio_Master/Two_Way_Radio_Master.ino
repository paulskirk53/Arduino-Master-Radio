//this is the MASTER
//experiment to try a two way radio comms without using the ackpayload feature
// hopefully will make comms more straightforward
// 

#include <SPI.h>
#include <RF24.h>
#include <SoftwareSerial.h>

#define PIN10  10

RF24 radio(7, 8);                                 // CE, CSN
const byte Encoder_address[6] = "00001";          //the address used to write to the encoder arduino board
const byte Shutter_address[6] = "00002";          //the address used to write to the shutter arduino board
const byte Master_address[6] = "00000";
String  ReceivedData  = "";
String Message = "";
String stringtosend;

bool tx_sent;
char theCommand[32] = "";                   // confusingly, you can initialise a char array in this way, but later in code, it is not possible to assign in this way.
double ReceivedPayload = 0.0;
char response[32] = "";

void setup()
{
  //pinMode(PIN10, OUTPUT);      // this is an NRF24L01 requirement if pin 10 is not used
  Serial.begin(9600);            // this module uses the serial channel to Tx/ Rx commands from the Dome driver
  radio.begin();
  radio.setChannel(100); //ensure it matches the target host causes sketch to hang

  radio.enableAckPayload();

  radio.setDataRate(RF24_250KBPS);  // set RF datarate didn't work with the + devices either



  radio.setPALevel(RF24_PA_MIN);
  radio.setRetries(5, 10);           // time between tries and No of tries
  radio.enableDynamicPayloads();     // needs this for acknowledge to work


}

void loop()
{
  // initialises the character array used to store the commands AZ, SA, SL, OS, CS and SS
  theCommand[0] = 0;                                // only first 3 characters used in this sketch
  theCommand[1] = 0;                                // arduino uses zero as null character - no quotes
  theCommand[2] = 0;

  if (Serial.available() > 0)         // the dome driver has sent a command
  {
    // Serial.print("Radio channel used is ");
    // Serial.println(radio.getChannel());
    ReceivedData = Serial.readStringUntil('#');            // the string does not contain the # character

    //ReceivedData.equalsIgnoreCase("AZ")
    if ((ReceivedData.equalsIgnoreCase("AZ")) || (ReceivedData.equalsIgnoreCase("SA")) || (ReceivedData.equalsIgnoreCase("SL")))
    {

      //these three commands all just require the azimuth to be returned, so just send AZ# as the command for all three

      radio.openWritingPipe(Encoder_address);
      theCommand[0] = 'A';                                // note single quote use
      theCommand[1] = 'Z';
      theCommand[2] = '#';


      //Serial.print ("theCommand contains  ");
      //Serial.println(theCommand);                      // print the string terminator
      SendTheCommand();
      ReceiveTheResponse();
      TransmitToDriver();

    }

    if (ReceivedData.equalsIgnoreCase("OS"))   // fill this area for open shutter
    {
      radio.openWritingPipe(Shutter_address);
      theCommand[0] = 'O';                                // note single quote use
      theCommand[1] = 'S';
      theCommand[2] = '#';
      SendTheCommand();         // this command works as part of the 2 way


    }

    if (ReceivedData.equalsIgnoreCase("CS"))   // fill this area for close shutter
    {
      radio.openWritingPipe(Shutter_address);
      theCommand[0] = 'C';                                // note single quote use
      theCommand[1] = 'S';
      theCommand[2] = '#';
      SendTheCommand();


    }
    if (ReceivedData.equalsIgnoreCase("SS"))   // fill this area for shutter status
    {
      radio.openWritingPipe(Shutter_address);
      theCommand[0] = 'S';                                // note single quote use
      theCommand[1] = 'S';
      theCommand[2] = '#';
      SendTheCommand();                                  // ok to here
      ReceiveTheResponse();
      // this is the only part of the shutter code which returns a value.
      // the payload will be 0 or 1, the driver requires 'open' or closed' so transpose appropriately
      if (ReceivedPayload == 1.0)
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

    }// end if receiveddata.startswith

  } //end if serial available

} //end void loop

void SendTheCommand()
{

  tx_sent = radio.write(&theCommand, sizeof(theCommand));
  Serial.print("The text sent was ");
  Serial.println(theCommand);
  ReceivedData = "";


}

void ReceiveTheResponse()
{
  if (tx_sent)
  {
    radio.openReadingPipe(1, Master_address);     //NEED 1 for shared addresses
    radio.startListening();
    while (!radio.available())
    {}
    if (radio.available())
    {
      radio.read(&response, sizeof(response));
      radio.stopListening();
      stringtosend = String(response);                  // convert char to string for sending to driver
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


  // need to change response to string i.e char to string
  Serial.println("[+] Successfully received data from node: ");
  Serial.print("The Transmit to driver message is ");

  Serial.print (stringtosend); //print value to serial, for the driver
  Serial.println("#");                      // print the string terminator

}
