#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <avr/wdt.h>

// SPI mode for BME280
// Hardware SPI (Uno): CS 10, SDA 11, SDO 12, SCL 13
#define BMP_CS 10
Adafruit_BMP280 bmp(BMP_CS);

// LCD Screen
LiquidCrystal_I2C lcd(0x27,16,2);

// Error count for sensor
uint8_t errorCount = 0;
const uint8_t MAX_ERRORS = 5;

// Poll Rate in ms
const uint8_t pollDelay = 1000;


void setup() {

  SPI.begin();

  Serial.begin(9600);

  Serial.println(F("Chamber Sensor v.0.1\n"));
  Serial.println(F("INFO - Initializing Sensors"));

  // init LCD Screen

  Serial.println(F("INFO - Initializing LCD over I2C"));
  lcd.init();
  lcd.backlight();
  
  // init BMP280
  
 Serial.println(F("INFO - Initializing BME280"));

  while(!bmp.begin()) {
    lcd_write("ERROR E01", "BMP280 NO INIT", false);
    Serial.println(F("ERROR - BMP280 INIT FAIL"));
    Serial.println(bmp.sensorID(),16);
    delay(1000);
  }

  // Enable Watchdog with 8 second timer
  wdt_enable(WDTO_8S);
}

void loop() {

  // Watchdog Reset
  wdt_reset();
  
  // Read Temperature and check if it's NaN
  float temp = bmp.readTemperature();

  if (isnan(temp) || temp < -40 || temp > 85) {
    errorCount++;
    if(errorCount >= MAX_ERRORS) {
      lcd_write("ERROR E02", "TEMP DATA FAULT", false);
      Serial.println(F("ERROR - Received invalid data from BMP280, reinitializing"));
      delay(3000);
      if (!bmp.begin()) {
        lcd_write("ERROR E02", "BOARD RESET", false);
        Serial.println(F("ERROR - Reinitializing failed!"));
        Serial.println(F("INFO - Restarting the Arduino"));
        while(1);
      }
    } else {
      Serial.println(F("ERROR - Received invalid data from BMP280, skipping..."));
      goto ENDFUNC;
    }
  }
  
  // Print Temperature to LCD
  char tempchar[8];
  dtostrf(temp, 5, 2, tempchar);
  Serial.print("Temp: ");
  Serial.print(tempchar);
  Serial.print("\n");
  lcd_write("Temperature:", tempchar, true);

  ENDFUNC:
  delay(pollDelay);
}

void lcd_write(char input1[15], char input2[15], bool enableDegrees) {
  lcd.clear();
  lcd.home();
  lcd.print(input1);
  lcd.setCursor(0,1);
  lcd.print(input2);
  if(enableDegrees) {
    lcd.print((char)223);
    lcd.print("C");
  }
}
