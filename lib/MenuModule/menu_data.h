#pragma once
#include <Arduino.h>

// Menu content - just data, no logic. Matches timer_configs.h's philosophy:
// keep what's displayed separate from how navigation/rendering works, so
// adding/renaming/reordering features never touches MenuModule itself.

static const char* const GROUP_NAMES[] = { "Timers", "Games", "Settings" };
static const int GROUP_COUNT = 3;

static const char* const TIMER_ITEMS[] = {
  "Pomodoro", "Timer", "Exercise Reminder", "Stopwatch"
};

static const char* const GAME_ITEMS[] = {
  "Rock Paper Scissors", "Coin Toss", "Dice Roll",
  "Random Motivation", "Random Joke"
};

// "Clock Format" needs to show which mode it's currently in (e.g. "Clock
// Format: 24h") rather than just being a static label - since it's an
// in-place toggle (no dedicated screen), the menu item text is the only
// feedback the user gets that anything changed. It lives in a mutable
// buffer so its *contents* can be rewritten by updateClockFormatLabel()
// while the pointer itself (what SETTINGS_ITEMS actually holds) stays
// fixed - MenuModule still just renders whatever string is at that address,
// same as any other item, so it doesn't need to know this one is dynamic.
static char clockFormatLabel[24] = "Clock Format: 24h";
static inline void updateClockFormatLabel(bool use24h) {
  snprintf(clockFormatLabel, sizeof(clockFormatLabel), "Clock Format: %s", use24h ? "24h" : "12h");
}

static const char* const SETTINGS_ITEMS[] = {
  "Brightness", "Volume", "Set Time", clockFormatLabel
  // more to come later
};

static const char* const* const GROUP_ITEMS[] = { TIMER_ITEMS, GAME_ITEMS, SETTINGS_ITEMS };
static const int GROUP_ITEM_COUNTS[] = {
  sizeof(TIMER_ITEMS) / sizeof(TIMER_ITEMS[0]),
  sizeof(GAME_ITEMS) / sizeof(GAME_ITEMS[0]),
  sizeof(SETTINGS_ITEMS) / sizeof(SETTINGS_ITEMS[0])
};
