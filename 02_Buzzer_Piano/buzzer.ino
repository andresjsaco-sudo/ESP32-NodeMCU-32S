#include <Arduino.h>

const int PIN_SPEAKER = 0;
const int PINES_BTN[] = { 18, 19, 21, 22, 23 };
const int N_NOTAS = 5;
const int FREQS[] = { 262, 294, 330, 349, 392 };
const int CANAL = 0;
const int RES_BITS = 8;

void setup() {
  ledcAttach(PIN_SPEAKER, 262, RES_BITS);
  ledcWrite(PIN_SPEAKER, 200);
  for (int i = 0; i < N_NOTAS; i++)
    pinMode(PINES_BTN[i], INPUT);
}


void loop() {
  ledcWriteTone(0, 0);

  for (int i = 0; i < N_NOTAS; i++) {
    if (digitalRead(PINES_BTN[i]) == HIGH) {
      ledcWriteTone(0, FREQS[i]);
      break;
    }
  }

 // delay(100);
}

