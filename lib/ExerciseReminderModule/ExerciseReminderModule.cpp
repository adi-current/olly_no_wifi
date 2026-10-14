#include "ExerciseReminderModule.h"
#include "BuzzerModule.h"
#include "UiKit.h"

extern BuzzerModule buzzer; // defined in main.cpp

static const char* PREFS_NAMESPACE = "olly_exrem";
static const char* PREFS_KEY_MIN   = "min";
static const char* PREFS_KEY_ARMED = "armed";

// Realistic break intervals. Fewer, meaningful choices = fewer clicks than
// stepping 5 minutes at a time (60 -> 120 used to take 12 clicks, now 2).
static const uint16_t PRESETS[] = {15, 30, 45, 60, 90, 120};
static const uint8_t  PRESET_COUNT = sizeof(PRESETS) / sizeof(PRESETS[0]);

static uint8_t nearestPresetIndex(uint16_t minutes) {
  uint8_t best = 0;
  int bestDiff = 9999;
  for (uint8_t i = 0; i < PRESET_COUNT; i++) {
    int diff = abs((int)PRESETS[i] - (int)minutes);
    if (diff < bestDiff) { bestDiff = diff; best = i; }
  }
  return best;
}

void ExerciseReminderModule::begin() {
  prefs.begin(PREFS_NAMESPACE, true); // read-only
  intervalMinutes = prefs.getUShort(PREFS_KEY_MIN, 60); // 60 min default, before anything's ever been saved
  armed = prefs.getBool(PREFS_KEY_ARMED, false);
  prefs.end();
  if (armed) engine.start((uint32_t)intervalMinutes * 60UL);
}

void ExerciseReminderModule::saveSettings() {
  prefs.begin(PREFS_NAMESPACE, false); // read-write
  prefs.putUShort(PREFS_KEY_MIN, intervalMinutes);
  prefs.putBool(PREFS_KEY_ARMED, armed);
  prefs.end();
}

void ExerciseReminderModule::startReminder(uint16_t minutes) {
  intervalMinutes = minutes;
  armed = true;
  engine.start((uint32_t)intervalMinutes * 60UL);
  saveSettings(); // only on commit - never per rotation tick
}

void ExerciseReminderModule::stopReminder() {
  armed = false;
  engine.reset();
  saveSettings();
}

void ExerciseReminderModule::toggleArmed() {
  if (armed) stopReminder();
  else       startReminder(intervalMinutes);
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
  drawHint(display, nullptr, "Press: Done");
}

void ExerciseReminderModule::start() {
  selIndex = nearestPresetIndex(intervalMinutes);
}

bool ExerciseReminderModule::updateSettingsScreen(int rotationDelta, bool encoderClick, bool /*touchShort*/, bool touchLong) {
  // Back. selIndex is only a preview - nothing was committed - so there is
  // nothing to revert. (The old version changed the live interval on every
  // turn, so "back" did not actually discard anything.)
  if (touchLong) return true;

  if (rotationDelta != 0) {
    int i = (int)selIndex + rotationDelta;
    if (i < 0) i = 0;
    if (i > PRESET_COUNT - 1) i = PRESET_COUNT - 1; // hard stops, no wrap - the end arrows disappear to show it
    selIndex = (uint8_t)i;
  }

  if (encoderClick) {
    // One button, one verb that the footer always spells out:
    //   stopped -> Start | running + different interval -> Apply | running + same -> Stop
    if (!armed || PRESETS[selIndex] != intervalMinutes) {
      startReminder(PRESETS[selIndex]);
      buzzer.beepSuccess();
    } else {
      stopReminder();
      buzzer.beepClick();
    }
  }
  // A short touch deliberately does nothing here: Press is the only action,
  // so each control has exactly one job and the footer tells the whole story.
  return false;
}

void ExerciseReminderModule::renderSettingsScreen(U8G2* display) {
  if (!display) return;

  // Title
  display->setFont(u8g2_font_6x10_tr);
  const char* title = "Exercise Reminder";
  display->drawStr((128 - display->getStrWidth(title)) / 2, 10, title);

  // The value you are choosing, big, flanked by arrows that show which way
  // you can still turn (an arrow vanishes at the end of the list).
  char valBuf[12];
  snprintf(valBuf, sizeof(valBuf), "%u min", PRESETS[selIndex]);
  display->setFont(u8g2_font_ncenB14_tr);
  display->drawStr((128 - display->getStrWidth(valBuf)) / 2, 36, valBuf);
  if (selIndex > 0)                display->drawTriangle(12, 29, 19, 24, 19, 34);
  if (selIndex < PRESET_COUNT - 1) display->drawTriangle(115, 29, 108, 24, 108, 34);

  // Status: a live countdown is the proof it is working
  char statusBuf[28];
  if (armed) {
    uint32_t rem = engine.remainingSeconds();
    if (rem >= 3600) snprintf(statusBuf, sizeof(statusBuf), "Next break %lu:%02lu:%02lu", (unsigned long)(rem / 3600), (unsigned long)((rem / 60) % 60), (unsigned long)(rem % 60));
    else             snprintf(statusBuf, sizeof(statusBuf), "Next break %02lu:%02lu", (unsigned long)(rem / 60), (unsigned long)(rem % 60));
  } else {
    snprintf(statusBuf, sizeof(statusBuf), "Reminder is off");
  }
  display->setFont(u8g2_font_6x10_tr);
  display->drawStr((128 - display->getStrWidth(statusBuf)) / 2, 49, statusBuf);

  // Fixed slots: Turn always on the left, Press always on the right; only the
  // Press verb changes, and it always says what the press will actually do.
  const char* act = !armed ? "Press: Start"
                  : (PRESETS[selIndex] != intervalMinutes ? "Press: Apply" : "Press: Stop");
  drawHint(display, "Turn: Pick", act);
}
