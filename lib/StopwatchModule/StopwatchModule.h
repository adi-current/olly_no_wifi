#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include "TimerEngine.h"

// Counts up from zero. Always starts fresh at 0:00 when opened (doesn't
// remember a paused time across visits, unlike Timer's remembered
// duration - this matches how phone/wall stopwatches normally behave).
//
// touch short press: 1st tap starts, 2nd pauses, 3rd resumes, etc.
// touch long press: back to menu, always.
class StopwatchModule {
  public:
    void begin(U8G2* display);

    // Call when selected from the menu - resets to 0:00.
    void start();

    // Returns true when the user wants to exit to the menu.
    bool update(bool actionPressed, bool backPressed);

    void render();

  private:
    U8G2* display = nullptr;
    TimerEngine engine;
    bool everStarted = false; // false = still at the initial "tap to start" prompt
};
