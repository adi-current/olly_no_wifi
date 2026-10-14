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

v4: input grammar + hint footer across every screen
 Turn = choose, Press = go forward, Tap = start/pause/resume clocks, Hold = back.
 Footer: left slot = Turn or Tap, right slot = Press (always right-aligned).
 - Timer: Tap pauses/resumes (no longer cancels); Press dismisses TIME'S UP
 - Pomodoro/Timer gauge: phase top-left, PAUSED top-right, footer hint
 - Games + quotes/jokes: Press is the action; Coin result = Press again (no auto-exit)
 - Interrupt: Press dismisses
 - main.cpp (NEW in patch): beeps only in the menu; screens beep for presses they use

v5: games, coin, dice, quotes and jokes use touch Tap (Hold = back); Press stays for menus/settings.
 RPS shows turn arrows in the picture so Tap keeps the left footer slot everywhere.
