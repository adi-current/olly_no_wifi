#include "PomodoroModule.h"
#include "gauge_render.h"
#include "UiKit.h"
#include "BuzzerModule.h"

extern BuzzerModule buzzer; // defined in main.cpp

void PomodoroModule::begin(U8G2* d) {
  display = d;
}

void PomodoroModule::beginPhase(Phase p) {
  phase = p;
  uint32_t dur = (p == Phase::WORK) ? WORK_SECONDS : BREAK_SECONDS;
  engine.start(dur);
  viewState = ViewState::RUNNING;
}

void PomodoroModule::start() {
  beginPhase(Phase::WORK);
}

const char* PomodoroModule::phaseLabel() const {
  return (phase == Phase::WORK) ? "WORK" : "BREAK";
}

bool PomodoroModule::update(bool actionPressed, bool backPressed) {
  if (backPressed) return true; // universal back, no confirm dialog

  if (viewState == ViewState::TRANSITION) {
    if (millis() - transitionStartMs >= TRANSITION_MS) {
      viewState = ViewState::RUNNING;
    }
    return false;
  }

  if (actionPressed) {
    if (engine.isRunning()) engine.pause();
    else if (engine.isPaused()) engine.resume();
  }

  if (engine.isFinished()) {
    buzzer.beepSuccess();
    Phase next = (phase == Phase::WORK) ? Phase::BREAK : Phase::WORK;
    beginPhase(next);
    viewState = ViewState::TRANSITION;
    transitionStartMs = millis();
  }

  return false;
}

void PomodoroModule::render() {
  if (!display) return;

  if (viewState == ViewState::TRANSITION) {
    display->setFont(u8g2_font_7x13B_tr);
    char buf[24];
    snprintf(buf, sizeof(buf), "%s TIME!", phaseLabel());
    int w = display->getStrWidth(buf);
    display->drawStr((128 - w) / 2, 36, buf);
    return;
  }

  uint32_t remaining = engine.remainingSeconds();
  uint32_t total = (phase == Phase::WORK) ? WORK_SECONDS : BREAK_SECONDS;
  float frac = (total > 0) ? (float)remaining / (float)total : 0;

  char timeStr[6];
  snprintf(timeStr, sizeof(timeStr), "%02lu:%02lu", (unsigned long)(remaining / 60), (unsigned long)(remaining % 60));

  int totalMinutes = (int)(total / 60);
  drawTimerGauge(display, frac, timeStr, phaseLabel(), totalMinutes, engine.isPaused() ? "PAUSED" : nullptr);
  drawHint(display, engine.isPaused() ? "Tap: Resume" : "Tap: Pause");

}
