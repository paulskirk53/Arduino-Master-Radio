
//this is the MASTER reviewed and checked for process on Sunday 2-12 @12.37 see below
// needs some error checks  and perhaps printing to an LCD with two rows for sent and two for received
//bug 2 hashes added to command -
// sequence needs to be this for master
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
const byte Master_address[6] = "mastr";
String  ReceivedData  = "";
String Message = "";
String stringtosend;

bool tx_sent;
char theCommand[32] = "";                    // confusingly, you can initialise a char array in this way, but later in code, it is not possible to assign in this way.
double ReceivedPayload = 0.0;
char response[32] = "";
int x=0;
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
	  lcdprint(0,1,"Comms check");
	  lcdprint(0,2,stringtosend);

	  stringtosend = "";
//end new

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

    if (ReceivedData.equalsIgnoreCase("ES")) 
    {
      Message = ReceivedData;                            // need to keep this as receivedData is initialised in calls below
      radio.openWritingPipe(Encoder_address);
      theCommand[0] = 'E';                             // note single quote use
      theCommand[1] = 'S';
      theCommand[2] = '#';

      SendTheCommand();
      ReceiveTheResponse();

      if (  Message.equalsIgnoreCase("ES") )            //  just update the LCD
      {
        lcdprint(0, 0, "Sent Emergency Stop ");

        lcdprint(8, 1, stringtosend.substring(0, 7));
      }


      Message = "";

    }  //endif received ES


    //end ES

    // AZ is the only compass related request this code will receive from the driver

    if ( (ReceivedData.equalsIgnoreCase("AZ")) )
    {
      Message = ReceivedData;                            // need to keep this as receivedData is initialised in calls below
      radio.openWritingPipe(Encoder_address);
      theCommand[0] = 'A';                             // note single quote use
      theCommand[1] = 'Z';
      theCommand[2] = '#';

	/*  //new

	    x=x+1;
	    lcd.clear();
	    lcd.setCursor(0, 0);
	    lcd.print("Count of Az sent:");
	    lcd.setCursor(0, 1);
	    lcd.print(x);
	    delay(2000);
// trace here {theCommand[0]}{theCommand[1]}{theCommand[2]}

	  // end new */

      SendTheCommand();
      ReceiveTheResponse();
      TransmitToDriver();

      if (  Message.equalsIgnoreCase("AZ") )            // these 3 if just update the LCD
      {                                                 // TRACE ON OPEN BRACE {stringtosend.substring(0, 7)}
        lcdprint(0, 0, "Sent AZ, Received   ");
		lcdprint(0, 1, "                    ");
        lcdprint(8, 1, stringtosend.substring(0, 7));                // the current azimuth is returned from the encoder
      }


      Message = "";

    }  //endif received AZ


    if (ReceivedData.equalsIgnoreCase("OS"))           // open shutter
    {

      radio.openWritingPipe(Shutter_address);
      theCommand[0] = 'O';                           // note single quote use
      theCommand[1] = 'S';
      theCommand[2] = '#';
      SendTheCommand();
      lcdprint(0, 0, "Sent Open Shutter   ");
      lcdprint(0, 1, "                    ");
    }


    if (ReceivedData.equalsIgnoreCase("CS"))           // close shutter
    {

      radio.openWritingPipe(Shutter_address);
      theCommand[0] = 'C';                           // note single quote use
      theCommand[1] = 'S';
      theCommand[2] = '#';
      SendTheCommand();
      lcdprint(0, 0, "Sent Close Shutter  ");
	  lcdprint(0, 1, "                    ");
    }


    if (ReceivedData.equalsIgnoreCase("SS"))           // shutter status
    {

      radio.openWritingPipe(Shutter_address);
      theCommand[0] = 'S';                           // note single quote use
      theCommand[1] = 'S';
      theCommand[2] = '#';
      SendTheCommand();
      ReceiveTheResponse();
      TransmitToDriver();
      lcdprint(0, 0, "Sent Status ?:      ");
	  lcdprint(0, 1, "                    ");
	  lcdprint(0, 1, "Received: ");
      lcdprint(10, 1, stringtosend.substring(0, 7));

    }// end if receiveddata.startswith

  } //end if serial available

} //end void loop


void initialisethecommand_to_null()
{
  for ( int i = 0; i < 32; i++)                          // initialise the command array back to nulls
  {
    theCommand[i] = 0;
  }
} // end void initialisethecommand_to_null

void SendTheCommand()
{


  radio.stopListening();
  tx_sent = radio.write(&theCommand, sizeof(theCommand));

  radio.startListening();                            // put after the radio write so that no delay to start listening for response


  ReceivedData = "";


}

void ReceiveTheResponse()
{
  if (tx_sent)
  {
    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long txIntervalMillis = 300;            // wait 0.3 seconds to see if a response will be received

    prevMillis = millis();
    while (!radio.available())
    {
      /*
        this while loop was originally empty and just waited for radio to become available. However a bug occurred which meant
        this section of code was introduced as a bug fix.
        The bug was: if az was sent twice in a row followed by ss, the program was locked in this (empty as was ) loop.
        apparently there was no radio available, even though the shutter arduino code reported to its serial monitor that a response had been sent. It was very perplexing.
        it can be seen that this code is designed to resend the command previously sent if there is no response within 0.5 seconds.
      */

      currentMillis = millis();

      if (currentMillis - prevMillis > txIntervalMillis)
      {
        SendTheCommand();
        prevMillis = currentMillis ; //reset the timer
      }


    }  // end while

    if (radio.available())  // nrf radio is only available if the complete TX was successful - datasheet
    {
      radio.read(&response, sizeof(response));
      //radio.stopListening();

      stringtosend = String(response) + '#';               // convert char to string for sending to driver
    }
  }
  else
  {
    stringtosend = "";
	lcdprint(0,0, "Transmission Failure");
	lcdprint(0,1, "                    ");
	lcdprint(0,2, "                    ");
	lcdprint(0,3, "                    ");
    // write this to LCD Serial.println("[-] The transmission to the selected node failed.");
    // need to think about what to do if tx fails
  }



} // end void receivetheresponse

void TransmitToDriver()
{

  Serial.println (stringtosend);              // print value to serial, for the driver
  // the string terminator # is already part of the string received from encoder


}   // end void transmit to driver

void lcdprint(int col, int row, String mess)
{
  //lcd.clear();
  lcd.setCursor(col, row);
  lcd.print(mess);

}
