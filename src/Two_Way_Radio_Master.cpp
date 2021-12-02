
//this is the Bluetooth-Version-21 v3 branch created 2/12/21


// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to

#include "Two_Way_Radio_Master.h"

const int rs = 27, en = 26, d4 = 25, d5 = 24, d6 = 23, d7 = 22;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


String ReceivedData  = "";
String Message       = "";
String stringtosend  = "";
String blank         = "                    ";
String pkversion     = "4.0";

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

  Serial.begin(19200);                       // this module uses the serial channel to Tx/ Rx commands from the Dome driver
    
  lcdprint(0, 0, "MCU version" + pkversion );
  delay(2000);

}  // end setup


void loop()
{
  
  

  if (Serial.available() > 0)                      // the dome driver has sent a command
  {

    ReceivedData = Serial.readStringUntil('#');   // the string does not contain the # character

    //ES

    if (ReceivedData.indexOf("ES", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {
      Message = "ES";                            // need to keep this as receivedData is initialised in calls below
      
      theCommand[0] = 'E';                             // note single quote use
      theCommand[1] = 'S';
      theCommand[2] = '#';

      
      ReceiveTheResponse();

      //   update the LCD

      lcdprint(0, 0, "Sent Emergency Stop ");

      lcdprint(8, 1, stringtosend.substring(0, 7));



      Message = "";

    }  //endif received ES


    //end ES

   


    if (ReceivedData.indexOf("OS", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {

// BT.print ("OS#");
      
      lcdprint(0, 2, blank);
      lcdprint(0, 2, "Sent Open Shutter   ");
      
    }


    if (ReceivedData.indexOf("CS", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {
//BT.print ("CS#");

      lcdprint(0, 2, "Sent Close Shutter  ");
      // lcdprint(0, 1, blank);
    }


    if (ReceivedData.indexOf("SS", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {

      //BT.print ("SS#");

    }// end if receiveddata

  } //end if serial available

} //end void loop





void ReceiveTheResponse()    // this routine reads the radio for an expected transmission
{


  

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


bool validate_the_response(String receipt)
{

  
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

                                               //end void AZaction










