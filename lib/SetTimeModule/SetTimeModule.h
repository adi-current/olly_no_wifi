#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include "RtcModule.h"

// Lets the user set the RTC's time directly on the device - the immediate,
// no-WiFi-needed way to get accurate time (NTP sync can come later as a
// second, automatic option once WiFi/Settings exists; this one always
// works regardless).
//
// Only hour and minute are editable - date/day-of-week/month/year are read
// from the RTC on entry and preserved untouched, so fixing the time never
// scrambles the date. Seconds are zeroed on confirm (setting to the top of
// the just-confirmed minute).
//
// encoder rotate -> adjust the active field (hour, then minute)
// encoder click  -> cycle EDIT_HOUR -> EDIT_MINUTE -> confirm+save+exit
// touch long-press -> cancel, back to menu, no changes saved
class SetTimeModule {
  public:
    void begin(U8G2* display, RtcModule* rtc);

    // Call when selected from the menu - loads the RTC's current time into
    // the edit fields as a starting point.
    void start();

    // Returns true when done (either saved, or cancelled) and the menu
    // should take back over.
    bool update(int rotationDelta, bool encoderClick, bool touchLong);

    void render();

  private:
    U8G2* display = nullptr;
    RtcModule* rtc = nullptr;

    enum class State { EDIT_HOUR, EDIT_MINUTE };
    State state = State::EDIT_HOUR;

    uint8_t editHour = 0;
    uint8_t editMinute = 0;
    RtcTime baseTime; // preserves date/dayOfWeek/month/year from the RTC, untouched
};
