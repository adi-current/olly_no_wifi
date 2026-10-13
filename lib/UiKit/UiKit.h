#pragma once
#include <Arduino.h>
#include <U8g2lib.h>

// Shared on-screen hint footer for Olly.
//
// One look everywhere: a thin full-width rule at y=54, then small 5x7 text
// on the bottom row. Inverted (white-on-black) drawing is reserved for
// selection highlights, so a hint never looks like something you can select.
//
// Fixed vocabulary - one word per physical input, never mixed:
//   Turn  = rotate the encoder
//   Press = push the encoder
//   Tap   = touch pad
//
// Usage:  drawHint(d, "Turn: Min", "Press: Save");   // left cell + right cell
//         drawHint(d, "Tap: Start");                  // single hint, centred
//
// Screen content must end above UI_HINT_TOP (row 53 is the last free row).
#define UI_HINT_TOP 54

inline void drawHint(U8G2* d, const char* left, const char* right = nullptr) {
  d->setDrawColor(1);
  d->drawHLine(0, UI_HINT_TOP, 128);
  d->setFont(u8g2_font_5x7_tr);
  if (right) {
    d->drawStr(2, 63, left);
    d->drawStr(126 - d->getStrWidth(right), 63, right);
  } else {
    d->drawStr((128 - d->getStrWidth(left)) / 2, 63, left);
  }
}
