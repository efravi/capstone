/* 
 * Project Capstone
 * Author: Efrain Villa
 * Date: 4.4.24
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"

// Include Neopixel library and colors
#include "neopixel.h"
#include "Color.h"
#include "IoTClassroom_CNM.h"

// Include OLED library
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

// function for WiFi connection
void getWiFi();

// Variables for mic

int digitalMicInt = D7;   // KY-037 digital interface
int analogMicInt = A0;   // KY-037 analog interface
int micLED = A5;      // LED pin
int digitalVal;       // digital readings
int analogVal;        // analog readings

// Variables and object for neopixel
const int PIXELCOUNT = 15;
int PIXNUM;
int rainseg;

Adafruit_NeoPixel pixel (PIXELCOUNT, SPI1, WS2812B);

// Variables and object for OLED
const int OLED_RESET=-1;
Adafruit_SSD1306 display(OLED_RESET);

//Variables for Buttons and objects
Button REDBUTTON(D6), GREENBUTTON(D5);

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(SEMI_AUTOMATIC);


void setup() {

  Serial.begin(9600);
  waitFor(Serial.isConnected,10000);
  delay(1000);

// WiFi credentials
  WiFi.clearCredentials();
  Serial.printf("Has Credentials = %i\n\n",WiFi.hasCredentials());

  delay(1000);

  WiFi.setCredentials("DDCIOT","ddcIOT2020");

  WiFi.on();
  WiFi.connect();
    while(WiFi.connecting()) {
    Serial.printf(".");
    delay(100);
    }
  Serial.printf("\n\n");
  delay(3000);

//Neopixel initialize
  pixel.begin();
  pixel.setBrightness (35);
  pixel.show();

//Mic initialize
  pinMode(digitalMicInt,INPUT); 
  pinMode(analogMicInt, INPUT);
  pinMode(micLED,OUTPUT);      
  Serial.begin(9600);

//OLED initialize
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.display(); // show splashscreen
  delay(2000);
  display.clearDisplay();   // clears the screen and buffer
}


void loop() {

// Mic read the digital interface
    digitalVal = digitalRead(digitalMicInt); 
  
    if(digitalVal == HIGH) 
    {
      digitalWrite(micLED, HIGH); // Turn ON test LED
    }
    else
    {
      digitalWrite(micLED, LOW);  // Turn OFF test LED
    }

    // Read analog interface
    analogVal = analogRead(analogMicInt);
    // Print analog value to serial
    Serial.println(analogVal); 

//Neopixel, colors, buttons
 
  if(REDBUTTON.isClicked()){
    for (PIXNUM = 0; PIXNUM < 16; PIXNUM ++){
      pixel.setPixelColor (PIXNUM, red);
      pixel.show();
      pixel.clear();
    }
  }
  if(GREENBUTTON.isClicked()){
    for (PIXNUM = 0; PIXNUM < 16; PIXNUM ++){
      pixel.setPixelColor (PIXNUM, green);
      pixel.show();
      pixel.clear();
    }
  }
}