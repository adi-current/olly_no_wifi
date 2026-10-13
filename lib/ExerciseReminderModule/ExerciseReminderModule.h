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
// Selecting "Exercise Reminder" from the menu now opens a small dedicated
// screen instead of toggling instantly - same idea as TimerModule's
// IDLE/EDIT flow: a short touch arms/disarms it, an encoder click opens
// interval editing (rotate to adjust, click to confirm+save).
//
// ASSUMPTION: armed/disarmed state is still NOT persisted - resets to
// disarmed on every reboot, unlike the interval (which now is). Easy to
// add later with the same Preferences technique used here if you want it
// to stick across power cycles too.
class ExerciseReminderModule {
  public:
    void begin();

    void toggleArmed(); // flips armed/disarmed using the current interval
    bool isArmed() const;

    void update(); // call every loop(), regardless of what else is on screen

    // True exactly once, the loop it fires - main.cpp should switch to
    // showing renderInterrupt() for a few seconds when this happens.
    bool checkAndClearFired();

    void renderInterrupt(U8G2* display); // the brief full-screen reminder message

    // --- Settings screen: view armed state + edit the interval ---
    void start(); // call when selected from the menu - resets to the view screen

    // touchShort toggles armed/disarmed. encoderClick (while viewing) opens
    // interval editing; (while editing) confirms+saves and returns to
    // viewing. touchLong always exits back to the menu, from either state.
    // Returns true once it's time to hand control back to the menu.
    bool updateSettingsScreen(int rotationDelta, bool encoderClick, bool touchShort, bool touchLong);

    void renderSettingsScreen(U8G2* display);

    uint16_t getIntervalMinutes() const { return intervalMinutes; }

  private:
    bool armed = false;
    TimerEngine engine;
    bool fired = false;

    uint16_t intervalMinutes = 60; // default 60 min, persisted via NVS once changed
    Preferences prefs;
    void saveInterval();

    enum class SettingsState { VIEW, EDIT_INTERVAL };
    SettingsState settingsState = SettingsState::VIEW;

    static const uint16_t MIN_INTERVAL_MINUTES = 5;
    static const uint16_t MAX_INTERVAL_MINUTES = 240; // 4 hours
    static const int INTERVAL_STEP_MINUTES = 5;
};
