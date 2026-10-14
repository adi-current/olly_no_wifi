#pragma once
#include <U8g2lib.h>

// Shared semicircle "speedometer" gauge for Pomodoro/Timer's countdown
// screens - a fixed arc across the top with small tick marks at 25/50/75%,
// numbered end-labels showing the total duration (scaled to whatever the
// phase actually is - e.g. 25/0 for a 25min Work phase, 5/0 for a 5min
// Break), a needle sweeping from full(left) to empty(right), the big
// MM:SS, and a phase caption at the bottom.
//
// fractionRemaining: 1.0 = just started (needle far left), 0.0 = done
// (needle far right). label may be "" for no caption. totalMinutes is the
// phase's full duration in minutes, used for the two end-tick numbers.
//
// Verified in a Python/PIL simulation before implementing here: every
// element's exact pixel bounds were checked to make sure nothing renders
// off-screen or overlaps another element (this replaced an earlier version
// that had a mountain-silhouette decoration competing with the needle for
// the same central space - dropped for a cleaner, better-balanced layout).
void drawTimerGauge(U8G2* d, float fractionRemaining, const char* timeText, const char* label, int totalMinutes, const char* status = nullptr);

// Big "LL:RR" editor line (Timer MM:SS, Set Time HH:MM). highlight: 0 = none,
// 1 = left field inverted, 2 = right field inverted. The colon has its own
// clear gap, so the inverted box never touches or clips it, and the box has
// even padding all round (it used to have ~1px on top and bite into the colon).
void drawEditableTime(U8G2* d, int baselineY, const char* left, const char* right, int highlight);
