#include <LiquidCrystal.h>
#include <Servo.h>

// LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Servo
Servo myservo;

// Pins
const int startBtn   = 8;
const int resetBtn   = 7;
const int drainBtn   = 6;

const int heaterRelay = 10;
const int drainRelay  = 9;
const int pumpPin     = 4;
const int ledPin      = 13;

// Timings (in seconds)
const int STERILIZE_TIME = 180;
const int DRY_TIME       = 120;
const int COOL_TIME      = 60;
const int DRAIN_TIME     = 5;

// States
enum State {
  IDLE,
  STERILIZING,
  DRYING,
  COOLING,
  DRAINING,
  COMPLETE
};

State currentState = IDLE;

// Timer variables
unsigned long previousMillis = 0;
int remainingTime = 0;

void setup() {
  lcd.begin(16, 2);

  pinMode(startBtn, INPUT_PULLUP);
  pinMode(resetBtn, INPUT_PULLUP);
  pinMode(drainBtn, INPUT_PULLUP);

  pinMode(heaterRelay, OUTPUT);
  pinMode(drainRelay, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  myservo.attach(3);

  resetSystem();
}

void loop() {

  if (digitalRead(resetBtn) == LOW) {
    resetSystem();
    return;
  }

  unsigned long currentMillis = millis();

  if (currentState != IDLE && currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    remainingTime--;
    updateLCD();

    if (remainingTime <= 0) {
      nextState();
    }
  }

  if (currentState == IDLE && digitalRead(startBtn) == LOW) {
    startSterilizing();
  }
}

void startSterilizing() {
  currentState = STERILIZING;
  remainingTime = STERILIZE_TIME;

  digitalWrite(pumpPin, HIGH);
  digitalWrite(heaterRelay, HIGH);
  myservo.write(180);

  updateLCD();
}

void nextState() {
  switch (currentState) {

    case STERILIZING:
      digitalWrite(pumpPin, LOW);
      myservo.write(0);
      currentState = DRYING;
      remainingTime = DRY_TIME;
      break;

    case DRYING:
      digitalWrite(heaterRelay, LOW);
      currentState = COOLING;
      remainingTime = COOL_TIME;
      break;

    case COOLING:
      currentState = DRAINING;
      remainingTime = DRAIN_TIME;
      digitalWrite(drainRelay, HIGH);
      break;

    case DRAINING:
      digitalWrite(drainRelay, LOW);
      currentState = COMPLETE;
      lcd.clear();
      lcd.print("Process Done");
      digitalWrite(ledPin, HIGH);
      break;

    default:
      break;
  }
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);

  switch (currentState) {
    case STERILIZING: lcd.print("Sterilizing"); break;
    case DRYING:      lcd.print("Steam Drying"); break;
    case COOLING:     lcd.print("Cooling Down"); break;
    case DRAINING:    lcd.print("Draining"); break;
    default:          return;
  }

  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(remainingTime);
  lcd.print("s");
}

void resetSystem() {
  currentState = IDLE;
  digitalWrite(pumpPin, LOW);
  digitalWrite(heaterRelay, LOW);
  digitalWrite(drainRelay, LOW);
  digitalWrite(ledPin, LOW);
  myservo.write(0);

  lcd.clear();
  lcd.print("System Ready");
}
