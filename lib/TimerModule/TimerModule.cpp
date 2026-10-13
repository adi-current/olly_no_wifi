#include "TimerModule.h"
#include "gauge_render.h"
#include "BuzzerModule.h"

extern BuzzerModule buzzer; // defined in main.cpp

static const char* PREFS_NAMESPACE = "olly_timer";
static const char* PREFS_KEY_MIN = "min";
static const char* PREFS_KEY_SEC = "sec";

void TimerModule::begin(U8G2* d) {
  display = d;
  loadSavedDuration();
}

void TimerModule::loadSavedDuration() {
  prefs.begin(PREFS_NAMESPACE, true); // read-only
  setMinutes = prefs.getUShort(PREFS_KEY_MIN, 10); // 10 min default, before anything's ever been saved
  setSeconds = prefs.getUChar(PREFS_KEY_SEC, 0);
  prefs.end();
}

void TimerModule::saveDuration() {
  prefs.begin(PREFS_NAMESPACE, false); // read-write
  prefs.putUShort(PREFS_KEY_MIN, setMinutes);
  prefs.putUChar(PREFS_KEY_SEC, setSeconds);
  prefs.end();
}

uint32_t TimerModule::totalSetSeconds() const {
  return (uint32_t)setMinutes * 60 + setSeconds;
}

void TimerModule::start() {
  state = State::IDLE; // show the remembered duration, ready to go
}

bool TimerModule::update(int rotationDelta, bool encoderClick, bool touchShort, bool touchLong) {
  if (touchLong) return true; // universal back, from any state

  switch (state) {
    case State::IDLE:
      if (touchShort && totalSetSeconds() > 0) {
        engine.start(totalSetSeconds());
        state = State::RUNNING;
      } else if (encoderClick) {
        state = State::EDIT_MINUTES;
      }
      return false;

    case State::EDIT_MINUTES:
      if (rotationDelta != 0) {
        int v = (int)setMinutes + rotationDelta;
        if (v < 0) v = 0;
        if (v > 99) v = 99;
        setMinutes = (uint16_t)v;
      }
      if (encoderClick) {
        state = State::EDIT_SECONDS;
      }
      return false;

    case State::EDIT_SECONDS:
      if (rotationDelta != 0) {
        int v = (int)setSeconds + rotationDelta;
        if (v < 0) v = 0;
        if (v > 59) v = 59;
        setSeconds = (uint8_t)v;
      }
      if (encoderClick) {
        saveDuration(); // one flash write, only on confirm - not per tick
        state = State::IDLE;
      }
      return false;

    case State::RUNNING:
      if (touchShort) {
        engine.reset();
        state = State::IDLE; // abort back to idle, same remembered duration ready to reuse
        return false;
      }
      if (engine.isFinished()) {
        buzzer.beepSuccess();
        state = State::COMPLETE;
        completeShownAtMs = millis();
      }
      return false;

    case State::COMPLETE:
      if (touchShort || millis() - completeShownAtMs >= COMPLETE_AUTO_RETURN_MS) {
        state = State::IDLE;
      }
      return false;
  }
  return false;
}

void TimerModule::render() {
  if (!display) return;

  if (state == State::RUNNING) {
    uint32_t remaining = engine.remainingSeconds();
    uint32_t total = totalSetSeconds();
    float frac = (total > 0) ? (float)remaining / (float)total : 0;
    char buf[6];
    snprintf(buf, sizeof(buf), "%02lu:%02lu", (unsigned long)(remaining / 60), (unsigned long)(remaining % 60));
    int totalMinutes = (int)(total / 60);
    drawTimerGauge(display, frac, buf, "TIMER", totalMinutes);
    return;
  }

  if (state == State::COMPLETE) {
    display->setFont(u8g2_font_7x13B_tr);
    const char* msg = "TIME'S UP!";
    int w = display->getStrWidth(msg);
    display->drawStr((128 - w) / 2, 36, msg);
    return;
  }

  // IDLE / EDIT_MINUTES / EDIT_SECONDS - big editable MM:SS
  char minBuf[3], secBuf[3];
  snprintf(minBuf, sizeof(minBuf), "%02u", setMinutes);
  snprintf(secBuf, sizeof(secBuf), "%02u", setSeconds);

  drawEditableTime(display, 40, minBuf, secBuf,
                   state == State::EDIT_MINUTES ? 1 : state == State::EDIT_SECONDS ? 2 : 0);

  display->setFont(u8g2_font_6x10_tr);
  const char* hint = (state == State::IDLE) ? "Tap:Start  Knob:Edit"
                     : (state == State::EDIT_MINUTES) ? "Rotate  Knob:Next"
                                                        : "Rotate  Knob:Save";
  int hw = display->getStrWidth(hint);
  display->drawStr((128 - hw) / 2, 58, hint);
}
