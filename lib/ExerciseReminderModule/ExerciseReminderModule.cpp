#include "ExerciseReminderModule.h"
#include "UiKit.h"
#include "BuzzerModule.h"

extern BuzzerModule buzzer; // defined in main.cpp

static const char* PREFS_NAMESPACE = "olly_exrem";
static const char* PREFS_KEY_MIN = "min";

void ExerciseReminderModule::begin() {
  armed = false;
  prefs.begin(PREFS_NAMESPACE, true); // read-only
  intervalMinutes = prefs.getUShort(PREFS_KEY_MIN, 60); // 60 min default, before anything's ever been saved
  prefs.end();
}

void ExerciseReminderModule::saveInterval() {
  prefs.begin(PREFS_NAMESPACE, false); // read-write
  prefs.putUShort(PREFS_KEY_MIN, intervalMinutes);
  prefs.end();
}

void ExerciseReminderModule::toggleArmed() {
  armed = !armed;
  if (armed) {
    engine.start((uint32_t)intervalMinutes * 60UL);
  } else {
    engine.reset();
  }
}

bool ExerciseReminderModule::isArmed() const {
  return armed;
}

void ExerciseReminderModule::update() {
  if (!armed) return;
  if (engine.isFinished()) {
    fired = true;
    buzzer.beepAlert(); // fires once here, not in renderInterrupt() below -
                         // that gets called every loop while the interrupt
                         // screen is showing, which would retrigger the tone continuously
    engine.start((uint32_t)intervalMinutes * 60UL); // re-arm immediately for the next interval
  }
}

bool ExerciseReminderModule::checkAndClearFired() {
  if (fired) {
    fired = false;
    return true;
  }
  return false;
}

void ExerciseReminderModule::renderInterrupt(U8G2* display) {
  if (!display) return;
  // TODO: swap this text for a dedicated Olly expression (e.g. brows up,
  // insistent look) once brows/mouth are modularized in FaceModule - see
  // face_static_parts.h's note about that being the next planned step.
  display->setFont(u8g2_font_7x13B_tr);
  const char* msg = "STRETCH!";
  int w = display->getStrWidth(msg);
  display->drawStr((128 - w) / 2, 30, msg);

  display->setFont(u8g2_font_6x10_tr);
  const char* sub = "Take a quick break";
  int sw = display->getStrWidth(sub);
  display->drawStr((128 - sw) / 2, 46, sub);
}

void ExerciseReminderModule::start() {
  settingsState = SettingsState::VIEW;
}

bool ExerciseReminderModule::updateSettingsScreen(int rotationDelta, bool encoderClick, bool touchShort, bool touchLong) {
  if (touchLong) return true; // universal back, from either state - discards any in-progress edit, same as TimerModule/SetTimeModule

  switch (settingsState) {
    case SettingsState::VIEW:
      if (touchShort) {
        toggleArmed();
      } else if (encoderClick) {
        settingsState = SettingsState::EDIT_INTERVAL;
      }
      return false;

    case SettingsState::EDIT_INTERVAL:
      if (rotationDelta != 0) {
        int v = (int)intervalMinutes + rotationDelta * INTERVAL_STEP_MINUTES;
        if (v < MIN_INTERVAL_MINUTES) v = MIN_INTERVAL_MINUTES;
        if (v > MAX_INTERVAL_MINUTES) v = MAX_INTERVAL_MINUTES;
        intervalMinutes = (uint16_t)v;
      }
      if (encoderClick) {
        saveInterval(); // one flash write, only on confirm - not per tick
        if (armed) engine.start((uint32_t)intervalMinutes * 60UL); // re-arm now so the new interval takes effect immediately instead of waiting out the old one
        settingsState = SettingsState::VIEW;
      }
      return false;
  }
  return false;
}

void ExerciseReminderModule::renderSettingsScreen(U8G2* display) {
  if (!display) return;

  display->setFont(u8g2_font_6x10_tr);
  const char* title = "Exercise Reminder";
  int tw = display->getStrWidth(title);
  display->drawStr((128 - tw) / 2, 12, title);

  char statusBuf[24];
  snprintf(statusBuf, sizeof(statusBuf), "Status: %s", armed ? "Armed" : "Disarmed");
  int sw = display->getStrWidth(statusBuf);
  display->drawStr((128 - sw) / 2, 28, statusBuf);

  char intervalBuf[24];
  snprintf(intervalBuf, sizeof(intervalBuf), "Every %u min", intervalMinutes);
  display->setFont(settingsState == SettingsState::EDIT_INTERVAL ? u8g2_font_7x13B_tr : u8g2_font_6x10_tr);
  int iw = display->getStrWidth(intervalBuf);
  display->drawStr((128 - iw) / 2, 44, intervalBuf);

  if (settingsState == SettingsState::VIEW) drawHint(display, "Tap: On/Off",  "Press: Edit");
  else                                      drawHint(display, "Turn: Adjust", "Press: Save");
}
