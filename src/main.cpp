#include <Arduino.h>
#include "DisplayModule.h"
#include "EncoderModule.h"
#include "TouchModule.h"
#include "GamesModule.h"
#include "FunModule.h"
#include "RtcModule.h"
#include "PomodoroModule.h"
#include "TimerModule.h"
#include "ExerciseReminderModule.h"
#include "StopwatchModule.h"
#include "SetTimeModule.h"
#include "BuzzerModule.h"
#include "BrightnessModule.h"

DisplayModule display;
EncoderModule encoder;
TouchModule touch;
GamesModule games;
FunModule fun;
RtcModule rtc;
PomodoroModule pomodoro;
TimerModule timerFeature;
ExerciseReminderModule exerciseReminder;
StopwatchModule stopwatch;
SetTimeModule setTimeFeature;
BuzzerModule buzzer;
BrightnessModule brightness;

// Which of Games/Fun's 5 items lives at each menu index, per menu_data.h's
// GAME_ITEMS order: {"Rock Paper Scissors","Coin Toss","Dice Roll",
// "Random Motivation","Random Joke"}. Indices 0-2 are games, 3-4 are fun.
//
// Which of the Timers group's 4 items lives at each menu index, per
// menu_data.h's TIMER_ITEMS order: {"Pomodoro","Timer","Exercise
// Reminder","Stopwatch"}.
enum class AppMode {
  HOME_MENU,
  GAME_ACTIVE,
  FUN_ACTIVE,
  POMODORO_ACTIVE,
  TIMER_ACTIVE,
  STOPWATCH_ACTIVE,
  EXERCISE_INTERRUPT,
  SET_TIME_ACTIVE,
  BRIGHTNESS_ACTIVE,
  EXERCISE_SETTINGS_ACTIVE
};
AppMode appMode = AppMode::HOME_MENU;

unsigned long lastClockReadMs = 0;
const unsigned long CLOCK_READ_INTERVAL_MS = 1000; // the clock only changes once a second - no need to hit the RTC over I2C every loop()

unsigned long exerciseInterruptStartMs = 0;
const unsigned long EXERCISE_INTERRUPT_MS = 4000; // auto-dismiss after this long, or on any touch

void setup() {
  Serial.begin(115200);

  display.begin(); // calls Wire.begin(SDA, SCL) - must happen before rtc.begin(), since the DS3231 shares this same I2C bus
  encoder.begin();
  touch.begin();
  games.begin(display.getRawDisplay());
  fun.begin(display.getRawDisplay());
  pomodoro.begin(display.getRawDisplay());
  timerFeature.begin(display.getRawDisplay());
  exerciseReminder.begin();
  stopwatch.begin(display.getRawDisplay());
  setTimeFeature.begin(display.getRawDisplay(), &rtc);
  buzzer.begin();
  brightness.begin(display.getRawDisplay(), &display);

  if (!rtc.begin()) {
    Serial.println("RTC not found - check DS3231 wiring (shares the OLED's SDA/SCL, GPIO8/9).");
  } else if (rtc.setToCompileTimeIfNeeded(__DATE__, __TIME__)) {
    Serial.println("RTC reported lost power - time set to this sketch's compile time.");
  } else {
    Serial.println("RTC time is trusted (battery held it) - left untouched.");
  }
}

