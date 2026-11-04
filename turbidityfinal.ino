#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ThingSpeak.h>

// Wi-Fi details
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ThingSpeak details
unsigned long channelID = 3551139;
const char* writeAPIKey = "YOUR_THINGSPEAK_WRITE_API_KEY";

WiFiClient client;

// LCD: I2C address 0x27, 16 columns, 2 rows
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Turbidity sensor analog output
const int TURBIDITY_PIN = 26;  // GP26 / ADC0

// Voltage divider ratio
const float DIVIDER_RATIO = 2.0;

// Calibration values
int CLEAR_ADC = 2078;
int CLOUDY_ADC = 1300;

// ThingSpeak upload interval: 20 seconds
const unsigned long uploadInterval = 20000;
unsigned long lastUpload = 0;

// Connect to Wi-Fi
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < 20000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected!");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi connection failed.");
  }
}

// Read and average 10 ADC samples
int readTurbidityADC() {
  long total = 0;

  for (int i = 0; i < 10; i++) {
    total += analogRead(TURBIDITY_PIN);
    delay(10);
  }

  return total / 10;
}

void setup() {
  Serial.begin(115200);

  analogReadResolution(12);

  // I2C connections for Raspberry Pi Pico W
  Wire.setSDA(4);  // GP4
  Wire.setSCL(5);  // GP5
  Wire.begin();

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Water Turbidity");

  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  // Connect Wi-Fi and initialize ThingSpeak
  connectWiFi();
  ThingSpeak.begin(client);

  delay(1500);
  lcd.clear();

  Serial.println("Water Turbidity Monitoring Started");
}

void loop() {
  // Read sensor
  int rawADC = readTurbidityADC();

  // Calculate ADC voltage
  float adcVoltage = rawADC * (3.3 / 4095.0);

  // Estimate sensor output voltage before divider
  float sensorVoltage = adcVoltage * DIVIDER_RATIO;

  // Calculate relative turbidity index
  float turbidityIndex = 0;

  if (CLEAR_ADC != CLOUDY_ADC) {
    turbidityIndex =
      100.0 * (CLEAR_ADC - rawADC) /
      (float)(CLEAR_ADC - CLOUDY_ADC);
  }

  // Limit index to 0-100
  turbidityIndex = constrain(turbidityIndex, 0.0, 100.0);

  int indexValue = (int)(turbidityIndex + 0.5);

  // Classify water into 3 stages
  String waterStatus;

  if (indexValue < 5) {
    waterStatus = "Pure";
  }
  else if (indexValue <= 40) {
    waterStatus = "Cloudy";
  }
  else {
    waterStatus = "Muddy";
  }

  // Display water stage on LCD
  lcd.setCursor(0, 0);
  lcd.print("Water: ");
  lcd.print(waterStatus);
  lcd.print("         ");

  lcd.setCursor(0, 1);
  lcd.print("Index: ");
  lcd.print(indexValue);
  lcd.print("/100     ");

  // Print readings to Serial Monitor
  Serial.print("ADC = ");
  Serial.print(rawADC);

  Serial.print(" | ADC Voltage = ");
  Serial.print(adcVoltage, 3);
  Serial.print(" V");

  Serial.print(" | Sensor Voltage = ");
  Serial.print(sensorVoltage, 3);
  Serial.print(" V");

  Serial.print(" | Turbidity Index = ");
  Serial.print(indexValue);

  Serial.print(" | Water Status = ");
  Serial.println(waterStatus);

  // Upload to ThingSpeak every 20 seconds
  if (millis() - lastUpload >= uploadInterval) {
    lastUpload = millis();

    connectWiFi();

    if (WiFi.status() == WL_CONNECTED) {

      // Field 1: Turbidity index
      int result = ThingSpeak.writeField(
        channelID,
        1,
        indexValue,
        writeAPIKey
      );

      if (result == 200) {
        Serial.println("ThingSpeak upload successful!");
      }
      else {
        Serial.print("ThingSpeak error code: ");
        Serial.println(result);
      }
    }
  }

  delay(1000);
}
