//July '21 it looks like this is the currently in use program. Why? becuase of the date below (9-2-20)
//todo - radio failure has a call to config - which has been commented out - check if this is required and if so what parameter will be passed - encod or shutt
//Version 4.0 9-2-20 - change the pkversion variable too.
//this is the MASTER v3 branch created 4-1-2020

//4-feb-20 - todo look to include the number of radio resets on the LCD
//Need to think about errors occuring when there is a send and no receive expected e.g. OS, CS and SS(?)
//one way would be to expect a response from the command processor


//TEST SERIAL PRINTS were removed 9-1-20 and the sketch uploaded and tested for Radio TX and Rx only - tests ok
// when delay(100) was removed in the two retry sections, the sketch did not work
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

const int channel = 115;


String ReceivedData  = "";
String Message       = "";
String stringtosend  = "";
String blank         = "                    ";
String pkversion     = "4.0";
String NodeAddress   = "";
bool tx_sent;
char theCommand[32]  = "";                    // confusingly, you can initialise a char array in this way, but later in code, it is not possible to assign in this way.

char response[32]    = "";

int azcount          = 0;
double ssretrycount  = 0;
double azretrycount  = 0;
int FailureCount     = 0;

bool timeout = false;

void setup()
{
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
  // Print a message to the LCD.
  lcd.setCursor(0, 0);                    //remove - reinstate these 2 commands
  lcd.print("Master Radio V " + pkversion);
  delay(1000);

  SPI.begin();
  //pinMode(PIN10, OUTPUT);                   // this is an NRF24L01 requirement if pin 10 is not used
  Serial.begin(19200);                       // this module uses the serial channel to Tx/ Rx commands from the Dome driver
  printf_begin();

  ConfigureRadio("encod");



  lcdprint(0, 0, "MCU version" + pkversion );
  delay(2000);

}  // end setup


uint32_t configTimer =  millis();



