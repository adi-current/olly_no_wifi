#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <Preferences.h>
#include "TimerEngine.h"

// A general-purpose countdown timer that remembers its last-used duration
// across power cycles (via NVS/Preferences) - so re-opening it after days
// away still shows the same duration you used last time, ready to go.
//
// This is the one feature that genuinely needs BOTH encoder and touch as
// distinct inputs (not just touch), per the explicit design:
//   encoder rotate -> adjust the currently-active field (minutes/seconds)
//   encoder click  -> cycle IDLE -> edit minutes -> edit seconds -> (save+)IDLE
//   touch short    -> start (from IDLE) / pause <-> resume (RUNNING/PAUSED), same as Pomodoro and Stopwatch
//   encoder click  -> also dismisses COMPLETE
//   touch long     -> back to menu (cancels a running timer), always, from any state
class TimerModule {
  public:
    void begin(U8G2* display);

    // Call when selected from the menu - loads the saved duration, shows IDLE.
    void start();

    // Returns true when the user wants to exit to the menu.
    bool update(int rotationDelta, bool encoderClick, bool touchShort, bool touchLong);

    void render();

  private:
    U8G2* display = nullptr;
    TimerEngine engine;
    Preferences prefs;

    enum class State { IDLE, EDIT_MINUTES, EDIT_SECONDS, RUNNING, PAUSED, COMPLETE };
    State state = State::IDLE;

    uint16_t setMinutes = 10; // default the very first time, before anything's ever been saved
    uint8_t setSeconds = 0;

    unsigned long completeShownAtMs = 0;
    static const unsigned long COMPLETE_AUTO_RETURN_MS = 4000;

    void loadSavedDuration();
    void saveDuration(); // writes to flash - only called on edit-confirm, not continuously
    uint32_t totalSetSeconds() const;
};
