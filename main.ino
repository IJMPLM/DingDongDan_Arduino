#include <EEPROM.h>

int setPin = 2, setLedPin = 3, resetPin = 4, resetLedPin = 5, doorbellPin = 7, doorbellLedPin = 6, buzzerPin = 8;
int resetSession = 0, idleTimer = 0, setTimer = 0, setPatternIndex = 0, patternDuration = 0, patternError = 0;
int savedPattern[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // Pattern array with 7 slots + termination

unsigned long lastActivityTime = 0; // Tracks the last activity for idle timer
const int idleTimeout = 3000;       // 3 seconds in milliseconds
bool settingPattern = false;

void setup() {
  Serial.begin(9600); // Initialize Serial Communication
  pinMode(setPin, INPUT);
  pinMode(resetPin, INPUT);
  pinMode(doorbellPin, INPUT);
  pinMode(setLedPin, OUTPUT);
  pinMode(resetLedPin, OUTPUT);
  pinMode(doorbellLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  loadPatternFromEEPROM(); // Load saved pattern at startup
  Serial.println("System Initialized.");
}

void loop() {
  // Reset Pattern
  if (digitalRead(resetPin) == HIGH) {
    handleReset();
  }

  // Set Pattern
  if (digitalRead(setPin) == HIGH) {
    handleSetPattern();
  }

  // Doorbell Pattern Match
  if (digitalRead(doorbellPin) == HIGH) {
    handleDoorbell();
  }
}

void handleReset() {
  Serial.println("Resetting pattern...");
  digitalWrite(resetLedPin, HIGH);
  for (int i = 0; i < 7; i++) {
    savedPattern[i] = 0;
  }
  savePatternToEEPROM();
  setPatternIndex = 0;
  delay(1000);
  digitalWrite(resetLedPin, LOW);
  blinkConfirmation();
  Serial.println("Pattern reset successfully.");
}

void handleSetPattern() {
  Serial.println("Recording pattern...");
  settingPattern = true;

  // Clear the pattern array before recording a new one
  for (int i = 0; i < 8; i++) {
    savedPattern[i] = 0;
  }

  setPatternIndex = 0;
  lastActivityTime = millis();

  while (millis() - lastActivityTime < idleTimeout && setPatternIndex < 7) {
    if (digitalRead(setPin) == HIGH) {
      digitalWrite(setLedPin, HIGH);
      patternDuration = 0;

      // Wait for button release to measure duration
      while (digitalRead(setPin) == HIGH) {
        delay(100);
        patternDuration++;
      }
      int inputType = (patternDuration > 10) ? 2 : 1;
      savedPattern[setPatternIndex++] = inputType;

      // Debugging message for short/long press
      if (inputType == 1) {
        Serial.print("Input "); Serial.print(setPatternIndex); Serial.println(": Short press recorded.");
      } else {
        Serial.print("Input "); Serial.print(setPatternIndex); Serial.println(": Long press recorded.");
      }

      digitalWrite(setLedPin, LOW);
      lastActivityTime = millis(); // Reset idle timer
    }
  }

  settingPattern = false;
  savePatternToEEPROM();
  blinkConfirmation();
  Serial.println("Pattern saved successfully.");
  Serial.print("Saved Pattern: ");
  printPattern(savedPattern);
}

void handleDoorbell() {
  Serial.println("Checking doorbell pattern...");
  int doorbellIndex = 0;
  patternError = 0;
  lastActivityTime = millis();

  while (millis() - lastActivityTime < idleTimeout && doorbellIndex < 7 && savedPattern[doorbellIndex] != 0) {
    if (digitalRead(doorbellPin) == HIGH) {
      digitalWrite(doorbellLedPin, HIGH);
      patternDuration = 0;

      // Wait for button release to measure duration
      while (digitalRead(doorbellPin) == HIGH) {
        delay(100);
        patternDuration++;
      }
      int inputType = (patternDuration > 10) ? 2 : 1;

      // Debugging message for short/long press
      if (inputType == 1) {
        Serial.print("Doorbell Input "); Serial.print(doorbellIndex + 1); Serial.println(": Short press.");
      } else {
        Serial.print("Doorbell Input "); Serial.print(doorbellIndex + 1); Serial.println(": Long press.");
      }

      if (inputType != savedPattern[doorbellIndex]) {
        patternError = 1;
        break;
      }

      doorbellIndex++;
      digitalWrite(doorbellLedPin, LOW);
      lastActivityTime = millis(); // Reset idle timer
    }
  }

  // Check if pattern matched completely
  if (patternError == 0 && savedPattern[doorbellIndex] == 0) {
    patternError = 2;
  }

  // Act based on pattern matching results
  if (patternError == 1) {
    Serial.println("Pattern does NOT match.");
  } else if (patternError == 2) {
    Serial.println("Pattern matches!");
  }

  playFeedback(patternError);
}

void blinkConfirmation() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(setLedPin, HIGH);
    digitalWrite(resetLedPin, HIGH);
    delay(250);
    digitalWrite(setLedPin, LOW);
    digitalWrite(resetLedPin, LOW);
    delay(250);
  }
}

void playFeedback(int errorType) {
  if (errorType == 1) {
    // Incorrect pattern: Play "ding-dong"
    tone(buzzerPin, 783, 300);
    delay(350);
    tone(buzzerPin, 523, 400);
    Serial.println("Playing: Ding-dong (incorrect pattern).");
  } else if (errorType == 2) {
    // Correct pattern: Play "ding-dong-dan"
    tone(buzzerPin, 783, 300);
    delay(350);
    tone(buzzerPin, 523, 400);
    delay(350);
    tone(buzzerPin, 659, 500);
    delay(1000);
    Serial.println("Playing: Ding-dong-dan (correct pattern).");
  }
}

void savePatternToEEPROM() {
  for (int i = 0; i < 8; i++) {
    EEPROM.write(i, savedPattern[i]);
  }
}

void loadPatternFromEEPROM() {
  for (int i = 0; i < 8; i++) {
    savedPattern[i] = EEPROM.read(i);
  }
  Serial.print("Loaded Pattern: ");
  printPattern(savedPattern);
}

void printPattern(int pattern[]) {
  for (int i = 0; i < 7; i++) {
    Serial.print(pattern[i]);
    if (i < 6) Serial.print(", ");
  }
  Serial.println();
}
