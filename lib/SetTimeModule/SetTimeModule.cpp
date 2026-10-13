#include "SetTimeModule.h"
#include "gauge_render.h"

void SetTimeModule::begin(U8G2* d, RtcModule* r) {
  display = d;
  rtc = r;
}

void SetTimeModule::start() {
  if (rtc) {
    baseTime = rtc->now(); // captures current date/dayOfWeek/month/year to preserve
    editHour = baseTime.hour;
    editMinute = baseTime.minute;
  }
  state = State::EDIT_HOUR;
}

bool SetTimeModule::update(int rotationDelta, bool encoderClick, bool touchLong) {
  if (touchLong) return true; // cancel, no changes saved

  switch (state) {
    case State::EDIT_HOUR:
      if (rotationDelta != 0) {
        int v = (int)editHour + rotationDelta;
        v = ((v % 24) + 24) % 24; // wrap 0-23 both directions
        editHour = (uint8_t)v;
      }
      if (encoderClick) {
        state = State::EDIT_MINUTE;
      }
      return false;

    case State::EDIT_MINUTE:
      if (rotationDelta != 0) {
        int v = (int)editMinute + rotationDelta;
        v = ((v % 60) + 60) % 60; // wrap 0-59 both directions
        editMinute = (uint8_t)v;
      }
      if (encoderClick) {
        if (rtc) {
          RtcTime t = baseTime; // keeps date/dayOfWeek/month/year unchanged
          t.hour = editHour;
          t.minute = editMinute;
          t.second = 0;
          rtc->setTime(t); // also clears the Oscillator Stop Flag, marking this time as trustworthy
        }
        return true; // saved, back to menu
      }
      return false;
  }
  return false;
}

void SetTimeModule::render() {
  if (!display) return;

  display->setFont(u8g2_font_6x10_tr);
  const char* title = "Set Time";
  int tw = display->getStrWidth(title);
  display->drawStr((128 - tw) / 2, 10, title);

  char hourBuf[3], minBuf[3];
  snprintf(hourBuf, sizeof(hourBuf), "%02u", editHour);
  snprintf(minBuf, sizeof(minBuf), "%02u", editMinute);

  drawEditableTime(display, 42, hourBuf, minBuf, state == State::EDIT_HOUR ? 1 : 2);

  display->setFont(u8g2_font_6x10_tr);
  const char* hint = (state == State::EDIT_HOUR) ? "Turn:hour  Knob:next"
                                                  : "Turn:min  Knob:save";
  int hintW = display->getStrWidth(hint);
  display->drawStr((128 - hintW) / 2, 60, hint);
}
