int setPin = 2, setLedPin = 3, resetPin = 4, resetLedPin = 5,  doorbellPin = 7, doorbellLedPin = 6, buzzerPin = 8, resetSession = 0, idleTimer = 0;
int savedPattern[8] = {0, 0, 0, 0, 0, 0, 0, 0};
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
    for(int i = 0; i<8; i++){
      savedPattern[i] = 0;
    }
    while(idleTimer < 3){
      if(setPin == HIGH){
        
      }
      delay(1000);
      idleTimer++;
    }
  }
}
