Olly graphics patch v2 - changed/new files only (copy over your lib/ folders).

NEW  lib/UiKit/UiKit.h   drawHint(): thin rule + 5x7 footer. Vocabulary: Turn / Press / Tap.
Home strip: plain AM/PM text beside the minutes.
Footer hints on: Timer, Set Time, Exercise, Brightness, Stopwatch, Quotes/Jokes, RPS, Coin, Dice.
Plus earlier fixes: gauge rewrite, menu counter/scrollbar, edit-field highlight.

v3: Exercise Reminder rebuilt (lib/ExerciseReminderModule/*.h and .cpp)
 - presets 15/30/45/60/90/120; Turn = pick (preview), Press = start/apply, Tap = stop/start, long touch = back
 - live "Next break mm:ss" countdown; interval AND armed state saved to flash
 - fixes: "back" now really discards; reminder no longer silently disarms after reboot
 - main.cpp is unchanged (same public API)
