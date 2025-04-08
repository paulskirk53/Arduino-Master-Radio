
//this is the Bluetooth-Version-21 v3 branch created 2/12/21
/* The functions this program has to deliver are:

ASCOM SEND functions first:
Received from the ASCOM driver via USB connection....
1 - Send shutter status request "SS"
2 - Send Open Shutter request   "OS"
3 - Send Close shutter request  "CS"
4 - Send emrgency stop request  "ES"  .    

NON ASCOM Function:
1 - reset - via the USB ASCOM cable, but not with the ASCOM client connected. i.e. from the Monitor program
RECEIVE functions:
Received via Bluetooth from the Command Processor MCU....
1 - Receive Status - can be "open", "closed", "opening", "closing"


*/

#include <avr/wdt.h>
#include "Two_Way_Radio_Master.h"
//#include <LiquidCrystal.h>

#define ASCOM Serial
#define Bluetooth Serial1   // connect the HC05 to these Tx and Rx pins
#define ledtest 5
#define shutterStateSwitch 6            // the reason for this switch is to have user control over whether open or closed is report to the ASCOM driver in case the Bluetooth radio fails
                                        // radio failure is a dealbreaker and no imaging would be possible.
#define shutterStateSwitchMormal HIGH   // shutterStateSwitch Normal is normal operation where the software reports the shutter state returned by the Shutter MCU 
#define shutterStateSwitchOverride LOW  // shutterStateSwitch override is where the radio assumes the shutter is open and reports that to the ASCOM driver 

const int rs = 27, en = 26, d4 = 25, d5 = 24, d6 = 23, d7 = 22;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


String blank         = "                    ";
String pkversion     = "5.0";
long TMRReceivedBT   = millis(); 
int statusCount       = 0;
int BTReceiptCount   = 0;
int BTMinuteCount    = 0;

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

  sendViaBluetooth("CONNECT");         // this is used to test if BT cnnection is available. The shutter sends OK in response and this shoud appear on the master radio lcd if connection works.

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
      sendViaASCOM("shutter");         // this is an MCU identification code
      digitalWrite(ledtest, HIGH);
    }
     
    // reset this MCU  - A NON ASCOM command wich means it must be rec'd via a terminal e.g. arduino Sermon
    if (ASCOMReceipt.indexOf("reset", 0) > -1)        // note NOTE note NOTE - this command can only arrive if ASCOM releases the port and the Monitor program send 'reset#'
      {
        sendViaASCOM(" Resetting ");
        lcdprint(0, 3, "Rec'd  a reset CMD  ");
        while(1)                                      // forever loop times out the wdt and causes reset
        {}
      }

      // reset the command processor - A NON ASCOM command wich means it must be rec'd via a terminal e.g. arduino Sermon
    if (ASCOMReceipt.indexOf("cprestart", 0) > -1)        // 
      {
        sendViaASCOM(" Resetting The Command Processor ");
        lcdprint(0, 3, "Reset CP & shutter  ");
        sendViaBluetooth("reset");                      // cp is coded to expect 'reset' - not cprestart as in this program
      }

     //ES

    if (ASCOMReceipt.indexOf("ES", 0) > -1) // THE INDEX VALUE IS the value of the position of the string in receivedData, or -1 if the string not found
      {
        
        sendViaBluetooth("ES");  // sent to command processor

        //   update the LCD

        lcdprint(0, 2, "Sent Emergency Stop ");
        delay(1000);
        
        
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
      statusCount ++;
      if (statusCount > 99)
      {
        statusCount = 0;
      }

      sendViaBluetooth("SS");
      lcdprint(0, 2, blank);
      lcdprint(0, 2, "Status Request " + String(statusCount) );

      if (shutterStateSwitch == shutterStateSwitchOverride)   // if the switch on the front of the radio box is in the override position, status is overriden to 'open'
      {
       sendViaASCOM("open");  // return 'open' to the ASCOM driver
      }
    }  // end if SS

  } //end if ASCOM available


// now check for receipts in response to any commands sent by the code above
// receipts arrive on Bluetooth

if ( Bluetooth.available() > 0) 
    {
      String BluetoothReceipt = Bluetooth.readStringUntil('#');   // the string does not contain the # character
      BTReceiptCount ++;
      if (BTReceiptCount > 99)
      {
        BTReceiptCount = 0;
      }
      
      // print what's returned to the LCD row 3
      lcdprint(0, 0, blank);      // blank out the BT not received for 1 min message
      lcdprint(0, 0, "Bluetooth connected ");
      lcdprint(0, 3, blank);
      lcdprint(0, 3,  "BT Rec' " + BluetoothReceipt + " " + String(BTReceiptCount));

      //  validate what came back - 
      //four cases are "open", "closed", "opening", "closing" note case

      bool receiptOK = validate_the_response(BluetoothReceipt);

    if (receiptOK)                           // valid receipts are open, closed, opening, closing.
        {
          TMRReceivedBT = millis();          // reset the BT detection timer
          sendViaASCOM(BluetoothReceipt);   // if the receipt from the command processor is valid, send it through to the ASCOM driver
        }
    }

  if ((millis() - TMRReceivedBT)  > 60000 )  // 1 minute timer
  {
    BTMinuteCount++;
    if (BTMinuteCount >99)
    {
      BTMinuteCount=0;
    }

    lcdprint(0, 0, blank);
    lcdprint(0, 0,  " No BT in " + String(BTMinuteCount) + " minutes");
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
  
    if ( (receipt.indexOf("opening", 0) > -1) || (receipt.indexOf("open", 0) > -1) || (receipt.indexOf("closed", 0) > -1) || (receipt.indexOf("closing", 0) > -1) )
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










