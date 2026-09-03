#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// =================================================
// LCD
// =================================================
LiquidCrystal_I2C lcd(0x27, 16, 2);


// =================================================
// PIN DEFINITIONS
// =================================================
const int trigPin = 9;
const int echoPin = 10;

const int lightSensor = A0;

const int ledPin = 13;
const int buzzerPin = 8;

const int buttonPin = 7;


// =================================================
// TIMER VARIABLES
// =================================================
unsigned long dangerStartTime = 0;

bool dangerTimerStarted = false;
bool wrecked = false;
bool openSea = false;


// =================================================
// SETUP
// =================================================
void setup() {

  Serial.begin(9600);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  // Pushbutton connected between pin 7 and GND
  pinMode(buttonPin, INPUT_PULLUP);

  // Start LCD
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("System Ready");

  delay(2000);

  lcd.clear();
}


// =================================================
// MAIN LOOP
// =================================================
void loop() {

  // =================================================
  // DISTANCE SENSOR
  // =================================================

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);

  float distance = duration * 0.0343 / 2;


  // =================================================
  // LIGHT SENSOR
  // =================================================

  int lightValue = analogRead(lightSensor);


  // =================================================
  // DETERMINE DANGER
  // =================================================

  bool distanceDanger = false;
  bool lightDanger = false;

  // Distance danger
  if (distance <= 100 && distance > 0) {
    distanceDanger = true;
  }

  // Light danger
  if (lightValue > 512) {
    lightDanger = true;
  }

  // Either sensor is enough to cause danger
  bool danger = distanceDanger || lightDanger;


  // =================================================
  // LED CONTROL
  // =================================================

  if (distanceDanger) {
    digitalWrite(ledPin, HIGH);
  }
  else {
    digitalWrite(ledPin, LOW);
  }


  // =================================================
  // BUZZER CONTROL
  // =================================================

  if (lightDanger) {
    digitalWrite(buzzerPin, HIGH);
  }
  else {
    digitalWrite(buzzerPin, LOW);
  }


  // =================================================
  // PUSHBUTTON
  // =================================================

  // Button can ONLY activate OPEN SEA during
  // the countdown, NOT during WRECKED.

  if (digitalRead(buttonPin) == LOW && !wrecked && danger) {

    openSea = true;

    // Stop countdown
    dangerTimerStarted = false;

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("OPEN SEA");

    lcd.setCursor(0, 1);
    lcd.print("Override Active");

    delay(300);
  }


  // =================================================
  // OPEN SEA MODE
  // =================================================

  if (openSea) {

    // Sensors continue working
    // LED and buzzer continue working

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("OPEN SEA");

    lcd.setCursor(0, 1);

    if (distanceDanger && lightDanger) {
      lcd.print("DANGER!");
    }
    else if (distanceDanger) {
      lcd.print("LED ACTIVE");
    }
    else if (lightDanger) {
      lcd.print("BUZZER ACTIVE");
    }
    else {
      lcd.print("CLEAR");
    }


    // -----------------------------------------------
    // Leave OPEN SEA if either danger condition
    // turns OFF
    // -----------------------------------------------

    if (!distanceDanger || !lightDanger) {

      openSea = false;

      dangerTimerStarted = false;
      wrecked = false;


      // ---------------------------------------------
      // If the other sensor is still detecting danger,
      // start a NEW countdown
      // ---------------------------------------------

      if (danger) {

        dangerStartTime = millis();
        dangerTimerStarted = true;

        lcd.clear();

        lcd.setCursor(0, 0);
        lcd.print("DANGER!");

        lcd.setCursor(0, 1);
        lcd.print("WRECKED IN: 5");
      }

      // ---------------------------------------------
      // If no danger remains
      // ---------------------------------------------

      else {

        lcd.clear();

        lcd.setCursor(0, 0);
        lcd.print("SAFE");
      }

      delay(100);
    }

    return;
  }


  // =================================================
  // WRECKED MODE
  // =================================================

  if (wrecked) {

    // Button DOES NOT work here.
    // WRECKED remains active.

    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzerPin, HIGH);

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("!!! WRECKED !!!");

    lcd.setCursor(0, 1);
    lcd.print("PRESS RESET");

    delay(100);

    return;
  }


  // =================================================
  // DANGER COUNTDOWN
  // =================================================

  if (danger) {

    // Start countdown
    if (!dangerTimerStarted) {

      dangerStartTime = millis();

      dangerTimerStarted = true;

      wrecked = false;
    }


    // Calculate elapsed time
    unsigned long elapsedTime =
      millis() - dangerStartTime;


    // -----------------------------------------------
    // WRECKED AFTER 5 SECONDS
    // -----------------------------------------------

    if (elapsedTime >= 5000) {

      wrecked = true;

      dangerTimerStarted = false;

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("!!! WRECKED !!!");

      lcd.setCursor(0, 1);
      lcd.print("PRESS RESET");
    }


    // -----------------------------------------------
    // COUNTDOWN
    // -----------------------------------------------

    else {

      int remainingSeconds =
        5 - (elapsedTime / 1000);

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("DANGER!");

      lcd.setCursor(0, 1);
      lcd.print("WRECKED IN: ");

      lcd.print(remainingSeconds);
    }
  }


  // =================================================
  // SAFE MODE
  // =================================================

  else {

    dangerTimerStarted = false;
    wrecked = false;

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("D:");
    lcd.print(distance, 1);
    lcd.print("cm");

    lcd.setCursor(9, 0);
    lcd.print("L:");
    lcd.print(lightValue);

    lcd.setCursor(0, 1);
    lcd.print("SAFE");
  }


  // =================================================
  // SERIAL MONITOR
  // =================================================

  Serial.print("Distance: ");
  Serial.print(distance);

  Serial.print(" cm | Light: ");
  Serial.print(lightValue);

  if (wrecked) {
    Serial.println(" | WRECKED!");
  }
  else if (openSea) {
    Serial.println(" | OPEN SEA");
  }
  else if (danger) {
    Serial.println(" | COUNTDOWN");
  }
  else {
    Serial.println(" | SAFE");
  }


  delay(100);
}
