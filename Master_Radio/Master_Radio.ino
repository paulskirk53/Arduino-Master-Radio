
//    Name:       Master_Radio.ino
//    Created:	11/11/2018 18:22:57
//    Author:     DESKTOP-OCFJAV9\Paul
// in boiler this board is UNo port 6
// working transmitter code
// works with Radio_Receiver1 as tested 13/11/18
// 

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SoftwareSerial.h>

#define PIN10  10

RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00001";
String ReceivedData  = "";
//String RemoteResponse = "";

double RemoteResponse=0.0;

void setup()
{
  //pinMode(PIN10, OUTPUT);      // this is an NRF24L01 requirement if pin 10 is not used
  Serial.begin(9600);

  //radio.setChannel(0x66); //ensure it matches the target host causes sketch to hang
  radio.begin();
  radio.enableAckPayload();

//   radio.setDataRate(RF24_1MBPS);  // set RF datarate didn't work with the + devices either


  radio.openWritingPipe(address);    // this needs to cycle through the two adresses eventually
  radio.setPALevel(RF24_PA_MIN);
  radio.setRetries(5, 10);           // time between tries and No of tries
  radio.enableDynamicPayloads();     // needs this for acknowledge to work
}

void loop()
{
  //Serial.print("serial test ");     // test used in v early stages
  if (Serial.available() > 0)
  {
   // Serial.print("Radio channel used is ");
   // Serial.println(radio.getChannel());
    ReceivedData = Serial.readStringUntil('#');
    
    /*
       there will be a series of ifs here all of which broadcast on the receiver's address
       model these on the previous compass code
       each node will receive the transmission and respond on the pipe address
    */

    if (ReceivedData.startsWith("start", 0))
    {
      char text[32] = "Sir Arthur Calling";
      bool tx_sent;
      tx_sent = radio.write(&text, sizeof(text));
      Serial.print("The text sent was ");
      Serial.println(text);
      ReceivedData = "";
     
      
     // delay(200);                         // give the remote time to respond
      if (tx_sent)
      {
        if (radio.isAckPayloadAvailable())   // tests if the remote device has acknowledged and its
                                             // acknowledge payload (i.e its response )is available
        {

          // read ack payload and copy data to the remoteresponse variable
          radio.read(&RemoteResponse, sizeof(RemoteResponse));

          Serial.println("[+] Successfully received data from node: ");
          Serial.print("The response was ");
          Serial.println(RemoteResponse);
         // Serial.print("The data rate is ");
         // Serial.println(radio.getDataRate());

        } // endif is ackpayloadavailable

        else
        {
          Serial.print("[-] The transmission to the selected node failed.");
        }

        Serial.println("--------------------------------------------------------");
      }

     


    } // end if receiveddata.startswith

  } //end if serial available
}
