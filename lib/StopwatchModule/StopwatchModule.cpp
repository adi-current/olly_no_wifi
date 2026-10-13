#include "StopwatchModule.h"

void StopwatchModule::begin(U8G2* d) {
  display = d;
}

void StopwatchModule::start() {
  engine.reset();
  everStarted = false;
}

bool StopwatchModule::update(bool actionPressed, bool backPressed) {
  if (backPressed) return true;

  if (actionPressed) {
    if (!everStarted) {
      engine.start(0); // 0 = count-up mode
      everStarted = true;
    } else if (engine.isRunning()) {
      engine.pause();
    } else if (engine.isPaused()) {
      engine.resume();
    }
  }
  return false;
}

void StopwatchModule::render() {
  if (!display) return;

  uint32_t totalMs = engine.elapsedMillis();
  uint32_t mins = totalMs / 60000;
  uint32_t secs = (totalMs / 1000) % 60;
  uint32_t hundredths = (totalMs % 1000) / 10;
  char buf[12];

  if (mins >= 100) {
    // over 99 minutes - fall back to H:MM:SS, dropping hundredths (less
    // meaningful at that scale, and keeps the string a sane length)
    uint32_t hrs = mins / 60;
    uint32_t minsPart = mins % 60;
    snprintf(buf, sizeof(buf), "%lu:%02lu:%02lu", (unsigned long)hrs, (unsigned long)minsPart, (unsigned long)secs);
  } else {
    snprintf(buf, sizeof(buf), "%02lu.%02lu.%02lu", (unsigned long)mins, (unsigned long)secs, (unsigned long)hundredths);
  }

  display->setFont(u8g2_font_ncenB18_tr);
  int w = display->getStrWidth(buf);
  display->drawStr((128 - w) / 2, 40, buf);

  display->setFont(u8g2_font_6x10_tr);
  const char* hint;
  if (!everStarted) hint = "Tap to start";
  else if (engine.isRunning()) hint = "Tap to pause";
  else hint = "Tap to resume";
  int hw = display->getStrWidth(hint);
  display->drawStr((128 - hw) / 2, 58, hint);
}
