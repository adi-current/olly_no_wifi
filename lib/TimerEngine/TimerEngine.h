#pragma once
#include <Arduino.h>

// Minimal, UI-agnostic elapsed-time primitive shared by all 4 timer
// features (Pomodoro, Timer, Exercise Reminder, Stopwatch). Doesn't know
// about phases, screens, or persistence - just tracks running/paused
// elapsed time via millis(), non-blocking (no delay() calls).
//
// Countdown use (Timer, Pomodoro, Exercise Reminder):
//   start(durationSeconds), then read remainingSeconds() each frame;
//   isFinished() flips true once it hits zero.
// Count-up use (Stopwatch):
//   start(0), then read elapsedSeconds() each frame; remainingSeconds()/
//   isFinished() aren't meaningful in this mode.
class TimerEngine {
  public:
    void start(uint32_t durationSeconds); // 0 = count-up (stopwatch) mode
    void pause();
    void resume();
    void reset(); // stops and clears back to zero/idle

    uint32_t elapsedSeconds() const;
    uint32_t elapsedMillis() const; // full sub-second precision - for displays like Stopwatch's MM.SS.CC
    uint32_t remainingSeconds() const; // only meaningful if started with duration>0

    bool isRunning() const;
    bool isPaused() const;
    bool isFinished() const; // countdown mode only - true once remaining==0

  private:
    uint32_t durationSeconds_ = 0;
    unsigned long startMillis_ = 0;
    unsigned long accumulatedMs_ = 0; // time banked from previous run/pause segments
    bool running_ = false;
    bool paused_ = false;

    unsigned long currentSegmentMs() const;
};
