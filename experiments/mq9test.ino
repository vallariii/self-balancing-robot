#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pins
#define MQ9_PIN A0
#define FLAME_PIN 2
#define BUZZER 8
#define IN1 3
#define IN2 4
#define IN3 5
#define IN4 6
#define ENA 9
#define ENB 10

// PPM Constants (Approximate for MQ-9 CO detection)
const float RL = 10.0;      // Load resistor in kilo-ohms
const float Ro = 1.2;       // Ro must be calibrated in clean air
const float m = -2.1;       // Slope from datasheet
const float b = 2.3;        // Intercept from datasheet

// Variables
int gasValue = 0;
float ppm = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastMoveTime = 0;
bool isEscaping = false;

void setup() {
  pinMode(FLAME_PIN, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  
  Serial.begin(9600);
  lcd.init(); // Use init() for newer LiquidCrystal_I2C versions
  lcd.backlight();
  lcd.print("System Warming Up");
  delay(2000); // MQ sensors need heat-up time
}

// -------- SENSOR MATH --------
float calculatePPM(int rawValue) {
  if (rawValue == 0) return 0;
  float vOut = rawValue * (5.0 / 1023.0);
  float rs = ((5.0 - vOut) / vOut) * RL; 
  float ratio = rs / Ro;
  float ppm_log = (log10(ratio) - b) / m;
  return pow(10, ppm_log);
}

// -------- MOTOR CONTROL --------
void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void escapeManeuver() {
  unsigned long elapsed = millis() - lastMoveTime;
  if (elapsed < 1500) { // Move Backward
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    analogWrite(ENA, 200); analogWrite(ENB, 200);
  } 
  else if (elapsed < 3000) { // Sharp Right Turn
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    analogWrite(ENA, 220); analogWrite(ENB, 0);
  } 
  else {
    stopMotors();
  }
}

void loop() {
  gasValue = analogRead(MQ9_PIN);
  ppm = calculatePPM(gasValue);
  int flameState = digitalRead(FLAME_PIN);
  bool fireDetected = (flameState == LOW);

  // -------- LOGIC GATE --------
  if (fireDetected || ppm > 100) { // 100ppm is a standard safety threshold
    if (!isEscaping) {
      lastMoveTime = millis();
      isEscaping = true;
    }
    escapeManeuver();
    tone(BUZZER, fireDetected ? 2500 : 1500); 
  } else {
    stopMotors();
    noTone(BUZZER);
    isEscaping = false;
  }

  // -------- LCD DISPLAY --------
  if (millis() - lastLCDUpdate > 500) {
    lcd.clear();
    lcd.setCursor(0, 0);
    
    if (fireDetected) {
      lcd.print("!!! FIRE !!!");
    } else {
      lcd.print("CO Level: ");
      lcd.print((int)ppm);
      lcd.print("ppm");
    }

    lcd.setCursor(0, 1);
    if (ppm > 50 && ppm < 200) lcd.print("SMOKE: LIGHT");
    else if (ppm >= 200) lcd.print("SMOKE: HEAVY");
    else lcd.print("STATUS: SECURE");

    lastLCDUpdate = millis();
    
    // Serial Debug for Plotter
    Serial.print("Raw:"); Serial.print(gasValue);
    Serial.print(",PPM:"); Serial.println(ppm);
  }
}
