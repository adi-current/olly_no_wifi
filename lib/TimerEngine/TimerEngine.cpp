#include "TimerEngine.h"

void TimerEngine::start(uint32_t durationSeconds) {
  durationSeconds_ = durationSeconds;
  startMillis_ = millis();
  accumulatedMs_ = 0;
  running_ = true;
  paused_ = false;
}

void TimerEngine::pause() {
  if (running_ && !paused_) {
    accumulatedMs_ += currentSegmentMs();
    paused_ = true;
  }
}

void TimerEngine::resume() {
  if (running_ && paused_) {
    startMillis_ = millis(); // fresh segment - accumulatedMs_ already banked the rest
    paused_ = false;
  }
}

void TimerEngine::reset() {
  running_ = false;
  paused_ = false;
  accumulatedMs_ = 0;
  durationSeconds_ = 0;
}

unsigned long TimerEngine::currentSegmentMs() const {
  if (!running_ || paused_) return 0;
  return millis() - startMillis_;
}

uint32_t TimerEngine::elapsedSeconds() const {
  unsigned long totalMs = accumulatedMs_ + currentSegmentMs();
  return totalMs / 1000;
}

uint32_t TimerEngine::elapsedMillis() const {
  return accumulatedMs_ + currentSegmentMs();
}

uint32_t TimerEngine::remainingSeconds() const {
  uint32_t elapsed = elapsedSeconds();
  if (elapsed >= durationSeconds_) return 0;
  return durationSeconds_ - elapsed;
}

bool TimerEngine::isRunning() const {
  return running_ && !paused_;
}

bool TimerEngine::isPaused() const {
  return running_ && paused_;
}

bool TimerEngine::isFinished() const {
  return durationSeconds_ > 0 && running_ && remainingSeconds() == 0;
}
