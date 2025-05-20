#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <SoftwareSerial.h>

// Initialize LCD (I2C address 0x27, 16 chars, 2 lines)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// DHT11 setup
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Sensors Pins
#define MQ7_PIN A0         // CO sensor
#define MQ135_PIN A1       // VOC sensor
#define BUTTON_PIN 3       // Button

// MH-Z19 setup for CO2
SoftwareSerial mySerial(10, 11); // RX, TX
const int MHZ19_ADDRESS = 0x00;  // Default address

// Variables
float temperature = 0.0;
float humidity = 0.0;
float co_ppm = 0.0;
float voc_ppm = 0.0;
float co2_ppm = 0.0;
bool buttonPressed = false;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  lcd.init();
  lcd.backlight();

  pinMode(BUTTON_PIN, INPUT_PULLUP); // Button with internal pull-up

  lcd.setCursor(0, 0);
  lcd.print("Air Quality Sys");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Read button state
  if (digitalRead(BUTTON_PIN) == LOW) {
    buttonPressed = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Calibrating...");
    delay(2000);
  }

  // Read DHT11
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    lcd.setCursor(0, 0);
    lcd.print("DHT Error");
  } else {
    // Read MQ-7 (CO)
    int mq7_value = analogRead(MQ7_PIN);
    co_ppm = mapAnalogToPPM(mq7_value, 'CO');

    // Read MQ-135 (VOC)
    int mq135_value = analogRead(MQ135_PIN);
    voc_ppm = mapAnalogToPPM(mq135_value, 'VOC');

    // Read MH-Z19 (CO2)
    co2_ppm = readCO2();

    // Display on LCD
    displayAirQuality();

    // Optional: Send data to Serial Monitor
    Serial.print("Temp: "); Serial.print(temperature); Serial.print("C ");
    Serial.print("Hum: "); Serial.print(humidity); Serial.println("%");
    Serial.print("CO: "); Serial.print(co_ppm); Serial.println(" ppm");
    Serial.print("VOC: "); Serial.print(voc_ppm); Serial.println(" ppm");
    Serial.print("CO2: "); Serial.print(co2_ppm); Serial.println(" ppm");
  }

  delay(3000); // Wait before next reading
}

// Function to map analog reading to PPM (simple calibration placeholder)
float mapAnalogToPPM(int analogValue, char sensorType) {
  // Placeholder mapping, calibration needed
  if (sensorType == 'CO') {
    return map(analogValue, 0, 1023, 0, 100); // 0-100 ppm
  } else if (sensorType == 'VOC') {
    return map(analogValue, 0, 1023, 0, 300); // 0-300 ppm
  }
  return 0;
}

// Function to read CO2 from MH-Z19
float readCO2() {
  byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  mySerial.write(cmd, 9);
  delay(100);

  if (mySerial.available()) {
    byte response[9];
    for (int i=0; i<9; i++) {
      response[i] = mySerial.read();
    }
    if (response[0] == 0xFF && response[1] == 0x86) {
      int high = response[2];
      int low = response[3];
      int ppm = (high << 8) + low;
      return ppm;
    }
  }
  return -1; // Error
}

// Function to display data on LCD
void displayAirQuality() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CO2:");
  lcd.print(co2_ppm);
  lcd.print("ppm");
  lcd.setCursor(0, 1);
  lcd.print("CO:");
  lcd.print(co_ppm);
  lcd.print(" VOC:");
  lcd.print(voc_ppm);
  lcd.print("ppm");
}
