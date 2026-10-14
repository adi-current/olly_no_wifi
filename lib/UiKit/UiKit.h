#pragma once
#include <Arduino.h>
#include <U8g2lib.h>

// Shared on-screen hint footer for Olly.
//
// One look everywhere: a thin full-width rule at y=54, then small 5x7 text
// on the bottom row. Inverted (white-on-black) drawing is reserved for
// selection highlights, so a hint never looks like something you can select.
//
// Controls, and what each one always means:
//   Turn  = choose / change a value
//   Press = go forward in menus and settings: confirm, next, save
//   Tap   = the action on play and content screens: start / pause / resume,
//           flip, roll, pick, next quote or joke
//   Hold  = back / cancel (not shown in the footer)
//
// Fixed slots, so you always know where to look:
//   LEFT  = Turn or Tap (whichever the screen uses; Tap is always left)
//   RIGHT = Press - right-aligned even when it is the only hint
//   (A screen that turns AND taps shows a turn cue in the picture itself,
//   e.g. the arrows on Rock Paper Scissors, so Tap keeps the left slot.)
//
// Usage:  drawHint(d, "Turn: Min", "Press: Save");   // both slots
//         drawHint(d, "Tap: Pause");                  // left only
//         drawHint(d, nullptr, "Press: Next");        // right only
//
// Screen content must end above UI_HINT_TOP (row 53 is the last free row).
#define UI_HINT_TOP 54

inline void drawHint(U8G2* d, const char* left, const char* right = nullptr) {
  d->setDrawColor(1);
  d->drawHLine(0, UI_HINT_TOP, 128);
  d->setFont(u8g2_font_5x7_tr);
  if (left)  d->drawStr(2, 63, left);
  if (right) d->drawStr(126 - d->getStrWidth(right), 63, right);
}
