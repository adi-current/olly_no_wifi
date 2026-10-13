#pragma once
#include <Arduino.h>

// Driver for a KY-040 rotary encoder module: quadrature rotation (CLK/DT)
// plus its built-in push button (SW).
//
// Rotation decoding uses a full-step state-graph algorithm (the same
// approach used by most mature Arduino rotary encoder libraries), NOT a
// "count raw pulses and divide by 4" approach. That distinction matters in
// practice: a raw pulse-counting approach assumes every physical detent
// produces a clean multiple of 4 transitions, which is often false on
// cheap encoders like the KY-040 - a slightly short or bouncy turn can
// leave a fractional remainder that either delays a later, unrelated turn
// or gets discarded outright, causing clicks to feel skipped or laggy.
//
// The state-graph approach instead tracks the encoder's exact position in
// a known sequence of valid states. A click is only ever emitted once a
// complete, valid detent-to-detent path has actually occurred; any bounce
// or noise along the way just falls back to the start state harmlessly,
// with nothing partial left over to confuse a later turn.
//
// The button gets simple polled debounce (in update()) plus short/long
// press classification, matching the interaction rules already established
// project-wide: encoder click = forward/confirm, touch long-press = back.
class EncoderModule {
  public:
    void begin();

    // Call every loop() iteration. Cheap - just reads the button debounce
    // timer. Rotation is handled entirely by the ISR; nothing to poll there.
    void update();

    // Net detent clicks since the last call: positive = clockwise,
    // negative = counter-clockwise, 0 = no movement. Consumes/resets after
    // reading. Each full physical detent is exactly +1 or -1 - no
    // division, no remainders.
    int takeRotation();

    // Edge-triggered, consume-once flags - each returns true exactly once
    // per physical press, then resets, so calling it in a loop doesn't
    // fire repeatedly for one press.
    bool wasClicked();      // short press released
    bool wasLongPressed();  // held past LONG_PRESS_MS

    bool isButtonDown() const;

  private:
    static const unsigned long DEBOUNCE_MS = 8;
    static const unsigned long LONG_PRESS_MS = 600;

    // Button state
    bool buttonStableState = false;   // true = pressed (debounced)
    bool buttonRawLast = false;
    unsigned long buttonLastChangeMs = 0;
    unsigned long buttonPressedAtMs = 0;
    bool longPressFired = false;
    bool clickPending = false;
    bool longPressPending = false;

    // Rotation - the ISR advances encoderState through the state graph and
    // increments/decrements this directly (by whole clicks, never
    // fractional) whenever a full valid rotation completes. update()/
    // takeRotation() just drain it - no per-call math needed.
    volatile int rotationAccumulator = 0;
    uint8_t encoderState = 0; // starts at R_START, see EncoderModule.cpp

    static void IRAM_ATTR onEncoderChangeISR();
    void handleEncoderChange(); // actual state-graph logic, called from ISR

    static EncoderModule* instance; // for the static ISR to reach handleEncoderChange()
};
