#include "TouchModule.h"
#include "config.h"

void TouchModule::begin() {
  pinMode(TOUCH_SENSOR_PIN, INPUT); // TTP223 actively drives HIGH/LOW - no pull-up needed

  bool initialReading = (digitalRead(TOUCH_SENSOR_PIN) == HIGH);
  rawLast = initialReading;
  stableState = initialReading;
  lastChangeMs = millis();
}

void TouchModule::update() {
  bool raw = (digitalRead(TOUCH_SENSOR_PIN) == HIGH); // HIGH while touched (typical TTP223 default mode)
  unsigned long now = millis();

  if (raw != rawLast) {
    rawLast = raw;
    lastChangeMs = now;
  }

  if ((now - lastChangeMs) >= DEBOUNCE_MS && raw != stableState) {
    stableState = raw;

    if (stableState) {
      // Just touched
      touchedAtMs = now;
      longPressFired = false;
    } else {
      // Just released - if we didn't already fire a long press while held,
      // this release counts as a short press.
      if (!longPressFired) {
        pressPending = true;
      }
    }
  }

  // Long press fires while still held, not on release, so the UI can react
  // immediately (e.g. show "release to go back") rather than waiting.
  if (stableState && !longPressFired && (now - touchedAtMs) >= LONG_PRESS_MS) {
    longPressFired = true;
    longPressPending = true;
  }
}

bool TouchModule::wasPressed() {
  if (pressPending) {
    pressPending = false;
    return true;
  }
  return false;
}

bool TouchModule::wasLongPressed() {
  if (longPressPending) {
    longPressPending = false;
    return true;
  }
  return false;
}

bool TouchModule::isTouched() const {
  return stableState;
}
