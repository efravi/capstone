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
#include "Colors.h"
#include "IoTClassroom_CNM.h"

// Include animation and OLED library
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "sleepanimation.h"
#include "cowreading.h"
#include "cowoverhead.h"
#include "raisedhand.h"

// Include Adafruit Dashboard library
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_SPARK.h"
#include "credentials.h"

IoTTimer redTimer, greenTimer, micTimer;

// function for WiFi connection
void getWiFi();

// Objects and Variables for mic
int digitalMicInt = D7;   // KY-037 digital interface
int analogMicInt = A0;   // KY-037 analog interface
int micLED = A5;      // LED pin
int digitalVal;       // digital readings
int analogVal;        // analog readings

// Variables, functions and object for neopixels
const int PIXELCOUNT = 15;
int PIXNUM;
int rainseg;
int lastSecond;
int currentTime;
int color;
bool greenNeoActive = false;
bool redNeoActive = false;
byte buf [6];

void pixelFill(int startPixel, int endPixel, int hexColor);

Adafruit_NeoPixel pixel (PIXELCOUNT, SPI1, WS2812B);

// Variables and object for OLED
const int OLED_RESET=-1;
Adafruit_SSD1306 display(OLED_RESET);

//Objects and Variables for Buttons
Button REDBUTTON(D6), GREENBUTTON(D5);

//Variable, Objects, Feeds for MQTT-Cloud

TCPClient TheClient; 

Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY); 

int subValue,subValue2,subValue3, subValue4;

// Setup Feeds to publish or subscribe to Adafruit
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname> 
Adafruit_MQTT_Subscribe neoPixel = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/green-light-button");
Adafruit_MQTT_Subscribe neoPixel2 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/red-light-button");
Adafruit_MQTT_Subscribe neoPixel3 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/rainbow-button");
Adafruit_MQTT_Subscribe neoPixel4 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/read-button");
Adafruit_MQTT_Publish pubFeedR = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/rbuttonpress");
Adafruit_MQTT_Publish pubFeedG = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/gbuttonpress");
Adafruit_MQTT_Subscribe mqttColor = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/colorpicker");

void MQTT_connect();
bool MQTT_ping();

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
    while(WiFi.connecting()){
    Serial.printf(".");
    delay(100);
    }
  Serial.printf("\n\n");
  delay(3000);

//Neopixel initialize
  pixel.begin();
  pixel.setBrightness (150);
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

  //This is to initiate subscribing to Adafruit dashboard
  mqtt.subscribe(&neoPixel);
  mqtt.subscribe(&neoPixel2);
  mqtt.subscribe(&neoPixel3);
  mqtt.subscribe(&neoPixel4);
  mqtt.subscribe(&mqttColor);

}

