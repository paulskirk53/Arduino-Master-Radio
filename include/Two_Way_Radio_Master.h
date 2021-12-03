#pragma once
#include <Arduino.h>
#include <LiquidCrystal.h>


void lcdprint(int col, int row, String mess);
bool validate_the_response(String receipt);
void sendViaBluetooth(String textToSend);
void sendViaASCOM(String textToSend);




