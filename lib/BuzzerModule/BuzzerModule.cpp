#include "BuzzerModule.h"
#include "config.h"

void BuzzerModule::begin() {
  pinMode(BUZZER_PIN, OUTPUT);
}

void BuzzerModule::tone(uint16_t freq, uint16_t durationMs) {
  // "::" calls the global Arduino tone(), not this class's own tone()
  ::tone(BUZZER_PIN, freq);
  stopAtMs = millis() + durationMs;
  playing = true;
}

void BuzzerModule::stop() {
  ::noTone(BUZZER_PIN);
  playing = false;
}

void BuzzerModule::update() {
  if (playing && millis() >= stopAtMs) {
    stop();
  }
  if (secondBeepPending && millis() >= secondBeepAtMs) {
    secondBeepPending = false;
    tone(1500, 100);
  }
}

void BuzzerModule::beepClick() {
  tone(1800, 40);
}

void BuzzerModule::beepSuccess() {
  tone(2400, 120);
}

void BuzzerModule::beepAlert() {
  tone(1500, 100);
  secondBeepPending = true;
  secondBeepAtMs = millis() + 180; // 100ms tone + 80ms gap before the second beep
}
