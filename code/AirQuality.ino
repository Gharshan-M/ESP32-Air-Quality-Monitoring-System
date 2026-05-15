#define BLYNK_TEMPLATE_ID "TMPL3t-qlMi0Q"
#define BLYNK_TEMPLATE_NAME "AirQuality"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_TOKEN"

#include <Wire.h>
#include <LiquidCrystal.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <DHT.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// ---------------- Pin Definitions ----------------

#define MQ7_PIN     34
#define MQ135_PIN   35
#define DHTPIN      4
#define BUZZER_PIN  23

#define DHTTYPE DHT11

// ---------------- WiFi Credentials ----------------

char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// ---------------- LCD Configuration ----------------

LiquidCrystal lcd(13, 14, 27, 26, 25, 33);

// ---------------- Sensor Objects ----------------

Adafruit_BME680 bme;
DHT dht(DHTPIN, DHTTYPE);

// ---------------- Variables ----------------

int mq7Value = 0;
int mq135Value = 0;

int mq7Percentage = 0;
int mq135Percentage = 0;

float temperature = 0;
float humidity = 0;

float pressure = 0;
int pressurePercentage = 0;

// ---------------- Setup Function ----------------

void setup()
{
    Serial.begin(115200);

    // Pin Modes
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(MQ7_PIN, INPUT);
    pinMode(MQ135_PIN, INPUT);

    // LCD Initialization
    lcd.begin(16, 2);

    lcd.setCursor(0, 0);
    lcd.print(" IOT Air ");

    lcd.setCursor(0, 1);
    lcd.print("Quality Monitor");

    delay(2000);
    lcd.clear();

    // DHT Initialization
    dht.begin();

    // BME680 Initialization
    if (!bme.begin())
    {
        Serial.println("BME680 Sensor Not Found!");

        lcd.setCursor(0, 0);
        lcd.print("BME680 Error");

        while (1);
    }

    bme.setPressureOversampling(BME680_OS_4X);

    // WiFi Connection
    Serial.println("Connecting to WiFi...");

    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi Connected");

    // Blynk Initialization
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Buzzer Indication
    digitalWrite(BUZZER_PIN, HIGH);
    delay(1000);
    digitalWrite(BUZZER_PIN, LOW);
}

// ---------------- Main Loop ----------------

void loop()
{
    Blynk.run();

    // Read MQ Sensors
    mq7Value = analogRead(MQ7_PIN);
    mq135Value = analogRead(MQ135_PIN);

    // Convert to Percentage
    mq7Percentage = map(mq7Value, 0, 4095, 0, 100);
    mq135Percentage = map(mq135Value, 0, 4095, 0, 100);

    // Read DHT Sensor
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    // Read BME680 Pressure
    pressure = bme.readPressure() / 100.0F;
    pressurePercentage = map(pressure, 0, 1100, 0, 100);

    // Sensor Validation
    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("DHT Sensor Read Failed");
        return;
    }

    // ---------------- Serial Monitor Output ----------------

    Serial.print("MQ7: ");
    Serial.print(mq7Percentage);
    Serial.print("%  ");

    Serial.print("MQ135: ");
    Serial.print(mq135Percentage);
    Serial.print("%  ");

    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print(" C  ");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("%  ");

    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" hPa");

    // ---------------- LCD Display ----------------

    lcd.setCursor(0, 0);
    lcd.print("AQ:");
    lcd.print(mq135Percentage);
    lcd.print("% ");

    lcd.print("CO:");
    lcd.print(mq7Percentage);

    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(temperature);

    lcd.print(" H:");
    lcd.print(humidity);

    // ---------------- Blynk Cloud Update ----------------

    Blynk.virtualWrite(V0, mq7Percentage);
    Blynk.virtualWrite(V1, mq135Percentage);
    Blynk.virtualWrite(V2, temperature);
    Blynk.virtualWrite(V3, humidity);
    Blynk.virtualWrite(V4, pressurePercentage);

    // ---------------- Alert Condition ----------------

    if (mq135Percentage > 70 || mq7Percentage > 70)
    {
        digitalWrite(BUZZER_PIN, HIGH);
    }
    else
    {
        digitalWrite(BUZZER_PIN, LOW);
    }

    delay(1000);
}
