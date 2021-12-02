#pragma once
#include <Arduino.h>
#include <SPI.h>

#include <RF24.h>
#include <LiquidCrystal.h>
#include <printf.h>


void ReceiveTheResponse() ;

void lcdprint(int col, int row, String mess);

bool validate_the_response(String receipt);

void SSaction();





