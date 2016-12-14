
#include "tuino_swisscom_lpn.h"

// Third Parties Sensors Libs
#include "ChainableLED.h"
#include "Seeed_BMP280.h"

#define NUM_LEDS  1

// You must specify the correct PINS based on your setup
ChainableLED leds(8, 9, NUM_LEDS);

int colorR = 0;
int colorG = 0;
int colorB = 0;

long int timer_period_to_tx = 60000;
long int timer_millis_lora_tx = 0;

float temperature;
float pressure;
float altitude;

// This must be correct pin based on your setup
const int buttonPin = 2;     

int buttonState = 0;

BMP280 bmp280;

void setup()
{
  Serial.begin(9600);
  Serial.println("Temperature Sensor");
 
  pinMode(buttonPin, INPUT);
  
  leds.init();
  
  if(!bmp280.init())
  {
    Serial.print("BMP280 Sensor not present!");
    while(1);
  }
  
  // Uncomment this to suppress traces
  // LoRaWan_verbose = false;
 
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
  byte buf[32];
  char lora_data[32];
  int temperature_int;
  
  // Main Sensor Function
  // Check if we press Button to send immediately LoRa Packet - this violates dutycyle use only for demos!
 
  buttonState = digitalRead(buttonPin);

  if ( buttonState == 1 )
  {
    timer_millis_lora_tx = 0;
  }
   

  // Read Sensor Data
  temperature = bmp280.getTemperature();
  pressure = bmp280.getPressure();
  altitude = bmp280.calcAltitude(pressure);

    
  delta_lora_tx = millis() - timer_millis_lora_tx;

  if (( delta_lora_tx > timer_period_to_tx) || (timer_millis_lora_tx == 0 )) {

   
   Serial.print("Temp: ");
   Serial.print(temperature);
   Serial.println("C"); // The unit for  Celsius because original arduino don't support speical symbols
  
   //get and print atmospheric pressure data
   Serial.print("Pressure: ");
   Serial.print(pressure);
   Serial.println("Pa");
  
   //get and print altitude data
   Serial.print("Altitude: ");
   Serial.print(altitude);
   Serial.println("m");
  
   Serial.println("\n");

   
   // Setup LoRaWAN Payload
 
   temperature_int = temperature * 100;

   buf[0]=0x01;  // packet header - temperature
   buf[1]= (temperature_int & 0xff00 ) >> 8;
   buf[2]= (temperature_int & 0x00ff );

   sprintf(lora_data,"%02X%02x%02x",buf[0],buf[1],buf[2]);

   Serial.println( lora_data );

    colorB = ~colorB;
   leds.setColorRGB(0, colorR, colorG, colorB);      
   // transmit
   LoRaWan_send(lora_data, 1, false);
   colorB = ~colorB;
   leds.setColorRGB(0, colorR, colorG, colorB);
   
   timer_millis_lora_tx = millis();
}



}


