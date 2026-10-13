#pragma once
#include <Arduino.h>

// Direct I2C register-level driver for the DS3231 RTC - no external RTC
// library needed, matching the project's minimal-dependency style (this
// is the same approach used by most DS3231 libraries under the hood; the
// register map is simple and well-documented).
//
// The DS3231 shares the OLED's I2C bus (SDA=GPIO8, SCL=GPIO9) - it's a
// different device address (0x68) on the same two wires, not a separate
// set of pins. IMPORTANT: this module assumes Wire.begin(SDA, SCL) has
// ALREADY been called elsewhere (DisplayModule::begin() already does this)
// - call rtc.begin() AFTER display.begin() in setup(), not before.
struct RtcTime {
  uint8_t second;
  uint8_t minute;
  uint8_t hour;      // 24-hour format, 0-23
  uint8_t dayOfWeek; // 1-7 (1 = Sunday)
  uint8_t date;      // 1-31
  uint8_t month;     // 1-12
  uint8_t year;      // 0-99, meaning 2000-2099
};

class RtcModule {
  public:
    // Checks the DS3231 actually responds on the bus. Returns false if
    // it's not wired up correctly (wrong pins, no power, bad connection).
    bool begin();

    // Reads the current time over I2C. Call this periodically (e.g. once
    // a second) rather than every loop() - the value only changes once a
    // second, no need to hammer the I2C bus every frame for it.
    //
    // If the DS3231 doesn't respond (disconnected, wiring problem), this
    // does NOT return garbage - I2C reads with nothing on the bus return
    // undefined/leftover data, which can decode into nonsense values (e.g.
    // an hour way outside 0-23), producing a garbled display. Instead,
    // this returns the last known-good time unchanged, so the displayed
    // clock just freezes rather than showing corrupted digits. Check
    // isConnected() if you want to know whether that just happened.
    RtcTime now();

    // True if the most recent now() call actually got a real reply from
    // the DS3231. False means it's disconnected/miswired right now - the
    // time being returned is frozen at its last known-good value, not live.
    bool isConnected() const;

    // Sets the RTC's time. Only needed once - the DS3231 keeps ticking on
    // its own coin-cell backup battery after that, even with the ESP32
    // powered off, so this shouldn't need to run on every boot.
    void setTime(const RtcTime& t);

    // True if the DS3231's backup battery ran out (or it's brand new and
    // has never been set) - i.e. "the time it's reporting is not to be
    // trusted." Check this once at boot; see setToCompileTimeIfNeeded().
    bool lostPower();

    // Convenience for first-time setup: if lostPower() is true, sets the
    // RTC to this sketch's own compile date/time (call with __DATE__ and
    // __TIME__) and returns true. If the battery-backed time is still
    // good, does nothing and returns false. Safe to call every boot -
    // it only actually writes the time on that first, battery-lost case.
    bool setToCompileTimeIfNeeded(const char* compileDate, const char* compileTime);

    // Unconditionally syncs the RTC to this sketch's compile date/time,
    // regardless of lostPower(). Useful while your DS3231's battery backup
    // isn't holding time reliably (common on cheap boards with a charging
    // circuit meant for a rechargeable cell, not a standard CR2032) - every
    // fresh upload re-syncs the clock to your computer's clock at compile
    // time. NOTE: this does NOT fix time persisting through a plain
    // unplug/replug with no reflash - that still requires the battery
    // backup to actually be working. Switch back to
    // setToCompileTimeIfNeeded() once that's confirmed good, since calling
    // this on every boot means a correctly-kept battery-backed time would
    // get needlessly overwritten too.
    void setToCompileTimeAlways(const char* compileDate, const char* compileTime);

  private:
    static const uint8_t DS3231_ADDR = 0x68;

    uint8_t bcdToDec(uint8_t bcd);
    uint8_t decToBcd(uint8_t dec);
    uint8_t readRegister(uint8_t reg);
    void writeRegister(uint8_t reg, uint8_t value);

    uint8_t dayOfWeekFor(int year, int month, int day); // Sakamoto's algorithm
    void applyCompileTime(const char* compileDate, const char* compileTime); // shared parsing + setTime(), no lostPower check

    RtcTime lastGoodTime = {0, 0, 0, 1, 1, 1, 0}; // frozen fallback until the first successful read
    bool connected = false;
};
