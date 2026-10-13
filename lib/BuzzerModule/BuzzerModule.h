#pragma once
#include <Arduino.h>

// Non-blocking passive-buzzer driver using the ESP32 LEDC (PWM) peripheral.
// Call begin() once in setup(), and update() every loop() so timed tones
// stop themselves without ever calling delay().
class BuzzerModule {
public:
  void begin();

  // Play a tone at `freq` Hz for `durationMs` milliseconds, non-blocking.
  // Calling this again before the previous tone finishes just replaces it.
  void tone(uint16_t freq, uint16_t durationMs);

  // Stop any tone immediately.
  void stop();

  // Must be called every loop() - turns the tone off once its time is up.
  void update();

  // --- A few ready-made cues so call sites can stay descriptive ---
  void beepClick();       // short, low - menu confirm/click
  void beepSuccess();     // short rising chirp - task/timer done
  void beepAlert();       // two short beeps - exercise reminder / notification

private:
  unsigned long stopAtMs = 0;
  bool playing = false;

  // beepAlert() is documented as two short beeps - this schedules the
  // second one without blocking, same non-blocking philosophy as the tone
  // timing above.
  bool secondBeepPending = false;
  unsigned long secondBeepAtMs = 0;
};
