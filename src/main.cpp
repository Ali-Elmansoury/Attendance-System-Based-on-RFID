#include <Arduino.h>

#define SS_PIN D4  //D2
#define RST_PIN D3 //D1
#define screen_SCL D1
#define screen_SDA D2
#define redled 10
#define blueled D0
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <AudioFileSourcePROGMEM.h>
#include <AudioGeneratorTalkie.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2SNoDAC.h>
#include <Servo.h>
#include <time.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <BlynkSimpleEsp8266.h>
// VIOLA sample taken from https://ccrma.stanford.edu/~jos/pasp/Sound_Examples.html
#include "viola.h"
#define BLYNK_TEMPLATE_ID "TMPLJXZV3dJ2"
#define BLYNK_DEVICE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "GUsqrJzI2TLuk9gADS8_FyKvRiGZAeN5"
#define BLYNK_PRINT Serial

const char *ssid = "";
const char *password = "";


String host = "";
char auth[] = BLYNK_AUTH_TOKEN;

String getData ;
String CardID="";




bool switch_mode;

AudioGeneratorWAV *wav;
AudioFileSourcePROGMEM *file;
AudioOutputI2SNoDAC *out;
//state cstate = parking;

LiquidCrystal_I2C lcd(0x27,16,2);

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
int statuss = 0;
Servo myservo;

bool grant_access = false;
void servo_go(){
  if (grant_access){
    myservo.write(0);
  	delay(3000);
  	myservo.write(90);
  }
  grant_access = false;
}


void a(){
  lcd.setCursor(0,0);
  lcd.print("Attendance");
if(WiFi.status() != WL_CONNECTED){
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    Serial.print("Reconnecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    Serial.println("");
    Serial.println("Connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //IP address assigned to your ESP
  }
  
  //look for new card
   if ( ! mfrc522.PICC_IsNewCardPresent()) {
  return;//got to start of loop if there is no card present
 }
 // Select one of the cards
 if ( ! mfrc522.PICC_ReadCardSerial()) {
  return;//if read card serial(0) returns 1, the uid struct contians the ID of the read card.
 }

 for (byte i = 0; i < mfrc522.uid.size; i++) {
     CardID += mfrc522.uid.uidByte[i];
}
  WiFiClient WiFiClient;
  HTTPClient http;    //Declare object of class HTTPClient
  
  //GET Data
  getData = "?CardID=" + CardID;  //Note "?" added at front
  String Link = "http://192.168.1.14/loginsystem/postdemo.php" + getData;
  
  http.begin(WiFiClient,Link);
  
  int httpCode = http.GET();            //Send the request
  delay(10);
  String payload = http.getString();    //Get the response payload
  
  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload
  Serial.println(CardID);     //Print Card ID
  
  if(payload == "login"){
    digitalWrite(blueled,HIGH);
    lcd.print("Logged In");
    delay(2000);
    lcd.clear();
    Serial.println("red on");
    delay(500);  //Post Data at every 5 seconds
  }
  else if(payload == "logout"){
    digitalWrite(redled,HIGH);
    lcd.print("Logged Out");
    delay(2000);
    lcd.clear();
    Serial.println("Blue on");
    delay(500);  //Post Data at every 5 seconds
  }
  else if(payload == "Successful" || payload == "Card Available"){
    if (payload=="Successful")
    {
      digitalWrite(blueled,HIGH);
      lcd.setCursor(0,0);
      lcd.print("Card added");
      lcd.setCursor(0,1);
      lcd.print("successfully");
      delay(2000); 
      lcd.clear();
    }
    else
    {
      digitalWrite(blueled,HIGH);
      lcd.setCursor(0,0);
      lcd.print("Card already");
      lcd.setCursor(0,1);
      lcd.print("available");
      delay(2000); 
      lcd.clear();
    } 
  }
  delay(500);
  
  CardID = "";
  getData = "";
  Link = "";
  http.end();  //Close connection
  
  digitalWrite(blueled,LOW);
  digitalWrite(redled,LOW);
}


void p(){
  lcd.setCursor(0,0);
  lcd.print("Parking");
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Show UID on serial monitor
  Serial.println();
  Serial.print(" UID tag :");
  String content= "";
  //byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println();
  lcd.setCursor(0,0);
  lcd.print("Parking");
  if (content.substring(1) == "39 F4 0E B3") //change UID of the card that you want to give access
  {
    grant_access = true;
    digitalWrite(redled,LOW);
    digitalWrite(blueled, HIGH);
    Serial.println(" Access Granted ");
    Serial.println();
      statuss = 1;
    // print message
    lcd.setCursor(0,1);
    lcd.print("Access Granted!!");
    // wav->say(viola,sizeof(viola));
    servo_go();
    
    // clears the display to print new message
    lcd.clear();
    // set cursor to first column, second row
    delay(1000);
    digitalWrite(redled,LOW);
    digitalWrite(blueled, LOW);
    lcd.clear(); 
  }
  
  else   {
    digitalWrite(redled,HIGH);
    digitalWrite(blueled, LOW);
    lcd.setCursor(0,1);
    Serial.println(" Access Denied ");
    lcd.print("Access Denied");
    delay(1000);
    digitalWrite(redled,LOW);
    digitalWrite(blueled, LOW);
    lcd.clear();
  }
}


void setup() 
{
  pinMode(redled,OUTPUT);
  pinMode(blueled, OUTPUT);
  Serial.begin(9600);   // Initiate a serial communication
  Blynk.begin(auth, ssid, password);
  delay(1000);
  lcd.init();   // Initiate MFRC522
  lcd.backlight();
  Serial.printf("WAV start\n");
  audioLogger = &Serial;
  file = new AudioFileSourcePROGMEM( viola, sizeof(viola) );
  out = new AudioOutputI2SNoDAC();
  wav = new AudioGeneratorWAV();
  wav->begin(file, out);
  myservo.attach(D8);
  //struct tm tmstruct;
  delay(1000);
  SPI.begin();  // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card

  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");
  
  
  Serial.print("Connecting to ");
  Serial.print(ssid);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.println("Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}

void loop() 
{
  Blynk.run();
  if (WiFi.status() != WL_CONNECTED){
    delay(500);

  }
  switch(switch_mode)
  {
    case true: p();
    break;

    case false: a();
    break;
  }
} 