void loop() {

// Adafruit connect and ping
    MQTT_connect();
    MQTT_ping();
    mqtt.Update();

// Mic read the digital interface
    digitalVal = digitalRead(digitalMicInt); 
  
    if(digitalVal == HIGH){
    digitalWrite(micLED, HIGH); // Turn ON test LED
    micTimer.startTimer(2000);
    }
    if(micTimer.isTimerReady()){
      digitalWrite(micLED, LOW);
    }

    // Read analog interface
    analogVal = analogRead(analogMicInt);
    // Print analog value to serial
    Serial.println(analogVal); 

//Neopixel, colors, buttons

  if(REDBUTTON.isClicked()){
    for (PIXNUM = 0; PIXNUM < 16; PIXNUM ++){
      pixel.setPixelColor (PIXNUM, red);
    }
    pixel.show();
    pubFeedR.publish(1);
    redTimer.startTimer(2000);
    redNeoActive = true;
  }

if (redNeoActive && redTimer.isTimerReady()) {
    pubFeedR.publish(0);
    for (PIXNUM = 0; PIXNUM < 16; PIXNUM++) {
      pixel.setPixelColor(PIXNUM, 0);
    }
    pixel.show();
    redNeoActive = false;
  }

  if(GREENBUTTON.isClicked()){
    for (PIXNUM = 0; PIXNUM < 16; PIXNUM ++){
    pixel.setPixelColor (PIXNUM, green);
    }
    pixel.show();

    pubFeedG.publish(1);
    greenTimer.startTimer(2000);
    greenNeoActive = true;
  }
  if (greenNeoActive && greenTimer.isTimerReady()) {
    pubFeedG.publish(0);

    for (PIXNUM = 0; PIXNUM < 16; PIXNUM++) {
      pixel.setPixelColor(PIXNUM, 0);
    }
    pixel.show();
    greenNeoActive = false;
  }

  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(100))) {
    if (subscription == &neoPixel) {
      subValue = atof((char *)neoPixel.lastread);
      Serial.printf ("%f\n", subValue);
      if(subValue == 1){
        for (PIXNUM = 0; PIXNUM < 16; PIXNUM ++){
          pixel.setPixelColor (PIXNUM, green);
          pixel.show();
        }
        display.drawBitmap(0, 0,  bitmap_v65em, 128, 64, 1);
        display.display();
        display.clearDisplay();
      }
      else{
        for (PIXNUM = 0; PIXNUM < 16; PIXNUM ++){
          pixel.setPixelColor (PIXNUM, black);
          pixel.show();

          display.display();
          display.clearDisplay();
        }
      }
    }
    if (subscription == &neoPixel2) {
      subValue2 = atof((char *)neoPixel2.lastread);
      Serial.printf ("%f\n", subValue2);
      if(subValue2 == 1){
        for (PIXNUM = 0; PIXNUM < 16; PIXNUM ++){
          pixel.setPixelColor (PIXNUM, red);
          pixel.show();
        }
        display.drawBitmap(0, 0,  bitmap_21xftc, 128, 64, 1);
        display.display();
        display.clearDisplay();
      }
      else{
        for (PIXNUM = 0; PIXNUM < 16; PIXNUM ++){
          pixel.setPixelColor (PIXNUM, black);
          pixel.show();

          display.display();
          display.clearDisplay();
        }
      }
    }
    if (subscription == &neoPixel3) {
      subValue3 = atof((char *)neoPixel3.lastread);
      Serial.printf ("%f\n", subValue3);
      if(subValue3 == 1){
        pixel.setPixelColor (0, red);
        for(int c = 0; c < 7; c++){
          int raincolor = rainbow[c];
          int rainstart = (c * 2)+1;
          int rainend = rainstart+2;
          pixelFill (rainstart, rainend, raincolor);
          pixel.setPixelColor (PIXNUM, raincolor);
          pixel.show();
        }
          // OLED functions and animations
          display.drawBitmap(0, 0,  bitmap_mvofoe, 128, 64, 1);
          display.display();
          display.clearDisplay();
          display.drawBitmap(0, 0,  bitmap_e209b, 128, 64, 1);
          display.display();
          display.clearDisplay();
          display.drawBitmap(0, 0,  bitmap_6iiz6q, 128, 64, 1);
          display.display();
          display.clearDisplay();
      }
      else{
        for (PIXNUM = 0; PIXNUM < 16; PIXNUM ++){
          pixel.setPixelColor (PIXNUM, black);
          pixel.show();

          display.display();
          display.clearDisplay();
        }
      }
    }
    if (subscription == &neoPixel4) {
      subValue4 = atof((char *)neoPixel4.lastread);
      Serial.printf ("%f\n", subValue4);
      if(subValue4 == 1){
        for (PIXNUM = 0; PIXNUM < 16; PIXNUM ++){
          pixel.setPixelColor (PIXNUM, blue);
          pixel.show();
        }
        display.drawBitmap(0, 0, bitmap_oj0ngf, 128, 64, 1);
        display.display();
        display.clearDisplay();
      }
      else{
        for (PIXNUM = 0; PIXNUM < 16; PIXNUM ++){
          pixel.setPixelColor (PIXNUM, black);
          pixel.show();

          display.display();
          display.clearDisplay();
        }
      }
    } 
    // if (subscription == &mqttColor) {
    //   Serial.printf ("Received from Adafruit : %s \n", (char *)mqttColor.lastread);
    //   memcpy (buf, &mqttColor.lastread[1], 6); // strip off the ’#’
    //   Serial.printf ("Buffer: %s \n", (char *)buf);
    //   color = strtol((char *)buf,NULL,16); // convert string to int (hex)
    //   Serial.printf ("Buffer: 0x%02X \n",color);
    //   }
  
    // for (PIXNUM = 0; PIXNUM < 17; PIXNUM ++){
    //   pixel.setPixelColor (PIXNUM, color);
    //   pixel.show();
    // }  
  }
}
//FunctionS that connects to Adafruit
void MQTT_connect() {
  int8_t ret;
 
  // Return if already connected.
  if (mqtt.connected()) {
    return;
  }
 
  Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.printf("Error Code %s\n",mqtt.connectErrorString(ret));
       Serial.printf("Retrying MQTT connection in 5 seconds...\n");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds and try again
  }
  Serial.printf("MQTT Connected!\n");
}

bool MQTT_ping() {
  static unsigned int last;
  bool pingStatus;

  if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n");
      pingStatus = mqtt.ping();
      if(!pingStatus) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = millis();
  }
  return pingStatus;
}

void pixelFill(int startPixel, int endPixel, int hexColor){
  for(int i =startPixel;i<=endPixel;i++){
    pixel.setPixelColor(i, hexColor);
    pixel.show();
    delay(100);
  }
}

