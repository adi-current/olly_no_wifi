#pragma once
#include <Arduino.h>

// Driver for a single touch sensor breakout (e.g. TTP223 - the ESP32-C3 has
// no built-in capacitive touch peripheral, so this just reads a plain
// digital HIGH/LOW signal from an external module, not an internal
// touchRead()).
//
// Only one physical sensor exists, so it carries both project-wide touch
// roles via press duration, same pattern as the encoder's button:
//   short press -> feature-specific action (start/stop, confirm a pick, etc.)
//   long press  -> always means "go back one level"
class TouchModule {
  public:
    void begin();

    // Call every loop() iteration.
    void update();

    // Edge-triggered, consume-once - each returns true exactly once per
    // physical touch, then resets.
    bool wasPressed();      // short touch released - feature-specific action
    bool wasLongPressed();  // held past LONG_PRESS_MS - always means "back"

    bool isTouched() const;

  private:
    // A bit more debounce margin than the encoder's mechanical button -
    // capacitive sensors can briefly flicker right at the edge of a touch.
    static const unsigned long DEBOUNCE_MS = 30;
    // Matches the encoder button's long-press threshold, so both inputs
    // feel consistent to use.
    static const unsigned long LONG_PRESS_MS = 600;

    bool rawLast = false;
    bool stableState = false; // true = currently touched (debounced)
    unsigned long lastChangeMs = 0;
    unsigned long touchedAtMs = 0;
    bool longPressFired = false;
    bool pressPending = false;
    bool longPressPending = false;
};
