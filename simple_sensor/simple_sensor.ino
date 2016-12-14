
#include "tuino_swisscom_lpn.h"

// Third Parties Sensors Libs
#include "ChainableLED.h"

#define NUM_LEDS  1

// You must specify the correct PINS based on your setup
ChainableLED leds(8, 9, NUM_LEDS);

byte colorR = 0;
byte colorG = 0;
byte colorB = 0;

long int timer_period_to_tx = 60000;
long int timer_millis_lora_tx = 0;

// This must be correct pin based on your setup
const int buttonPin = 2;   

// Sensor - Button State
int buttonState = 0;
int lastButtonState = 0;

byte toggleValue = 0;

void setup()
{
  Serial.begin(9600);
 
  pinMode(buttonPin, INPUT);  
  leds.init();
  
  // Uncomment this to suppress traces
  //LoRaWan_verbose = false;
 
  // Initialize communication with LoRaWan modem
  LoRaWan_init();

  // to DevEUI and AppsKey
  LoRaWan_read_provisioning();  
 
  // Join Network
  Join_SwisscomLPN("A");
  
  // Blink Green Led
   for (int i=0;i<6;i++) {
      colorG = ~colorG;
      leds.setColorRGB(0, colorR, colorG, colorB);
      delay(50);
   }
  
}


void loop() {

  long int delta_lora_tx;
  char lora_data[32];

  // Main Sensor Function
  // Check if we press Button and change State
  buttonState = digitalRead(buttonPin);

  if ( buttonState != lastButtonState ) {
    if ( buttonState == 1 ) {
      toggleValue = !toggleValue;
      colorR = ~colorR;
      leds.setColorRGB(0, colorR, colorG, colorB );
    }
  }
  
  lastButtonState = buttonState;
  // end sensor Function

  delta_lora_tx = millis() - timer_millis_lora_tx;
   
  // Transmit Period 
  if ( delta_lora_tx > timer_period_to_tx) {
   
   // Setup LoRaWAN Payload
   // need HEX string with leading 0
   sprintf(lora_data,"%02X",toggleValue);


   colorB = ~colorB;
   leds.setColorRGB(0, colorR, colorG, colorB);      
   // transmit
   LoRaWan_send(lora_data, 1, false);
   colorB = ~colorB;
   leds.setColorRGB(0, colorR, colorG, colorB);
   
   timer_millis_lora_tx = millis();
  }
  
}


