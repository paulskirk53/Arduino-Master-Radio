
//this is the Bluetooth-Version-21 v3 branch created 2/12/21
/* The functions this program has to deliver are:

SEND functions first:
1 - Send shutter status request "SS"
2 - Send Open Shutter request   "OS"
3 - Send Close shutter request  "CS"
4 - Send emrgency stop request  "ES"

RECEIVE functions:
1 - Receive Status - can be "OPEN", "CLOSED", "opening", "closing"


*/

#include "Two_Way_Radio_Master.h"

#define ASCOM Serial
#define Bluetooth Serial2   // connect the HC05 to these Tx and Rx pins

const int rs = 27, en = 26, d4 = 25, d5 = 24, d6 = 23, d7 = 22;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


String blank         = "                    ";
String pkversion     = "4.0";


void setup()
{
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
  // Print a message to the LCD.
  lcd.setCursor(0, 0);                    //remove - reinstate these 2 commands
  lcd.print("Master Radio V " + pkversion);
  

  ASCOM.begin(9600);                       // this module uses the serial channel as ASCOM Tx/ Rx commands from the Dome driver
  Bluetooth.begin(9600) ;                   // the bluetooth HC05 devices are set as baud 9600, so it's important to match.
  lcdprint(0, 0, "MCU version" + pkversion );
  delay(2000);

}  // end setup


void loop()
{
  
  //code the send functions first

  if (ASCOM.available() > 0)                      // the dome driver has sent a command
  {

    String ASCOMReceipt = ASCOM.readStringUntil('#');   // the string does not contain the # character

    //ES

    if (ASCOMReceipt.indexOf("ES", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {
      
// TODO remove the test line below
sendViaASCOM("Emergency stop was received from KB");

      sendViaBluetooth("ES");

      //   update the LCD

      lcdprint(0, 0, "Sent Emergency Stop ");

    }  //endif received ES


     


    if (ASCOMReceipt.indexOf("OS", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {

// BT.print ("OS#");
      
      lcdprint(0, 2, blank);
      lcdprint(0, 2, "Sent Open Shutter   ");
      
      sendViaBluetooth("OS");

    }// endif OS


    if (ASCOMReceipt.indexOf("CS", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {

      lcdprint(0, 2, "Sent Close Shutter  ");
      sendViaBluetooth("CS");

    }  //endif CS


    if (ASCOMReceipt.indexOf("SS", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {

      sendViaBluetooth("SS");

    }// end if SS

  } //end if ASCOM available


// now check for receipts in response to any commands sent by the code above
// receipts arrive on Bluetooth

if ( Bluetooth.available() > 0) 
    {
      String BluetoothReceipt = Bluetooth.readStringUntil('#');   // the string does not contain the # character
      // first validate - 
      //four cases are "OPEN", "CLOSED", "opening", "closing" note case

      bool receiptOK = validate_the_response(BluetoothReceipt);

    if (receiptOK)
        {
          sendViaASCOM(BluetoothReceipt);   // if the receipt from the command processor is valid, send it through to the ASCOM driver. otherwise ignore it
        }
    }


} //end void loop


void sendViaASCOM(String textToSend)
{
  
  ASCOM.print (textToSend + '#');             // print value to ASCOM, for the driver
  


}   // end void transmit to driver

void lcdprint(int col, int row, String mess)

{
  //lcd.clear();
  lcd.setCursor(col, row);
  lcd.print(mess);

}


bool validate_the_response(String receipt)
{
  
    if ( (receipt.indexOf("OPEN", 0) > -1) | (receipt.indexOf("opening", 0) > -1) | (receipt.indexOf("CLOSED", 0) > -1) | (receipt.indexOf("closing", 0) > -1) )
    {
     
      return true;
    }
    else
    {
      return false;
    }
  

}

void sendViaBluetooth(String textToSend)
{
  // TODO remove the test line below
  ASCOM.print("Test textToSend is " + textToSend);
  Bluetooth.print(textToSend + '#');
}










