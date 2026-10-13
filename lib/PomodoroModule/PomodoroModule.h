#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include "TimerEngine.h"

// Fixed-cycle Pomodoro: 25min WORK <-> 5min BREAK, looping automatically
// until the user backs out. Full-screen while running, no face/strip.
//
// touch short press -> pause/resume toggle
// touch long press  -> back to menu (immediate, no confirm - matches Timer)
class PomodoroModule {
  public:
    void begin(U8G2* display);

    // Call when selected from the menu - always starts fresh on WORK.
    void start();

    // Returns true when the user wants to exit to the menu.
    bool update(bool actionPressed, bool backPressed);

    void render();

  private:
    U8G2* display = nullptr;
    TimerEngine engine;

    enum class Phase { WORK, BREAK };
    Phase phase = Phase::WORK;

    enum class ViewState { RUNNING, TRANSITION };
    ViewState viewState = ViewState::RUNNING;
    unsigned long transitionStartMs = 0;
    static const unsigned long TRANSITION_MS = 1800; // brief "WORK TIME!" / "BREAK TIME!" message between phases

    static const uint32_t WORK_SECONDS = 25UL * 60UL;
    static const uint32_t BREAK_SECONDS = 5UL * 60UL;

    void beginPhase(Phase p);
    const char* phaseLabel() const;
};