void loop()
{

  NodeAddress = "";
  RadioFailureCheck();

  //move this to az, ss, os, cs TestforlostRadioConfiguration(NodeAddress);      //check for lost radio config - remove the timer



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
      NodeAddress = "encod";
      TestforlostRadioConfiguration(NodeAddress);
      AZaction();

    }  //endif received AZ


    if (ReceivedData.indexOf("OS", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {
      NodeAddress = "shutt";
      TestforlostRadioConfiguration(NodeAddress);
      radio.openWritingPipe(Shutter_address);
      theCommand[0] = 'O';                           // note single quote use
      theCommand[1] = 'S';
      theCommand[2] = '#';
      SendTheCommand();
      lcdprint(0, 2, blank);
      lcdprint(0, 2, "Sent Open Shutter   ");
      // lcdprint(0, 1, blank);

    }


    if (ReceivedData.indexOf("CS", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {

      radio.openWritingPipe(Shutter_address);

      NodeAddress = "shutt";
      TestforlostRadioConfiguration(NodeAddress);

      theCommand[0] = 'C';                           // note single quote use
      theCommand[1] = 'S';
      theCommand[2] = '#';
      SendTheCommand();
      lcdprint(0, 2, "Sent Close Shutter  ");
      // lcdprint(0, 1, blank);
    }


    if (ReceivedData.indexOf("SS", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {
      NodeAddress = "shutt";
      TestforlostRadioConfiguration(NodeAddress);
      SSaction();
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

void ReceiveTheResponse()    // this routine reads the radio for an expected transmission
{
  // new checks fromNRF24l01_failureDetect sketch - This is fail modes 2 and 3 :
  // 2  - response Timeout (i.e. we wait forever for the response back from the encoder or the shutter)
  // 3 -  the Radio is always available error

  // Note that void SendTheCommand() sets radio.startListening(), so we are in listening mode at the start of this routine - ReceiveTheResponse()

  // 2 - response timeout check first:
  // put the variable defs and while loop into a procedure and call it here and also where there are sends with no receive expected



  //check if a response has been received (case 2) and set the timeout flag if not

  CheckifResponseReceived();     //sets timeout if no response

  if ( timeout )
  { // set the flag
    radio.failureDetected = true;           // Serial.println("Radio timeout failure detected");
    // call the radio restart from here
    ConfigureRadio(NodeAddress);                   //restart the radio
  }
  else    // there was no timeout and a Tx has been received.
  {
    // Grab the response.

    //check for failure 3 - radio always available Failure:
    //*************  Note that this routine READS the response *************
    //
    CheckforRadioAlwaysAvailableError();


  }


  stringtosend = String(response) + '#';               // convert char to string for sending to driver

  // initialise the response variable
  initialisetheresponse_to_null();

  RadioFailureCheck();

} // end void receivetheresponse

void TransmitToDriver()
{
  //Serial.print ("the string sent to driver is ");

  Serial.print (stringtosend);             // print value to serial, for the driver
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
    if ( (stringtosend.indexOf("OPEN#", 0) > -1) | (stringtosend.indexOf("opening#", 0) > -1) | (stringtosend.indexOf("CLOSED#", 0) > -1) | (stringtosend.indexOf("closing#", 0) > -1) )
    {
      if (stringtosend.indexOf("OPEN#", 0) > -1)
      {
        stringtosend = "OPEN#";
      }

      if (stringtosend.indexOf("opening#", 0) > -1)
      {
        stringtosend = "opening#";
      }


      if (stringtosend.indexOf("CLOSED#", 0) > -1)
      {
        stringtosend = "CLOSED#";
      }

      if (stringtosend.indexOf("closing#", 0) > -1)
      {
        stringtosend = "closing#";
      }

      return true;
    }
    else
    {
      return false;
    }
  }

}
void AZaction()
{

  Message = "AZ";                            // need to keep this as receivedData is initialised in calls below
  //Serial.println("az");

  radio.openWritingPipe(Encoder_address);
  theCommand[0] = 'A';                             // note single quote use
  theCommand[1] = 'Z';
  theCommand[2] = '#';

  bool az_retry = false;

  while (az_retry == false )
  {
    azretrycount++;
    SendTheCommand();
    delay(100);                  // DELAY HERE SEEMS TO BE CRITICAL - as to length of delay required, it needs some empirical
    ReceiveTheResponse();
    az_retry = validate_the_response("AZ");

    //      Serial.print("stringtosend value is ");
    //    Serial.println(stringtosend);


    if (az_retry)
    {


      TransmitToDriver();
    }
    //  Serial.print("Test print AZ retry counter value ");
    //  Serial.println(azretrycount);
  }
  // update the LCD


  azcount++;                                               // TRACE ON OPEN BRACE {stringtosend.substring(0, 7)}
  lcdprint(0, 0, blank);
  lcdprint(0, 0, "Sent AZ, Rec'd ");

  lcdprint(15, 0, stringtosend.substring(0, 4));                // the current azimuth is returned from the encoder
  lcdprint(0, 1, "No of AZ calls: " + String(azcount));
  // lcdprint(0, 3, blank);


  //reset counter on 9999
  if (azcount > 999)
  {
    azcount = 0;
  }

  Message = "";

}                                                  //end void AZaction

void SSaction()
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
    delay(100);                  // DELAY HERE SEEMS TO BE CRITICAL - as to length of delay required, it needs some empirical
    ReceiveTheResponse();
    ss_retry = validate_the_response("SS");

    //  Serial.print("stringtosend value is ");                       //here
    //  Serial.println(stringtosend);

    if (ss_retry)
    {

      //     Serial.println();
      //     Serial.print("string ACTUALLY SENT IS ");                       //here
      //     Serial.println(stringtosend);
      TransmitToDriver();

    }
    //   Serial.print("Test print SS retry counter value ");
    //   Serial.println(ssretrycount);
  }

  //update lcd
  lcdprint(0, 2, "Sent Status:");
  //lcdprint(0, 1, blank);

  lcdprint(13, 2, stringtosend.substring(0, 6));
  lcdprint(0, 3, "Reset count:");
  lcdprint(13, 3, String(FailureCount));

}   // end void SSaction

void ConfigureRadio(String Address)                   //call this in Setup and then if there's a radio failure
{
  //modified to reflect the master radio

  radio.powerDown();
  delay(20);
  radio.powerUp();
  delay(20);
  // Set the PA Level low to prevent power supply related issues since this is a
  // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
  radio.begin();
  radio.setChannel(channel);                     // ensure it matches the target host causes sketch to hang
  radio.enableAckPayload();
  radio.setDataRate(RF24_250KBPS);           // set RF datarate
  radio.setPALevel(RF24_PA_LOW);
  radio.setRetries(15, 15);                  // time between tries and No of tries
  radio.enableDynamicPayloads();             // needs this for acknowledge to work
  radio.openReadingPipe(1, Master_address);  //NEED 1 for shared addresses

  if (Address == "encod")
  {
    radio.openWritingPipe(Encoder_address);
  }
  else
  {
    radio.openWritingPipe(Shutter_address);
  }
}


void TestforlostRadioConfiguration(String NodeAddress)   // this tests for the radio losing its configuration - one of the known failure modes for the NRF24l01+
{

  if (millis() - configTimer > 5000)
  {
    configTimer = millis();
    if (radio.getChannel() != 115)   // first possible radio error - the configuration has been lost. This can be checked
      // by testing whether a non default setting has returned to the default - for channel the default is 76
    {
      radio.failureDetected = true;
      Serial.print("Radio configuration error detected");
      ConfigureRadio(NodeAddress);
    }
  }

}
void CheckifResponseReceived()    // this is a timeout checker - call it after a write and startlistening
{

  unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
  // Set up a variable to indicate if a response was received or not

  while ( ! radio.available() )
  { // While nothing is received
    if (micros() - started_waiting_at > 200000 )
    { // If waited longer than 200ms, indicate timeout and exit while loop
      timeout = true;                  // radio has failed, flag set in receivetheResponse().
      break;

    }

  }

}

void RadioFailureCheck()        // this resets the timeout and failure count
{

  if (radio.failureDetected)    // if failure has been set in one of three situations, restart the radio.
  {
    radio.failureDetected = false;
    timeout = false;
    //delay(250);
    //Serial.println("Radio failure detected, restarting radio");
    // configureRadio();
    FailureCount++;
    if (FailureCount > 99)
    {
      FailureCount = 0;
    }
  }
}
void CheckforRadioAlwaysAvailableError()
{


  uint32_t failTimer = millis();
  while (radio.available())                //If available always returns true after a radio.read, there is a problem.
  {
    if (millis() - failTimer > 250)
    {
      radio.failureDetected = true;        // set the flag
      // call the radio restart from here
      Serial.println("Radio available failure detected");
      ConfigureRadio(NodeAddress);                   //restart the radio
      break;
    }

    radio.read(&response, sizeof(response));  //if all well, the read happens on the first iteration of the while loop and radio.available is cleared.
  }
}
//need this to stop stino stupid error :insert new code before this line
