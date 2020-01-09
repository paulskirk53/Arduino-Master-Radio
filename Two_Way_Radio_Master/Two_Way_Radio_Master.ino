
//this is the MASTER v3 branch created 4-1-2020
//
/*
  open the write pipe for the correct address - encoder or shutter
  stop listening
  write the command
  immediately start listening
  receive the response

  made changes such that we stoplistening - write - startlistening in that order to avoid periods where there is no listening and the slaves
  may be writing

*/
//

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal.h>
#include <printf.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to


const int rs = 27, en = 26, d4 = 25, d5 = 24, d6 = 23, d7 = 22;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);



#define PIN10  10

RF24 radio(7, 8);                            // CE, CSN
const byte Encoder_address[6] = "encod";     // the address used to write to the encoder arduino board
const byte Shutter_address[6] = "shutt";     // the address used to write to the shutter arduino board
const byte Master_address[6]  = "mastr";
String  ReceivedData  = "";
String Message = "";
String stringtosend;
String blank = "                    ";

bool tx_sent;
char theCommand[32] = "";                    // confusingly, you can initialise a char array in this way, but later in code, it is not possible to assign in this way.

char response[32] = "";

int azcount = 0;
double ssretrycount = 0;
double azretrycount = 0;

void setup()
{
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("Setup - Ready");

  //pinMode(PIN10, OUTPUT);                   // this is an NRF24L01 requirement if pin 10 is not used
  Serial.begin(115200);                       // this module uses the serial channel to Tx/ Rx commands from the Dome driver
  printf_begin();

  radio.begin();
  radio.setChannel(100);                     // ensure it matches the target host causes sketch to hang
  radio.enableAckPayload();
  radio.setDataRate(RF24_250KBPS);           // set RF datarate
  radio.setPALevel(RF24_PA_LOW);
  radio.setRetries(15, 15);                  // time between tries and No of tries
  radio.enableDynamicPayloads();             // needs this for acknowledge to work
  radio.openReadingPipe(1, Master_address);  //NEED 1 for shared addresses
  //new
  radio.openWritingPipe(Encoder_address);
  theCommand[0] = 'T';                           // note single quote use
  theCommand[1] = 'S';
  theCommand[2] = 'T';
  theCommand[3] = '#';
  SendTheCommand();
  ReceiveTheResponse();
  lcdprint(0, 0, "Comms check         ");
  lcdprint(0, 1, stringtosend);
  initialisethecommand_to_null();
  initialisetheresponse_to_null();
  Message = "";
  stringtosend = "";
  //end new

  delay (3000);     // delay for encoder to respond before more commands are send in void loop

}

