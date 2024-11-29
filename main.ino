int setPin = 2, setLedPin = 3, resetPin = 4, resetLedPin = 5,  doorbellPin = 7, doorbellLedPin = 6, buzzerPin = 8, resetSession = 0, idleTimer = 0, setTimer = 0, setPatternIndex = 0, patternDuration = 0, patternError = 0;
int savedPattern[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //only 7 inputs for the sequence, eighth 0 is for array termination
void setup(){
  pinMode(setPin, INPUT);
  pinMode(resetPin, INPUT);
  pinMode(doorbellPin, INPUT);
  pinMode(setLedPin, OUTPUT);
  pinMode(resetLedPin, OUTPUT);
  pinMode(doorbellLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
}
void loop() {
  if(resetPin == HIGH){
    digitalWrite(resetLedPin, HIGH);
    for(int i = 0; i<7; i++){     
      savedPattern[i] = 0;
    }
    while(idleTimer < 3 || setPatternIndex < 8){ //idle timerwaits for 3 secs
      if(setPin == HIGH){
        while(setPin == HIGH){
          digitalWrite(setLedPin, HIGH);
          delay(100);
          patternDuration++;
        }
        if(patternDuration > 10){
          savedPattern[setPatternIndex] = 2;
        } else{
          savedPattern[setPatternIndex] = 1;
        }
        setPatternIndex++;
        patternDuration = 0;
        digitalWrite(setLedPin, LOW);
        idleTimer = 0;
      }
      delay(1000);
      idleTimer++;
    }
    setPatternIndex = 0;
    //insert code here to save pattern to eeprom
    //blinking confirmation for new save pattern
    digitalWrite(setLedPin, LOW);
    digitalWrite(resetLedPin, LOW);
    digitalWrite(setLedPin, HIGH);
    digitalWrite(resetLedPin, HIGH);
    delay(500);
    digitalWrite(setLedPin, LOW);
    digitalWrite(resetLedPin, LOW);
    delay(300);
    digitalWrite(setLedPin, HIGH);
    digitalWrite(resetLedPin, HIGH);
    delay(500);
    digitalWrite(setLedPin, LOW);
    digitalWrite(resetLedPin, LOW);
  }
  //doorbell code below, not that if reset is active, doorbell is disabled and vice versa
  if(doorbellPin == HIGH){
    setPatternIndex = 0;
    while(idleTimer < 2 || setPatternIndex < 9 || patternError == 0 && savedPattern[setPatternIndex] > 0){ //idle timer waits for 2 secs
      if(dorbellPin == HIGH){
        while(doorbellPin == HIGH){
          digitalWrite(doorbellLedPin, HIGH);
          delay(100);
          patternDuration++;
        }
        if(patternDuration > 10){
          if(savedPattern[setPatternIndex] != 2){
            patternError = 1;
            continue;
          }
        } else{
          if(savedPattern[setPatternIndex] != 1){
            patternError = 1;
            continue;
          }
        }
        setPatternIndex++;
        patternDuration = 0;
        digitalWrite(setLedPin, LOW);
        idleTimer = 0;
      }
      if(savedPattern[setPatternIndex]==0){
        patternError = 2;
      }
      delay(1000);
      idleTimer++;
    }
    if(patternError == 1){
      //play ding dong
    }
    if(patternError == 2){
      //play ding dong dan
    }
  }
}
