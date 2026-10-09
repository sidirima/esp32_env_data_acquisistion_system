#include <Adafruit_ADS1X15.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include <LoRa.h>
#include <SPI.h>
#include <esp_sleep.h>

#define ENABLE 27   // D27 PIN AS ENABLE
#define NORTH 34    // D34 PIN AS NORTH WIND SENSOR
#define EAST 35     // D35 PIN AS EAST WIND SENSOR
#define WEST 32     // D32 PIN AS WEST WIND SENSOR
#define SOUTH 33    // D33 PIN AS SOUTH WIND SENSOR
#define RAIN 25     // D25 PIN AS RAIN SENSOR
#define WIND_SP 26  // D26 PIN AS WIND SPEED SENSOR
#define ss 5        // D5 PIN AS LORA SS PIN
#define rst 14      // D14 PIN AS LORA RST PIN
#define dio0 2      // D2 PIN AS LORA DI0 PIN

#define uS_TO_S_FACTOR 1000000  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 7200 // Time for ESP to go to sleep (in seconds)

Adafruit_ADS1015 ads; // I2C for ADC (0x48)
Adafruit_BME280 bme; // I2C for BME280 (0x77)

float  soil_temp_volts, soil_moist_volts; // Volts counted on ADC pins variables
float soil_temp = 0; // Final Soil Temp variable
float soil_moist = 0; // Final Soil Moisture variable

float air_temp = 0; // Air Temperature Reading variable
float air_humid = 0; // Air Humidity Reading variable
float air_press = 0; // Air Pressure Reading variable

float revolutions = 0; // Wind Speed revolutions variable
float windSpeed = 0; // Wind Speed Reading variable

const float mmPerPulse = 0.173; // Value of rain in mm for each bucket movement
float mmTotal = 0; // Rain Depth Reading variable
float pulses = 0; // Number of rain sensor movement variable

int packet_num = 0; // Packet counter for LoRa transmission

char soilTempString[10]; // Soil Temperature float to String variable
char soilMoistString[10]; // Soil Moisture float to String variable
char airTempString[10]; // Air Temperature float to String variable
char airHumidString[10]; // Air Humidity float to String variable
char airPressString[10]; // Air Pressure float to String variable
char rainDepthString[10]; // Rain Depth float to String variable
char windSpeedString[10]; // Wind speed float to String variable
char windDirection[10]; // Wind direction String variable
char n_name[10] = "node_test"; // Node name String variable
char s_id[10] = "007"; // Station id String variable
char s_name[10] = "stat1"; 

char loraMegaString[130];  // The combined string sent via LoRa 

void setup() {
  pinMode(ENABLE, OUTPUT);  // 5V Enable Pin
  pinMode(NORTH, INPUT);    // North Wind Sensor Input
  pinMode(EAST, INPUT);     // East Wind Sensor Input
  pinMode(WEST, INPUT);     // West Wind Sensor Input
  pinMode(SOUTH, INPUT);    // South Wind Sensor Input
  pinMode(RAIN, INPUT);     // Rain Sensor Input
  pinMode(WIND_SP, INPUT);  // Wind Speed Sensor Input
  Serial.begin(115200);
  
/*If BME280 fails to initialize*/
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  else Serial.println("BME280 connected succesfully!");

/*If ADS1015 fails to initialize */
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
  else Serial.println("ADS connected succesfully!");

/* LoRa Settings */
  LoRa.setPins(ss, rst, dio0);  //setup LoRa transceiver module

  while (!LoRa.begin(433E6))  //433E6 - Asia, 866E6 - Europe, 915E6 - North America
  {
    Serial.println("LoRa not initialized!");
    delay(500);
  }
  LoRa.setSyncWord(0xA5); // To synchronize transmitter and receiver modules
  Serial.println("LoRa Initializing OK!");

}

