
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

long int timer_period_to_tx =60000;
long int timer_millis_lora_tx = 0;

float temperature;
float pressure;
float altitude;

float thermostat_temperature;

// This must be correct pin based on your setup
const int buttonPin = 2;   
const int relayPin = 3;   

// Button State
int buttonState = 0;
int lastButtonState = 0;

byte manual_mode = 0;
byte relay_status = 0;

BMP280 bmp280;

void setup()
{
  Serial.begin(9600);
  Serial.println("Thermostat");
 
  pinMode(buttonPin, INPUT);
  pinMode(relayPin, OUTPUT);
  
  leds.init();
  
  if(!bmp280.init())
  {
    Serial.print("BMP280 Sensor not present!");
    // error
    leds.setColorRGB(0, 255, 0, 0);
    while(1);
  }
  
  // Uncomment this to suppress traces
  // LoRaWan_verbose = false;
  
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
  byte buf[32];
 int rx_check,rx_len;
  char lora_data[32];
  int temperature_int;
  
  buttonState = digitalRead(buttonPin);

  if ( buttonState != lastButtonState ) {
    if ( buttonState == 1 ) {
      manual_mode = !manual_mode;
      relay_status = manual_mode;
    }
  }
 
  lastButtonState = buttonState;
 
  temperature = bmp280.getTemperature();
  pressure = bmp280.getPressure();
  altitude = bmp280.calcAltitude(pressure);
  // end sensor Function
  


   delta_lora_tx = millis() - timer_millis_lora_tx;

   if (( delta_lora_tx > timer_period_to_tx) || (timer_millis_lora_tx == 0 )) {
   // Send Status Beacon  
   
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
   // header 02
   // temp_hi_byte 
   // temp_low_byte
   // relay_status
   // forced_manual
   // thermostat_temp_hi
   // thermostat_temp_low
   
 
   temperature_int = temperature * 100;

   buf[0]=0x02;  // packet header - multiple data
   buf[1]= (temperature_int & 0xff00 ) >> 8;
   buf[2]= (temperature_int & 0x00ff );
   buf[3]= relay_status;
   buf[4]= manual_mode;
   
   temperature_int = thermostat_temperature * 100;
   buf[5]= (temperature_int & 0xff00 ) >> 8;
   buf[6]= (temperature_int & 0x00ff );
   
   


   sprintf(lora_data,"%02X%02x%02x%02x%02x%02x%02x",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6]);

   Serial.println( lora_data );

   colorB = ~colorB;
   leds.setColorRGB(0, colorR, colorG, colorB);      
   // transmit
   LoRaWan_send(lora_data, 1, false);
   colorB = ~colorB;
   leds.setColorRGB(0, colorR, colorG, colorB);
   
   timer_millis_lora_tx = millis(); 
  }


  // check LoRa RX
  rx_len = LoRaWan_get_rx();
 
  if (rx_len) {
  
      // here you will have the LoRaWan_last_rx with the received data
      // two commands
      // 0101 => set manual mode
      // 0100 => off manuale mode
      // 02TTTT => set thermostat temperature
      
      if (LoRaWan_last_rx[0] == 0x01 ) {

          if ( LoRaWan_last_rx[1] == 0x01 )
          {  
            manual_mode = 1;
            relay_status = 1;
          }
          else
          {
            manual_mode = 0;
            relay_status = 0; 
          }

          Serial.print("Manual Mode=>");
          Serial.println( relay_status );
      } 

      if (LoRaWan_last_rx[0] == 0x02 ) {

          temperature_int = LoRaWan_last_rx[1];
          temperature_int = temperature_int << 8;
          temperature_int = temperature_int + LoRaWan_last_rx[2];

          thermostat_temperature = (float) ( temperature_int ) / 100.0;

          Serial.print("Set Temp=");
          Serial.println( thermostat_temperature );

          // blink green for command received
          for (int i=0;i<6;i++) {
            colorG = ~colorG;
            leds.setColorRGB(0, colorR, colorG, colorB);
            delay(50);
          }
      } 

     
  }  

  // finally do the thermostat
  if ( manual_mode == 0 ) {
    if ( thermostat_temperature != 0 ) {
      if (temperature<=thermostat_temperature) {
            relay_status = 1;  
      }
      else 
      {
        relay_status = 0;
      }
      
    }
  }



  digitalWrite(relayPin, relay_status);
  colorR = ( relay_status==1) ? 255 : 0;
  
  leds.setColorRGB(0, colorR, colorG, colorB);
  

}


