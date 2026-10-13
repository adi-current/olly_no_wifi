#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include <Preferences.h>
#include "TimerEngine.h"

// Background reminder - unlike the other 3 timer features, arming/disarming
// doesn't take over the screen long-term. update() must be called every
// loop() regardless of what else is showing (Home, a game, the menu...);
// checkAndClearFired() reports when it's time to briefly interrupt the
// screen with a reminder message (renderInterrupt()).
//
// Settings screen - one screen, no hidden edit mode, same grammar as
// Brightness:
//   Turn        pick an interval preset (live preview, nothing committed yet)
//   Press       Start / Apply (different interval) / Stop (same interval) - the
//               footer always spells out which
//   Tap         unused on this screen (one control, one job)
//   Long touch  back; the selection was only a preview, so nothing to undo
//
// Both the interval AND the armed state are saved, so a power cut no longer
// silently turns the reminder off. (After a reboot the countdown restarts
// from the full interval - a small inaccuracy, but better than silence.)
class ExerciseReminderModule {
  public:
    void begin();

    void toggleArmed(); // stop if running, else start with the current interval
    bool isArmed() const;

    void update(); // call every loop(), regardless of what else is on screen

    // True exactly once, the loop it fires - main.cpp should switch to
    // showing renderInterrupt() for a few seconds when this happens.
    bool checkAndClearFired();

    void renderInterrupt(U8G2* display); // the brief full-screen reminder message

    // --- Settings screen ---
    void start(); // call when selected from the menu - cursor starts on the current interval

    // Returns true once it's time to hand control back to the menu.
    bool updateSettingsScreen(int rotationDelta, bool encoderClick, bool touchShort, bool touchLong);

    void renderSettingsScreen(U8G2* display);

    uint16_t getIntervalMinutes() const { return intervalMinutes; }

  private:
    bool armed = false;
    TimerEngine engine;
    bool fired = false;

    uint16_t intervalMinutes = 60; // the interval actually running / last committed
    uint8_t selIndex = 3;          // settings-screen cursor into PRESETS (preview only)
    Preferences prefs;

    void saveSettings();                    // one flash write: interval + armed
    void startReminder(uint16_t minutes);   // commit interval, arm, restart countdown
    void stopReminder();
};