void loop() {
  digitalWrite(ENABLE, HIGH); // 5V Enable
  ads_readings(); // calls the ads_readings function
  delay(1000);
  bme_readings(); // calls the bme_readings function
  delay(1000);

  //RAIN DEPTH CALCULATION
  Serial.println("Calculating Rain Depth...");
  attachInterrupt(digitalPinToInterrupt(RAIN), rain_depth, CHANGE); // Begin Interrupt to find rain mm per minute
  delay(60000);
  detachInterrupt(RAIN); // End Interrupt 
  mmTotal = (pulses * mmPerPulse) * 60;
  dtostrf(mmTotal, 4, 2, rainDepthString); // Float rainDepth to String conversion

  Serial.print("Estimated Rain Depth: "); Serial.print(mmTotal); Serial.println("mm/hr"); // Return estimated rain depth for next hour
  pulses = 0;

  wind_direction(); // calls the windDirection function
  delay(1000);

  // WIND SPEED CALCULATION
  Serial.println("Calculating Wind Speed...");
  attachInterrupt(digitalPinToInterrupt(WIND_SP), wind_speed_function, RISING); // Begin Interrupt to find RPM of wind
  delay(60000); // Stop everything for 60s
  detachInterrupt(WIND_SP); // End Interrupt

  windSpeed = revolutions * 0.18; // calculate RPM
  dtostrf(windSpeed, 4, 2, windSpeedString); // Float windSpeed to String conversion

  Serial.print("Wind Speed: "); Serial.print(windSpeed); Serial.println(" km/h"); // Return windspeed
  revolutions = 0;

  loraMegaString_create(); // calls the loraMegaString_create function
  Serial.print("LORA MEGASTRING IS:");  Serial.println(loraMegaString); // TESTING ONLY
  lora_packet_send(); // calls the lora_packet_send function
  empty_strings();  // calls the empty_strings function
  digitalWrite(ENABLE, LOW);  // 5V Disable
  delay(5000);
  Serial.println("Πακέτο LoRa εστάλη.");
  Serial.println("Πηγαίνω σε deep sleep για 2 ώρες...");

  // Ρύθμιση χρονικού wakeup
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void wind_speed_function() {
  revolutions++;
  Serial.print(".");
}

void wind_direction() {
  // WIND DIRECTION SENSOR

  if (digitalRead(NORTH) == LOW && digitalRead(EAST) == HIGH && digitalRead(WEST)== HIGH && digitalRead(SOUTH) == HIGH) {
    strcpy(windDirection,"NORTH");
  }
  
  else if (digitalRead(NORTH) == LOW && digitalRead(EAST) == LOW && digitalRead(WEST) == HIGH && digitalRead(SOUTH) == HIGH) {
    strcpy(windDirection,"NORTHEAST");
  }

  else if (digitalRead(NORTH) == LOW && digitalRead(EAST) == HIGH && digitalRead(WEST) == LOW && digitalRead(SOUTH) == HIGH) {
    strcpy(windDirection,"NORTHWEST");
  }

  else if (digitalRead(EAST) == LOW && digitalRead(NORTH) == HIGH && digitalRead(WEST) == HIGH && digitalRead(SOUTH) == HIGH) {
    strcpy(windDirection,"EAST");
  }

  else if (digitalRead(SOUTH) == LOW && digitalRead(EAST) == LOW && digitalRead(WEST) == HIGH && digitalRead(SOUTH) == HIGH) {
    strcpy(windDirection,"SOUTHEAST");
  }

  else if (digitalRead(SOUTH) == LOW && digitalRead(EAST) == HIGH && digitalRead(WEST) == HIGH && digitalRead(NORTH) == HIGH) {
    strcpy(windDirection,"SOUTH");
  }

  else if (digitalRead(SOUTH) == LOW && digitalRead(EAST) == HIGH && digitalRead(WEST) == LOW && digitalRead(NORTH) == HIGH) {
    strcpy(windDirection,"SOUTHWEST");
  }

  else if (digitalRead(WEST) == LOW && digitalRead(NORTH) == HIGH && digitalRead(SOUTH) == HIGH && digitalRead(NORTH) == HIGH) {
    strcpy(windDirection,"WEST");
  }
  else {strcpy(windDirection,"UNKNOWN");} // Failsafe
  Serial.print("Wind Direction: "); Serial.println(windDirection);
}

void rain_depth() {
  // RAIN DEPTH SENSOR
  pulses++;
  Serial.print(".");
}

void bme_readings() {
  air_temp = bme.readTemperature(); // reads temperature in Celsius;
  air_humid = bme.readHumidity();   // reads absolute humidity;
  air_press = (bme.readPressure() / 100.0F);   // reads pressure in hPa (hectoPascal = millibar);

  dtostrf(air_temp, 4, 2, airTempString); // Air Temperature float to String conversion
  dtostrf(air_humid, 4, 2, airHumidString); // Air Humidity float to String conversion
  dtostrf(air_press, 4, 2, airPressString); // Air Pressure float to String conversion

  Serial.print("Air Temp: "); Serial.print(air_temp); Serial.println(" *C");
  Serial.print("Humidity: "); Serial.print(air_humid); Serial.println(" %");
  Serial.print("Pressure: "); Serial.print(air_press); Serial.println(" hPa");
  Serial.println();
}

void ads_readings() {
  soil_temp_volts = ads.computeVolts(ads.readADC_SingleEnded(0)); // Read volts of ADC channel 0 
  soil_moist_volts = ads.computeVolts(ads.readADC_SingleEnded(1)); // Read volts of ADC channel 1

  Serial.print("Soil Temperature Voltage: "); Serial.print(soil_temp_volts,4); Serial.println(" V");
  Serial.print("Soil Moisture Voltage: "); Serial.print(soil_moist_volts,4); Serial.println(" V");
  
  soil_temp = -16.83369 
              + (40.60419 * soil_temp_volts) 
              + (13.4579 * pow(soil_temp_volts, 2)) 
              - (24.47073 * pow(soil_temp_volts, 3)) 
              + (7.944197 * pow(soil_temp_volts, 4));
  
  if (soil_moist_volts > 4.0) {
    Serial.println("Soil Moisture Sensor not properly placed!");
    soil_moist = -10.0; // -10%, used as a sign for not properly placing the soil_moist sensor.
  }
  else if (soil_moist_volts < 3.0) {
    Serial.println("The soil is too wet to calculate!");
    soil_moist = 100.0; // 100%, used as a sign for too wet soil to calculate
  }
  else {
    soil_moist = + 10372.697
                 - (8521.883 * pow(soil_moist_volts, 1))
                 + (82335.220 * pow(soil_moist_volts, 2))
                 - (213.286 * pow(soil_moist_volts, 3));
  }
  
  Serial.print("Soil Temperature: "); Serial.print(soil_temp); Serial.println(" *C");
  Serial.print("Soil Moisture: "); Serial.print(soil_moist); Serial.println(" %");
  dtostrf(soil_temp, 4, 2, soilTempString); // Soil Temperature float to String conversion
  dtostrf(soil_moist, 4, 2, soilMoistString); // Soil Moisture float to String conversion
}

void loraMegaString_create() {
  strcpy(loraMegaString, soilTempString); strcat(loraMegaString, ";");  // loraMegaString = {soilTempString;}
  strcat(loraMegaString, soilMoistString); strcat(loraMegaString, ";"); // loraMegaString = {soilTempString;soilMoistString;}
  strcat(loraMegaString, airTempString); strcat(loraMegaString, ";");   // loraMegaString = {soilTempString;soilMoistString;airTempString;}
  strcat(loraMegaString, airHumidString); strcat(loraMegaString, ";");  // loraMegaString = {soilTempString;soilMoistString;airTempString;airHumidString;}
  strcat(loraMegaString, airPressString); strcat(loraMegaString, ";");  // loraMegaString = {soilTempString;soilMoistString;airTempString;airHumidString;airPressString;}
  strcat(loraMegaString, rainDepthString); strcat(loraMegaString, ";"); // loraMegaString = {soilTempString;soilMoistString;airTempString;airHumidString;airPressString;rainDepthString;}
  strcat(loraMegaString, windSpeedString); strcat(loraMegaString, ";"); // loraMegaString = {soilTempString;soilMoistString;airTempString;airHumidString;airPressString;rainDepthString;windSpeedString;}
  strcat(loraMegaString, windDirection); strcat(loraMegaString, ";");   // loraMegaString = {soilTempString;soilMoistString;airTempString;airHumidString;airPressString;rainDepthString;windSpeedString;windDirection;}
  strcat(loraMegaString, n_name); strcat(loraMegaString, ";");          // loraMegaString = {soilTempString;soilMoistString;airTempString;airHumidString;airPressString;rainDepthString;windSpeedString;windDirection;n_name;}
  strcat(loraMegaString, s_id); strcat(loraMegaString, ";");
  strcat(loraMegaString, s_name); 
}

void lora_packet_send() {
  Serial.print("Sending packet: ");
  Serial.println(packet_num);
 
  LoRa.beginPacket();   //Send LoRa packet to receiver
  LoRa.print(loraMegaString); 
  LoRa.print(packet_num);
  LoRa.endPacket();
  memset(loraMegaString, 0, sizeof loraMegaString); // Empty the loraMegaString array
  
  packet_num++;
 
  delay(10000);
}

void empty_strings() {
  memset(soilTempString,  0, sizeof soilTempString);    // Resets the soilTempString
  memset(soilMoistString, 0, sizeof soilMoistString);   // Resets the soilMoistString
  memset(airTempString,   0, sizeof airTempString);     // Resets the airTempString
  memset(airHumidString,  0, sizeof airHumidString);    // Resets the airHumidString
  memset(airPressString,  0, sizeof airPressString);    // Resets the airPressString
  memset(rainDepthString, 0, sizeof rainDepthString);   // Resets the rainDepthString
  memset(windSpeedString, 0, sizeof windSpeedString);   // Resets the windSpeedString
  memset(windDirection,   0, sizeof windDirection);     // Resets the windDirection
}
