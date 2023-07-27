
//this is the Bluetooth-Version-21 v3 branch created 2/12/21
/* The functions this program has to deliver are:

ASCOM SEND functions first:
Received from the ASCOM driver via USB connection....
1 - Send shutter status request "SS"
2 - Send Open Shutter request   "OS"
3 - Send Close shutter request  "CS"
4 - Send emrgency stop request  "ES"      

NON ASCOM Function:
1 - reset - via the USB ASCOM cable, but not with the ASCOM client connected. i.e. from the Monitor program
RECEIVE functions:
Received via Bluetooth from the Command Processor MCU....
1 - Receive Status - can be "OPEN", "CLOSED", "opening", "closing"


*/

#include <avr/wdt.h>
#include "Two_Way_Radio_Master.h"
//#include <LiquidCrystal.h>

#define ASCOM Serial
#define Bluetooth Serial1   // connect the HC05 to these Tx and Rx pins
#define ledtest 5
const int rs = 27, en = 26, d4 = 25, d5 = 24, d6 = 23, d7 = 22;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


String blank         = "                    ";
String pkversion     = "4.0";
long TMRReceivedBT   = millis(); 

void setup()
{
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
  // Print a message to the LCD.
  lcd.setCursor(0, 0);                    //remove - reinstate these 2 commands
  lcd.print(" Master Radio Reset " );     // 20 chars
  

  ASCOM.begin(19200);                      // this module uses the serial channel as ASCOM Tx/ Rx commands from the Dome driver
  Bluetooth.begin(9600) ;                  // the bluetooth HC05 devices are set as baud 9600, so it's important to match.
  lcdprint(0, 1, " MCU version " + pkversion );
  //  delay(2000);                             // gives time to see the message on the LCD
  lcdprint(0, 3, blank);
  lcdprint(0, 3,  "Awaiting BT Receipt");

  wdt_enable(WDTO_4S);                       // Watchdog set to 4 seconds
  wdt_reset();                       //execute this command within 4 seconds to keep the timer from resetting
}  // end setup


void loop()
{
  
  //code the send functions first

  if (ASCOM.available() > 0)                          // the dome driver has sent a command
  {

    String ASCOMReceipt = ASCOM.readStringUntil('#'); // the string does not contain the # character
    // ASCOM.print(ASCOMReceipt + '#');
    //*************************************************************************
    //******** code for MCU Identity process below ****************************
    //**** Used by the ASCOM driver to identify the COM port in use. **********
    //*************************************************************************
    //*************************************************************************

    if (ASCOMReceipt.indexOf("shutter", 0) > -1)
    {
      sendViaASCOM("shutter");
      digitalWrite(ledtest, HIGH);
    }

    // reset this MCU:
    if (ASCOMReceipt.indexOf("reset", 0) > -1)        // note NOTE note NOTE - this command can only arrive if ASCOM releases the port and the Monitor program send 'reset#'
      {
        sendViaASCOM(" Resetting ");
        lcdprint(0, 3, "Rec'd  a reset CMD  ");
        while(1)                                      // forever loop times out the wdt and causes reset
        {}
      }

   // reset the command processor:
 if (ASCOMReceipt.indexOf("cprestart", 0) > -1)        // note NOTE note NOTE this command can only arrive if ASCOM releases the port and the Monitor program send 'cprestart'
      {
        sendViaASCOM(" Resetting The Command Processor ");
        lcdprint(0, 3, "Reset CP & shutter  ");
        sendViaBluetooth("reset");                      // cp is coded to expect 'reset' - not cprestart as in this program
      }

    //ES

    if (ASCOMReceipt.indexOf("ES", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {
      
     // TODO remove the test line below
      //sendViaASCOM("Emergency stop was received from KB");

      sendViaBluetooth("ES");

      //   update the LCD

      lcdprint(0, 2, "Sent Emergency Stop ");

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
      lcdprint(0, 2, blank);
      lcdprint(0, 2, "Sent Close Shutter  ");
      sendViaBluetooth("CS");

    }  //endif CS


    if (ASCOMReceipt.indexOf("SS", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
    {
      //todo remove line below
      //digitalWrite(ledtest, LOW);
      sendViaBluetooth("SS");
      lcdprint(0, 2, blank);
      lcdprint(0, 2, "Sent Shutter Status ");
      // TODO REMOVE THE LINE BELOW WHICH WAS JUST FOR DEBUG OF CONNECTION PROBLEM  IN MAY 22
      // sendViaASCOM("closed");  // todo remove this line by commenting out - it is a useful addition as it returns a 'closed' status without needing a bluetooth
      //connection to the command processor, which is useful for testing the ASCOM driver
    }  // end if SS

  } //end if ASCOM available


// now check for receipts in response to any commands sent by the code above
// receipts arrive on Bluetooth

if ( Bluetooth.available() > 0) 
    {
      String BluetoothReceipt = Bluetooth.readStringUntil('#');   // the string does not contain the # character
      
      // print what's returned to the LCD row 3
      lcdprint(0, 3, blank);
      lcdprint(0, 3,  "BT Receipt " + BluetoothReceipt);

      //  validate what came back - 
      //four cases are "OPEN", "CLOSED", "opening", "closing" note case

      bool receiptOK = validate_the_response(BluetoothReceipt);

    if (receiptOK)
        {
          TMRReceivedBT = millis();          // reset the BT detection timer
          sendViaASCOM(BluetoothReceipt);   // if the receipt from the command processor is valid, send it through to the ASCOM driver
        }
    }

  if ((millis() - TMRReceivedBT)  > 60000 )  // 1 minute timer
  {
    lcdprint(0, 0, blank);
    lcdprint(0, 0,  " No BT in 1 min ");
    TMRReceivedBT = millis();

    lcdprint(0, 3, blank);
    lcdprint(0, 3,  "Awaiting BT Receipt");
  }
 // CREATE A 2 Minute bluetooth comms receipt check and print this message  '2 minutes of no BT' 


  wdt_reset();                       //execute this command within 4 seconds to keep the timer from resetting

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
  
    if ( (receipt.indexOf("opening", 0) > -1) | (receipt.indexOf("open", 0) > -1) | (receipt.indexOf("closed", 0) > -1) | (receipt.indexOf("closing", 0) > -1) )
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
 // FOR DEBUG ONLY  ASCOM.print("Test textToSend is " + textToSend);
  Bluetooth.print(textToSend + '#');
}