void loop()
{
  /*  radio.printDetails();

    while(1);{}  //just used to stop everything in order to view the print.details



    initialise the character array used to store the commands AZ, SA, SL, OS, CS and SS
    only first 3 characters used in this sketch
    arduino uses zero as null character - no quotes
  */
  initialisethecommand_to_null();


  if (Serial.available() > 0)                      // the dome driver has sent a command
  {

    ReceivedData = Serial.readStringUntil('#');   // the string does not contain the # character

    //ES

    if (ReceivedData.indexOf("ES", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {
      Message = "ES";                            // need to keep this as receivedData is initialised in calls below
      radio.openWritingPipe(Encoder_address);
      theCommand[0] = 'E';                             // note single quote use
      theCommand[1] = 'S';
      theCommand[2] = '#';

      SendTheCommand();
      ReceiveTheResponse();

      //   update the LCD

      lcdprint(0, 0, "Sent Emergency Stop ");

      lcdprint(8, 1, stringtosend.substring(0, 7));



      Message = "";

    }  //endif received ES


    //end ES

    // AZ is the only compass related request this code will receive from the driver

    if (ReceivedData.indexOf("AZ", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {
      Message = "AZ";                            // need to keep this as receivedData is initialised in calls below
      radio.openWritingPipe(Encoder_address);
      theCommand[0] = 'A';                             // note single quote use
      theCommand[1] = 'Z';
      theCommand[2] = '#';

      bool az_retry = false;

      while (az_retry == false )
      {
        azretrycount++;
        SendTheCommand();
        delay(100);
        ReceiveTheResponse();
        az_retry = validate_the_response("AZ");

        Serial.print("stringtosend value is ");                       //here
        Serial.println(stringtosend);
        // delay(1000);

        if (az_retry)
        {
          Serial.println();
          Serial.print("string ACTUALLY SENT IS ");                       //here
          Serial.println(stringtosend);

          TransmitToDriver();
        }
        Serial.print("Test print AZ retry counter value ");
        Serial.println(azretrycount);
      }
      // update the LCD


      azcount++;                                               // TRACE ON OPEN BRACE {stringtosend.substring(0, 7)}
      lcdprint(0, 0, blank);
      lcdprint(0, 0, "Sent AZ, Rec'd ");

      lcdprint(15, 0, stringtosend.substring(0, 4));                // the current azimuth is returned from the encoder
      lcdprint(0, 2, "No of AZ calls: " + String(azcount));
      // lcdprint(0, 3, blank);
      // lcdprint(0, 3, "AZ interval " + String(azinterval, 0));
      // last_update_time = millis();

      //reset counter on 9999
      if (azcount > 999)
      {
        azcount = 0;
      }

      Message = "";

    }  //endif received AZ


    if (ReceivedData.indexOf("OS", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {

      radio.openWritingPipe(Shutter_address);
      theCommand[0] = 'O';                           // note single quote use
      theCommand[1] = 'S';
      theCommand[2] = '#';
      SendTheCommand();
      lcdprint(0, 0, blank);
      lcdprint(0, 0, "Sent Open Shutter   ");
      lcdprint(0, 1, blank);
    }


    if (ReceivedData.indexOf("CS", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {

      radio.openWritingPipe(Shutter_address);
      theCommand[0] = 'C';                           // note single quote use
      theCommand[1] = 'S';
      theCommand[2] = '#';
      SendTheCommand();
      lcdprint(0, 0, "Sent Close Shutter  ");
      lcdprint(0, 1, blank);
    }


    if (ReceivedData.indexOf("SS", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {

      radio.openWritingPipe(Shutter_address);
      theCommand[0] = 'S';                           // note single quote use
      theCommand[1] = 'S';
      theCommand[2] = '#';

      bool ss_retry = false;

      while (ss_retry == false )
      {
        ssretrycount++;
        SendTheCommand();
        delay(100);
        ReceiveTheResponse();
        ss_retry = validate_the_response("SS");

        Serial.print("stringtosend value is ");                       //here
        Serial.println(stringtosend);
        // delay(100);
        if (ss_retry)
        {

          Serial.println();
          Serial.print("string ACTUALLY SENT IS ");                       //here
          Serial.println(stringtosend);
          TransmitToDriver();

        }
        Serial.print("Test print SS retry counter value ");
        Serial.println(ssretrycount);
      }

      //update lcd
      lcdprint(0, 0, "Sent Status ?:      ");
      lcdprint(0, 1, blank);
      lcdprint(0, 1, "Received: ");
      lcdprint(10, 1, stringtosend.substring(0, 7));

    }// end if receiveddata

  } //end if serial available

} //end void loop


void initialisethecommand_to_null()
{
  for ( int i = 0; i < 32; i++)                          // initialise the command array back to nulls
  {
    theCommand[i] = 0;
  }
} // end void initialisethecommand_to_null

void initialisetheresponse_to_null()
{
  for ( int i = 0; i < 32; i++)                          // initialise the command array back to nulls
  {
    response[i] = 0;
  }
} // end void initialisetheresponse_to_null


void SendTheCommand()
{
  radio.stopListening();
  tx_sent = radio.write(&theCommand, sizeof(theCommand));

  radio.startListening();                            // put after the radio write so that no delay to start listening for response

  ReceivedData = "";

}

void ReceiveTheResponse()
{

  radio.read(&response, sizeof(response));

  stringtosend = String(response) + '#';               // convert char to string for sending to driver

  // initialise the response variable
  initialisetheresponse_to_null();

} // end void receivetheresponse

void TransmitToDriver()
{
  //Serial.print ("the string sent to driver is ");

  Serial.print ("TAKE THIS OUT " + stringtosend);             // print value to serial, for the driver
  // the string terminator # is already part of the string received from encoder


}   // end void transmit to driver

void lcdprint(int col, int row, String mess)
{
  //lcd.clear();
  lcd.setCursor(col, row);
  lcd.print(mess);

}
void TransmissionFailure()
{

  lcdprint(0, 0, "Transmission Failure");
  lcdprint(0, 1, blank);
  lcdprint(0, 2, blank);
  lcdprint(0, 3, blank);
  // write this to LCD Serial.println("[-] The transmission to the selected node failed.");
  // need to think about what to do if tx fails
}
bool validate_the_response(String receipt)
{

  if (receipt == "AZ")
  {
    double azvalue;
    azvalue = stringtosend.toDouble();
    if ((azvalue > 0) and (azvalue < 361))
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  if (receipt == "SS")
  {
    if ( (stringtosend.indexOf("OPEN#", 0) > -1) | (stringtosend.indexOf("CLOSED#", 0) > -1) )
    {
      if (stringtosend.indexOf("OPEN#", 0) > -1)
      {
        stringtosend = "OPEN#";
      }

      if (stringtosend.indexOf("CLOSED#", 0) > -1)
      {
        stringtosend = "CLOSED#";
      }
      return true;
    }
    else
    {
      return false;
    }
  }

}
