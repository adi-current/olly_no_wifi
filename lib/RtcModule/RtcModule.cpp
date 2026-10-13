#include "RtcModule.h"
#include <Wire.h>

uint8_t RtcModule::bcdToDec(uint8_t bcd) {
  return ((bcd / 16 * 10) + (bcd % 16));
}

uint8_t RtcModule::decToBcd(uint8_t dec) {
  return ((dec / 10 * 16) + (dec % 10));
}

uint8_t RtcModule::readRegister(uint8_t reg) {
  Wire.beginTransmission(DS3231_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false); // repeated start - keep the bus held for the read
  Wire.requestFrom((int)DS3231_ADDR, 1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0;
}

void RtcModule::writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(DS3231_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

bool RtcModule::begin() {
  // Just check the DS3231 acknowledges on the bus - no data needed yet.
  Wire.beginTransmission(DS3231_ADDR);
  uint8_t error = Wire.endTransmission();
  if (error != 0) return false;

  // Ensure the oscillator keeps running on battery backup (VBAT) power.
  // The Control register's EOSC bit (bit7) STOPS the oscillator the
  // instant main power is removed, if it's set to 1 - regardless of how
  // good the battery is. This bit's default state varies by chip batch,
  // and we never touched this register before, so if it happened to be
  // set, the RTC would appear to freeze completely every time main power
  // was disconnected, always resuming from the same stuck point on
  // reconnect - which matches this exact symptom. Force it to 0 (enabled)
  // every boot, just in case.
  uint8_t control = readRegister(0x0E);
  if (control & 0x80) {
    control &= ~0x80;
    writeRegister(0x0E, control);
  }

  return true;
}

RtcTime RtcModule::now() {
  Wire.beginTransmission(DS3231_ADDR);
  Wire.write(0x00); // start reading from the seconds register
  Wire.endTransmission(false);
  uint8_t received = Wire.requestFrom((int)DS3231_ADDR, 7);

  if (received < 7) {
    // The DS3231 didn't actually answer (disconnected, bad wiring, power
    // issue). Whatever Wire.read() would return here is undefined/leftover
    // data, not real time bytes - decoding it can produce nonsense values
    // (e.g. an hour outside 0-23), which is exactly what causes a garbled
    // display. Freeze on the last known-good time instead of showing that.
    connected = false;
    return lastGoodTime;
  }

  uint8_t sec   = Wire.read();
  uint8_t min   = Wire.read();
  uint8_t hour  = Wire.read();
  uint8_t dow   = Wire.read();
  uint8_t date  = Wire.read();
  uint8_t month = Wire.read();
  uint8_t year  = Wire.read();

  RtcTime t;
  t.second     = bcdToDec(sec & 0x7F);
  t.minute     = bcdToDec(min & 0x7F);
  t.hour       = bcdToDec(hour & 0x3F);  // mask off 12/24-hour mode bits - we always run in 24hr mode
  t.dayOfWeek  = bcdToDec(dow & 0x07);
  t.date       = bcdToDec(date & 0x3F);
  t.month      = bcdToDec(month & 0x1F); // mask off the century bit, unused here
  t.year       = bcdToDec(year);

  connected = true;
  lastGoodTime = t;

  return t;
}

bool RtcModule::isConnected() const {
  return connected;
}

void RtcModule::setTime(const RtcTime& t) {
  Wire.beginTransmission(DS3231_ADDR);
  Wire.write(0x00); // start writing from the seconds register
  Wire.write(decToBcd(t.second));
  Wire.write(decToBcd(t.minute));
  Wire.write(decToBcd(t.hour)); // bit6=0 -> 24-hour mode, since valid hours (0-23) never set that bit
  Wire.write(decToBcd(t.dayOfWeek));
  Wire.write(decToBcd(t.date));
  Wire.write(decToBcd(t.month));
  Wire.write(decToBcd(t.year));
  Wire.endTransmission();

  // Clear the Oscillator Stop Flag now that we've set a known-good time -
  // otherwise lostPower() stays permanently true from here on, and every
  // future reboot re-triggers setToCompileTimeIfNeeded(), stamping the
  // clock back to this exact fixed compile-time value again and again,
  // discarding whatever real time the RTC (and its backup battery) kept
  // correctly in between.
  uint8_t status = readRegister(0x0F);
  status &= ~0x80; // bit7 = OSF, clear it
  writeRegister(0x0F, status);
}

bool RtcModule::lostPower() {
  uint8_t status = readRegister(0x0F);
  return (status & 0x80) != 0; // bit7 = Oscillator Stop Flag
}

uint8_t RtcModule::dayOfWeekFor(int year, int month, int day) {
  // Sakamoto's algorithm - returns 0=Sunday..6=Saturday, remapped below to
  // the DS3231's 1-7 (1=Sunday) convention.
  static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (month < 3) year -= 1;
  int dow = (year + year / 4 - year / 100 + year / 400 + t[month - 1] + day) % 7;
  return dow + 1; // 1-7, 1=Sunday
}

bool RtcModule::setToCompileTimeIfNeeded(const char* compileDate, const char* compileTime) {
  if (!lostPower()) {
    return false; // battery-backed time is still good - leave it alone
  }
  applyCompileTime(compileDate, compileTime);
  return true;
}

void RtcModule::setToCompileTimeAlways(const char* compileDate, const char* compileTime) {
  applyCompileTime(compileDate, compileTime);
}

void RtcModule::applyCompileTime(const char* compileDate, const char* compileTime) {
  // __DATE__ looks like "Nov 21 2025", __TIME__ like "14:32:07"
  static const char* const MONTHS[] = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
  };
  char monthStr[4] = {0};
  int day, year, hour, minute, second;
  sscanf(compileDate, "%3s %d %d", monthStr, &day, &year);
  sscanf(compileTime, "%d:%d:%d", &hour, &minute, &second);

  int month = 1;
  for (int i = 0; i < 12; i++) {
    if (strncmp(monthStr, MONTHS[i], 3) == 0) {
      month = i + 1;
      break;
    }
  }

  RtcTime t;
  t.second = (uint8_t)second;
  t.minute = (uint8_t)minute;
  t.hour = (uint8_t)hour;
  t.date = (uint8_t)day;
  t.month = (uint8_t)month;
  t.year = (uint8_t)(year - 2000);
  t.dayOfWeek = dayOfWeekFor(year, month, day);

  setTime(t);
}
