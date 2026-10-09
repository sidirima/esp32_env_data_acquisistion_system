#include <LoRa.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <LiquidCrystal.h>
 

//lora pins
#define ss 5
#define rst 14
#define dio0 2

//lcd pins
LiquidCrystal lcd(13, 12, 32, 33, 4, 15);

//wifi define
WiFiManager wm;

//sendind data to the backend
const char* URL = "https://users.iee.ihu.gr/~iee2019074/php/post_data.php";

void setup(){
  wm.resetSettings();
  Serial.begin(115200);

  //lcd starting
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Starting..");

  //wifi start
  WiFi.mode(WIFI_STA);

  Serial.println("Starting WiFiManager...");
  showMessageTwoRows("Connect to WiFi", "ESP32_Config");

  bool connected = wm.autoConnect("ESP32_Config");

  // Restart in case WiFi initialization fails
  if (!connected) {
    Serial.println("WiFi connection failed");
    showMessageTwoRows("WiFi failed", "Restarting...");
    delay(2000);
    ESP.restart();
  }

  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  showMessageTwoRows("WiFi Connected", WiFi.localIP().toString());
  delay(2000);

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.print("WiFi Connected");
  delay(1500);

  //LoRa start
  Serial.println("LoRa Receiver");

  lcd.clear();  
  lcd.print("Starting LoRa");

  LoRa.setPins(ss, rst, dio0);

  while (!LoRa.begin(433E6)){
    Serial.println(".");
    delay(500);
  }

  LoRa.setSyncWord(0xA5);

  Serial.println("LoRa Initializing OK!");

  lcd.clear();
  lcd.print("LoRa Ready");
  delay(1500);

  lcd.clear();
  lcd.print("Waiting packet");  
}

void loop() {
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    String LoRaData = "";

    Serial.print("Received packet '");

    while (LoRa.available()) {
      LoRaData += (char)LoRa.read();
    }

    Serial.print(LoRaData);
    Serial.print("' with RSSI");
    Serial.println(LoRa.packetRssi());

    //LCD verification message
    lcd.clear();
    showMessageTwoRows("Packet received", "from station");
    delay(2000);
    lcd.clear();
    //lcd.setCursor(0 ,1);
    showMessageTwoRows("Sending data",  "to site");
    delay(2000);

    //sending data to backend via wifi
    if (WiFi.status() == WL_CONNECTED){
      WiFiClientSecure client;
      client.setInsecure();

      HTTPClient https;
      https.begin(client, URL);
      https.addHeader("Content-Type", "text/plain");

      int httpCode = https.POST(LoRaData);
    
      Serial.print("HTTP Code: ");
      Serial.println(httpCode);

      String response = https.getString();
      Serial.println(response);

      lcd.clear();

      if(httpCode >= 200 && httpCode < 300) {
        lcd.setCursor(0, 0);
        lcd.print("Upload OK");
        lcd.setCursor(0, 1);
        lcd.print("Data sent");
      }else {
        lcd.setCursor(0, 0);
        lcd.print("Upload failed");
        lcd.setCursor(0, 1);
        lcd.print("Code: ");
        lcd.print(httpCode);
      }

      https.end();
    } else {
      Serial.println("WiFi disconnected");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("WiFi error");
      lcd.setCursor(0, 1);
      lcd.print("Not sent");
    }

    delay(2500);

    lcd.clear();
    lcd.print("Waiting packet");
  }
}

// showMessageTwoRows sub-routine
void showMessageTwoRows(String line1, String line2) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(line1.substring(0, 16));

  lcd.setCursor(0, 1);
  lcd.print(line2.substring(0, 16));
}




