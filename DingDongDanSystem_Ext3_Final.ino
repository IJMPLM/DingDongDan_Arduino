#include <EEPROM.h>

// Pin configurations
int set1ButtonPin = 11, set1LEDPin = 12;
int set2ButtonPin = 7, set2LEDPin = 10;
int set3ButtonPin = 8, set3LEDPin = 9;
int doorbellButtonPin = 4, doorbellLEDPin = 5;
int buzzerPin = 3;

int patternDuration = 0, patternError = 0;
unsigned long lastActivityTime = 0; // Tracks the last activity for idle timer
const int idleTimeout = 2000;       // 2 seconds for doorbell input checking
const int longPressDuration = 5000; // 5 seconds for clearing a pattern
bool settingPattern = false;

// Saved patterns
int savedPattern1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int savedPattern2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int savedPattern3[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void setup() {
  Serial.begin(9600);
  pinMode(set1ButtonPin, INPUT);
  pinMode(set1LEDPin, OUTPUT);
  pinMode(set2ButtonPin, INPUT);
  pinMode(set2LEDPin, OUTPUT);
  pinMode(set3ButtonPin, INPUT);
  pinMode(set3LEDPin, OUTPUT);
  pinMode(doorbellButtonPin, INPUT);
  pinMode(doorbellLEDPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  loadPatternsFromEEPROM();
  Serial.println("System Initialized.");
}

void loop() {
  // Handle set buttons
  handleSetButton(set1ButtonPin, set1LEDPin, savedPattern1, 0);
  handleSetButton(set2ButtonPin, set2LEDPin, savedPattern2, 8);
  handleSetButton(set3ButtonPin, set3LEDPin, savedPattern3, 16);

  // Handle doorbell input
  if (digitalRead(doorbellButtonPin) == HIGH) {
    handleDoorbell();
  }
}

void handleSetButton(int buttonPin, int ledPin, int pattern[], int eepromStartAddress) {
  if (digitalRead(buttonPin) == HIGH) {
    unsigned long pressStart = millis();
    digitalWrite(ledPin, HIGH);

    // Wait for release
    while (digitalRead(buttonPin) == HIGH) {
      if (millis() - pressStart >= longPressDuration) {
        clearPattern(pattern, eepromStartAddress);
        digitalWrite(ledPin, LOW);
        blinkLED(ledPin, 3); // Blink thrice for cleared pattern
        Serial.println("Pattern cleared.");
        return;
      }
    }
    digitalWrite(ledPin, LOW);

    // If short press, record a new pattern
    if (millis() - pressStart < longPressDuration) {
      recordPattern(buttonPin, ledPin, pattern, eepromStartAddress);
    }
  }
}

void clearPattern(int pattern[], int eepromStartAddress) {
  for (int i = 0; i < 8; i++) {
    pattern[i] = 0;
    EEPROM.write(eepromStartAddress + i, 0);
  }
}

void recordPattern(int buttonPin, int ledPin, int pattern[], int eepromStartAddress) {
  Serial.println("Recording pattern...");
  settingPattern = true;

  // Clear existing pattern
  for (int i = 0; i < 8; i++) {
    pattern[i] = 0;
  }

  int patternIndex = 0;
  lastActivityTime = millis();

  while (millis() - lastActivityTime < idleTimeout && patternIndex < 7) {
    if (digitalRead(buttonPin) == HIGH) {
      digitalWrite(ledPin, HIGH);
      patternDuration = 0;

      // Measure duration
      while (digitalRead(buttonPin) == HIGH) {
        delay(100);
        patternDuration++;
      }
      int inputType = (patternDuration > 10) ? 2 : 1;
      pattern[patternIndex++] = inputType;

      Serial.print("Input "); Serial.print(patternIndex); Serial.println(inputType == 1 ? ": Short press." : ": Long press.");
      digitalWrite(ledPin, LOW);
      lastActivityTime = millis();
    }
  }

  savePatternToEEPROM(pattern, eepromStartAddress);
  blinkLED(ledPin, 2); // Blink twice for saved pattern
  Serial.println("Pattern saved successfully.");
  settingPattern = false;
}

void handleDoorbell() {
  Serial.println("Checking doorbell pattern...");
  int inputPattern[8] = {0};
  int inputIndex = 0;
  lastActivityTime = millis();

  // Capture input pattern
  while (millis() - lastActivityTime < idleTimeout && inputIndex < 7) {
    if (digitalRead(doorbellButtonPin) == HIGH) {
      digitalWrite(doorbellLEDPin, HIGH);
      patternDuration = 0;

      while (digitalRead(doorbellButtonPin) == HIGH) {
        delay(100);
        patternDuration++;
      }
      inputPattern[inputIndex++] = (patternDuration > 10) ? 2 : 1;
      digitalWrite(doorbellLEDPin, LOW);
      lastActivityTime = millis();
    }
  }

  // Check against saved patterns
  if (matchPattern(inputPattern, savedPattern1)) {
    playFeedback(1);
  } else if (matchPattern(inputPattern, savedPattern2)) {
    playFeedback(2);
  } else if (matchPattern(inputPattern, savedPattern3)) {
    playFeedback(3);
  } else {
    playFeedback(0); // No match
  }
}

bool matchPattern(int input[], int saved[]) {
  for (int i = 0; i < 7; i++) {
    if (input[i] != saved[i]) return false;
    if (saved[i] == 0) break; // End of pattern
  }
  return true;
}

void playFeedback(int matchType) {
  if (matchType == 0) {
    // Incorrect pattern
    tone(buzzerPin, 783, 300);
    delay(350);
    tone(buzzerPin, 523, 400);
    Serial.println("Pattern mismatch: Ding-dong.");
  } else {
    // Correct pattern
    tone(buzzerPin, 783, 300);
    delay(350);
    tone(buzzerPin, 523, 400);
    delay(350);
    int danTone = (matchType == 1) ? 659 : (matchType == 2) ? 698 : 740;
    tone(buzzerPin, danTone, 500);
    Serial.print("Pattern matched: Ding-dong-dan (Type "); Serial.print(matchType); Serial.println(").");
  }
}

void savePatternToEEPROM(int pattern[], int startAddress) {
  for (int i = 0; i < 8; i++) {
    EEPROM.write(startAddress + i, pattern[i]);
  }
}

void loadPatternsFromEEPROM() {
  for (int i = 0; i < 8; i++) {
    savedPattern1[i] = EEPROM.read(i);
    savedPattern2[i] = EEPROM.read(8 + i);
    savedPattern3[i] = EEPROM.read(16 + i);
  }
}

void blinkLED(int ledPin, int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(ledPin, HIGH);
    delay(250);
    digitalWrite(ledPin, LOW);
    delay(250);
  }
}
