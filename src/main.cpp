#include <Arduino.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <SPI.h>
#include "PubSubClient.h" //MQTT
#include "ArduinoJson.h"  //MQTT
#include "TFT_eSPI.h" 
#include "MyFonts.h"
#include "credentials.h"
#include "DFRobot_GM60.h"
//GM61 manual:
//https://www.dropbox.com/sh/buysgr2aeutuub3/AADsvupiGseFm-6IowMBWGCOa

#define BUZZER_PIN  12
#define GM61_RX_PIN 27
#define GM61_TX_PIN 26

#define BLUE_LIGHT    (0x01)
#define GREEN_LIGHT   (0x02)
#define RED_LIGHT     (0x04)
#define PINK_LIGHT    (0x05)
#define AMBER_LIGHT   (0x06)
#define LIGHT_OFF     (0x00)

#define BUZZER_CHANNEL 0

WiFiClient espClient;
PubSubClient client(espClient);
DFRobot_GM60_UART  gm61;
HardwareSerial HWserial1(1);
TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library

uint8_t changeColor(uint8_t function, uint8_t startColor, uint8_t endColor, uint8_t times)
{
  uint8_t packet[] = {0x7E,0x00,0x0E,0x04,0x00,0x00,function,startColor,endColor,times,0xAB,0xCD};
  long time1 = millis();
  //Serial.print("Change color result: ");
  for(uint8_t i = 0 ; i < 12 ; i++)
  {
    HWserial1.write(packet[i]);
  }
  while(true)
  {
    delay(20);
    if(HWserial1.read() == 0x02)
    {
      //Serial.print("02 ");
      for(uint8_t i = 0 ; i < 6 ;i++)
      {
        packet[i] = HWserial1.read();
        //Serial.print(packet[i],HEX);
        //Serial.print(" ");
      }
      //Serial.println(" ");
      break;
    }
    if((millis() - time1) > 2000)
    {
      return 1;
    }
  }
  if(packet[0] == 0x02)
  {
     return 0x02;
  }
  else
  {
     return 0;
  }
}

uint8_t lightControlOn()
{
  uint8_t packet[] = {0x7E,0x00,0x08,0x01,0x00,0xD3,0x04,0xAB,0xCD};
  long time1 = millis();
  //Serial.print("Light Control On: ");
  for(uint8_t i = 0 ; i < 9 ; i++)
  {
    HWserial1.write(packet[i]);
  }
  while(true)
  {
    delay(20);
    if(HWserial1.read() == 0x02)
    {
      //Serial.print("02 ");
      for(uint8_t i = 0 ; i < 6 ;i++)
      {
        packet[i] = HWserial1.read();
        //Serial.print(packet[i],HEX);
        //Serial.print(" ");
      }
      //Serial.println(" ");
      break;
    }
    if((millis() - time1) > 2000)
    {
      return 1;
    }
  }
  if(packet[0] == 0x02)
  {
     return 0x02;
  }
  else
  {
     return 0;
  }
}

void toneOK()
{
  //Thanks Shiul93 for the chords! ^_^
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWriteNote(BUZZER_CHANNEL, NOTE_C, 5);
  delay(500);
  ledcWriteNote(BUZZER_CHANNEL, NOTE_E, 5);
  delay(500);
  ledcWriteNote(BUZZER_CHANNEL, NOTE_G, 5);
  delay(1000);
  ledcDetachPin(BUZZER_PIN);
}

void toneFAIL()
{
  //Thanks Shiul93 for the chords! ^_^
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWriteNote(BUZZER_CHANNEL, NOTE_C, 5);
  delay(500);
  ledcWriteNote(BUZZER_CHANNEL, NOTE_Eb, 5);
  delay(500);
  ledcWriteNote(BUZZER_CHANNEL, NOTE_A, 4);
  delay(1000);
  ledcDetachPin(BUZZER_PIN);
}

