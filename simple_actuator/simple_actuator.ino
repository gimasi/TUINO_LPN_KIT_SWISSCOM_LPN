
#include "tuino_swisscom_lpn.h"

// Third Parties Sensors Libs
#include "ChainableLED.h"

#define NUM_LEDS  1

// You must specify the correct PINS based on your setup
ChainableLED leds(8, 9, NUM_LEDS);

int colorR = 0;
int colorG = 0;
int colorB = 0;

long int timer_period_to_tx = 60000;
long int timer_millis_lora_tx = 0;

// This must be correct pin based on your setup
const int buttonPin = 2;   
const int relayPin = 3;   

// Button State
int buttonState = 0;
int lastButtonState = 0;

byte toggleValue = 0;

void setup()
{
  Serial.begin(9600);
  Serial.println("Simple Actuator");
   
  pinMode(buttonPin, INPUT);
  pinMode(relayPin, OUTPUT);
  
  leds.init();
 
  
  // Uncomment this to suppress traces
 //LoRaWan_verbose = false;
 
  // Initialize communication with LoRaWan modem
  LoRaWan_init();

  // to DevEUI and AppsKey
  LoRaWan_read_provisioning();  
 
  // Join Network
  Join_SwisscomLPN("C");

  // Blink Green Led
   for (int i=0;i<6;i++) {
      colorG = ~colorG;
      leds.setColorRGB(0, colorR, colorG, colorB);
      delay(50);
   }
 
}


void loop() {

  long int delta_lora_tx;
  int rx_check,rx_len;
  char lora_data[32];

   // Main Sensor Function
  // Check if we press Button and change State
  buttonState = digitalRead(buttonPin);

  if ( buttonState != lastButtonState ) {
    if ( buttonState == 1 ) {
      toggleValue = !toggleValue;
      colorR = ~colorR;
      leds.setColorRGB(0, colorR, colorG, colorB );
        digitalWrite(relayPin, toggleValue);
    }
  }

  lastButtonState = buttonState;
  // end sensor Function
  
  
  // LoRaWAN 
  // - first we check if we need to send data
  // - then we check if there is a payload
   
  delta_lora_tx = millis() - timer_millis_lora_tx;

  if (( delta_lora_tx > timer_period_to_tx) || (timer_millis_lora_tx == 0 )) {
   
   // Setup LoRaWAN Payload
   // need HEX string with leading 0
   sprintf(lora_data,"%02X",toggleValue);

   // transmit
   colorB = ~colorB;
   leds.setColorRGB(0, colorR, colorG, colorB);
   LoRaWan_send(lora_data, 1, false);
   colorB = ~colorB;
   leds.setColorRGB(0, colorR, colorG, colorB);
  
   timer_millis_lora_tx = millis();
  }

  // check LoRa RX
  rx_len = LoRaWan_get_rx();
 
  if (rx_len) {
  
      // here you will have the LoRaWan_last_rx with the received data
     
      if (LoRaWan_last_rx[0] == 0x01 ) {
          toggleValue=1;
          colorR=255;  
      } else {
          toggleValue=0;
          colorR=0;
      }

       leds.setColorRGB(0, colorR, colorG, colorB );
       digitalWrite(relayPin, toggleValue);
  }  
    
}


