#pragma once
#include <Arduino.h>
#include <SPI.h>

#include <RF24.h>
#include <LiquidCrystal.h>
#include <printf.h>

void initialisethecommand_to_null();
void initialisetheresponse_to_null();
void SendTheCommand();
void ReceiveTheResponse() ;
void TransmitToDriver();
void lcdprint(int col, int row, String mess);
void TransmissionFailure();
bool validate_the_response(String receipt);
void AZaction();
void SSaction();
void ConfigureRadio(String Address)       ;
void TestforlostRadioConfiguration(String NodeAddress) ;
void CheckifResponseReceived()  ;
void RadioFailureCheck() ;
void CheckforRadioAlwaysAvailableError();