void toneBEEP()
{
  //Thanks Shiul93 for the chords! ^_^
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWriteNote(BUZZER_CHANNEL, NOTE_B, 6);
  delay(300);
  ledcDetachPin(BUZZER_PIN);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print(" MQTT[");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  tft.fillRoundRect(0, 55, 240, 30, 1, TFT_BLACK);
  tft.setCursor(30, 80);
  tft.println("Acceso válido!");
  changeColor(3,GREEN_LIGHT,GREEN_LIGHT,0);
  toneOK();
  delay(1000);
  changeColor(3,BLUE_LIGHT,BLUE_LIGHT,0);
  tft.fillRoundRect(0, 55, 240, 30, 1, TFT_BLACK);
  tft.setCursor(30, 80);
  tft.println("Escanea QR!");
}

void reconnect(){
  while (!client.connected()) // Loop until we're reconnected
  {
    Serial.print("-Connecting MQTT server... ");
    String clientId = "ESP32Client-"; // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), "user", "ppp"))
    {
      Serial.println("OK!");
      client.subscribe("#");
      changeColor(3,GREEN_LIGHT,GREEN_LIGHT,0);
      yield();
      delay(1000);
      changeColor(3,BLUE_LIGHT,BLUE_LIGHT,0);
    }
    else
    {
      Serial.print("FAIL!, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      yield();
      delay(5000); // Wait 5 seconds before retrying
    }
  }
}

void setup(){
  HWserial1.begin(9600, SERIAL_8N1, GM61_TX_PIN, GM61_RX_PIN); //QR reader
  Serial.begin(115200); //debugging
  
  gm61.begin(HWserial1);//Init QR reader
  // gm61.reset(); //Restore to factory settings
  gm61.encode(gm61.eUTF8); //Data encoding mode
  gm61.setupCode(false,true); //Disable setting code by QR, Output the set code content
  gm61.setIdentify(gm61.eEnableAllBarcode); // Enable all QR code recognition
  lightControlOn(); //Activate user light control
  changeColor(3,PINK_LIGHT,PINK_LIGHT,0);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setFreeFont(&arial14);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(0, 0);
  tft.setTextDatum(TL_DATUM);
  tft.setTextWrap(true,true);
  tft.setSwapBytes(true);
  
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(22, 22);
  tft.println("B r i c o P a s s");

  Serial.println("///////////////////////");
  Serial.println("// B r i c o P a s s //");
  Serial.println("///////////////////////");

  WiFi.begin(ssid, password);
  tft.setCursor(20, 80);
  tft.print("Conectando...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.print("\r/Connecting to ");
    Serial.print(ssid);
    Serial.print("...");
    delay(250);
    Serial.print("\r\\Connecting to ");
    Serial.print(ssid);
    Serial.print("...");
  }
  Serial.print("\r-Connecting to ");
  Serial.print(ssid);
  Serial.println(" OK!");

  Serial.print("-IP address: ");
  Serial.println(WiFi.localIP());
  changeColor(3,GREEN_LIGHT,GREEN_LIGHT,0);
  delay(500);

  changeColor(3,PINK_LIGHT,PINK_LIGHT,0);
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);
  reconnect(); //Connect First time to MQTT server
  Serial.println("-BRICOPASS ready to scan!");
  tft.print("OK!");
  delay(500);
  tft.fillRoundRect(0, 55, 240, 30, 1, TFT_BLACK);
  tft.setCursor(30, 80);
  tft.println("Escanea QR!");
}

void loop(){
  if (!client.connected()) //Reconnect MQTT if disconnected
  {
    changeColor(3,RED_LIGHT,RED_LIGHT,0);
    reconnect();
  }
  client.loop(); //MQTT loop
  
  String codeRX = gm61.detection(); //Read scanned QR code
  if (codeRX != "null")
  {
    toneBEEP();
    client.publish("reader1/qr", (char*) codeRX.c_str());
    Serial.print(" Publishing: ");
    Serial.print(codeRX);
    Serial.println(" OK!");
    tft.fillRoundRect(0, 55, 240, 30, 1, TFT_BLACK);
    tft.setCursor(30, 80);
    tft.println("Validando QR...");
    changeColor(3,PINK_LIGHT,PINK_LIGHT,0);
  }
}