void loop() {
  encoder.update();
  touch.update();
  buzzer.update(); // must run every loop() so timed beeps stop themselves

  int rotation = encoder.takeRotation();
  bool confirm = encoder.wasClicked();     // Press = go forward: confirm / next / roll / commit
  bool actionShort = touch.wasPressed();   // Tap = run control for clocks only: start / pause / resume
  bool backLong = touch.wasLongPressed();  // Hold = back / cancel, everywhere

  // Only the menu beeps here. Every other screen plays its own feedback when
  // (and only when) a press actually does something - a beep on a dead press lies.
  if (confirm && appMode == AppMode::HOME_MENU) buzzer.beepClick();

  unsigned long now = millis();
  if (now - lastClockReadMs >= CLOCK_READ_INTERVAL_MS) {
    lastClockReadMs = now;
    RtcTime t = rtc.now();
    display.setTime(t.hour, t.minute);
  }

  // Exercise Reminder runs in the background regardless of what else is
  // showing - it just doesn't get to interrupt unless Home is truly idle
  // (not mid-menu, not mid-game), to keep this simple for v1.
  exerciseReminder.update();
  display.setReminderArmed(exerciseReminder.isArmed());
  if (appMode == AppMode::HOME_MENU && !display.isMenuActive() && exerciseReminder.checkAndClearFired()) {
    appMode = AppMode::EXERCISE_INTERRUPT;
    exerciseInterruptStartMs = now;
  }

  if (appMode == AppMode::HOME_MENU) {
    bool featureSelected = display.update(rotation, confirm, backLong);

    if (featureSelected) {
      int group = display.selectedGroup(); // 0 = Timers, 1 = Games, 2 = Settings
      int item = display.selectedItem();

      if (group == 0) { // Timers group
        if (item == 0) {
          pomodoro.start();
          appMode = AppMode::POMODORO_ACTIVE;
        } else if (item == 1) {
          timerFeature.start();
          appMode = AppMode::TIMER_ACTIVE;
        } else if (item == 2) {
          exerciseReminder.start();
          appMode = AppMode::EXERCISE_SETTINGS_ACTIVE;
        } else if (item == 3) {
          stopwatch.start();
          appMode = AppMode::STOPWATCH_ACTIVE;
        }
      } else if (group == 1) { // Games group (also holds Motivation/Joke, per menu_data.h)
        if (item <= 2) {
          games.start((GameType)item);
          appMode = AppMode::GAME_ACTIVE;
        } else {
          fun.start(item == 3 ? FunType::MOTIVATION : FunType::JOKE);
          appMode = AppMode::FUN_ACTIVE;
        }
      } else if (group == 2) { // Settings group
        if (item == 0) { // "Brightness"
          brightness.start();
          appMode = AppMode::BRIGHTNESS_ACTIVE;
        } else if (item == 2) { // "Set Time" - per menu_data.h's SETTINGS_ITEMS order
          setTimeFeature.start();
          appMode = AppMode::SET_TIME_ACTIVE;
        } else if (item == 3) {
          // Background toggle, same pattern as Exercise Reminder - flips
          // and saves immediately, stays on the Feature list.
          display.toggleClockFormat();
        }
        // item 1 (Volume): not built yet - stays on the Feature list.
      }
    }

  } else if (appMode == AppMode::GAME_ACTIVE) {
    bool done = games.update(rotation, confirm, backLong);
    U8G2* raw = display.getRawDisplay();
    raw->clearBuffer();
    games.render();
    raw->sendBuffer();
    if (done) appMode = AppMode::HOME_MENU;

  } else if (appMode == AppMode::FUN_ACTIVE) {
    bool done = fun.update(rotation, confirm, backLong);
    U8G2* raw = display.getRawDisplay();
    raw->clearBuffer();
    fun.render();
    raw->sendBuffer();
    if (done) appMode = AppMode::HOME_MENU;

  } else if (appMode == AppMode::POMODORO_ACTIVE) {
    bool done = pomodoro.update(actionShort, backLong);
    U8G2* raw = display.getRawDisplay();
    raw->clearBuffer();
    pomodoro.render();
    raw->sendBuffer();
    if (done) appMode = AppMode::HOME_MENU;

  } else if (appMode == AppMode::TIMER_ACTIVE) {
    bool done = timerFeature.update(rotation, confirm, actionShort, backLong);
    U8G2* raw = display.getRawDisplay();
    raw->clearBuffer();
    timerFeature.render();
    raw->sendBuffer();
    if (done) appMode = AppMode::HOME_MENU;

  } else if (appMode == AppMode::STOPWATCH_ACTIVE) {
    bool done = stopwatch.update(actionShort, backLong);
    U8G2* raw = display.getRawDisplay();
    raw->clearBuffer();
    stopwatch.render();
    raw->sendBuffer();
    if (done) appMode = AppMode::HOME_MENU;

  } else if (appMode == AppMode::SET_TIME_ACTIVE) {
    bool done = setTimeFeature.update(rotation, confirm, backLong);
    U8G2* raw = display.getRawDisplay();
    raw->clearBuffer();
    setTimeFeature.render();
    raw->sendBuffer();
    if (done) appMode = AppMode::HOME_MENU;

  } else if (appMode == AppMode::BRIGHTNESS_ACTIVE) {
    bool done = brightness.update(rotation, confirm, backLong);
    U8G2* raw = display.getRawDisplay();
    raw->clearBuffer();
    brightness.render();
    raw->sendBuffer();
    if (done) appMode = AppMode::HOME_MENU;

  } else if (appMode == AppMode::EXERCISE_SETTINGS_ACTIVE) {
    bool done = exerciseReminder.updateSettingsScreen(rotation, confirm, actionShort, backLong);
    U8G2* raw = display.getRawDisplay();
    raw->clearBuffer();
    exerciseReminder.renderSettingsScreen(raw);
    raw->sendBuffer();
    if (done) appMode = AppMode::HOME_MENU;

  } else if (appMode == AppMode::EXERCISE_INTERRUPT) {
    U8G2* raw = display.getRawDisplay();
    raw->clearBuffer();
    exerciseReminder.renderInterrupt(raw);
    raw->sendBuffer();

    if (confirm || backLong || now - exerciseInterruptStartMs >= EXERCISE_INTERRUPT_MS) {
      appMode = AppMode::HOME_MENU;
    }
  }
}
