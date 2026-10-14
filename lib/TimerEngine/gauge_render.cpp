#include "gauge_render.h"
#include <math.h>
#include <stdio.h>

// Semicircle gauge layout (128x64). Everything is derived from these three
// numbers, so the whole gauge can be nudged by editing them.
static const int G_CX = 64;   // pivot / arc centre
static const int G_CY = 31;   // pivot height - arc sweeps above it
static const int G_R  = 27;   // outer radius of the arc

// Point on a circle of radius r. deg 0 = far left, 90 = top, 180 = far right.
// lroundf() (not a plain int cast) so the arc doesn't shrink ~1px inward.
static inline int ptX(float r, float deg) { return G_CX - lroundf(r * cosf(deg * (float)PI / 180.0f)); }
static inline int ptY(float r, float deg) { return G_CY - lroundf(r * sinf(deg * (float)PI / 180.0f)); }

// Solid arc between two angles, drawn as short connected line segments
// (the old version plotted single pixels every 3 degrees = a dotted arc).
static void drawArc(U8G2* d, float r, float fromDeg, float toDeg) {
  int px = ptX(r, fromDeg), py = ptY(r, fromDeg);
  for (float a = fromDeg + 3; a < toDeg + 3; a += 3) {
    float deg = (a > toDeg) ? toDeg : a;
    int x = ptX(r, deg), y = ptY(r, deg);
    d->drawLine(px, py, x, y);
    px = x; py = y;
  }
}

void drawTimerGauge(U8G2* d, float fractionRemaining, const char* timeText, const char* label, int totalMinutes, const char* status) {
  if (fractionRemaining < 0) fractionRemaining = 0;
  if (fractionRemaining > 1) fractionRemaining = 1;

  // Scale runs left = total (just started) -> right = 0 (done), matching the
  // end labels. The needle therefore sits at the *elapsed* position.
  // (Previously it used 180*fractionRemaining, so it started at the "0" end
  // and swept towards the total - backwards relative to the labels.)
  float needleDeg = 180.0f * (1.0f - fractionRemaining);

  // Thin track along the whole sweep, thick band over the part still to go
  // (needle -> right end). The thick band shrinks as time runs out.
  drawArc(d, G_R, 0, 180);
  drawArc(d, G_R - 1, needleDeg, 180);
  drawArc(d, G_R - 2, needleDeg, 180);

  // Unlabelled ticks at 25/50/75%, on the inside of the band
  for (int i = 1; i <= 3; i++) {
    float deg = 45.0f * i;
    d->drawLine(ptX(G_R - 4, deg), ptY(G_R - 4, deg), ptX(G_R - 7, deg), ptY(G_R - 7, deg));
  }

  // End labels: total minutes on the left, 0 on the right
  d->setFont(u8g2_font_5x7_tr);
  char maxBuf[4];
  snprintf(maxBuf, sizeof(maxBuf), "%d", totalMinutes);
  d->drawStr(G_CX - G_R - 3 - d->getStrWidth(maxBuf), G_CY + 3, maxBuf);
  d->drawStr(G_CX + G_R + 3, G_CY + 3, "0");

  // Needle + pivot
  d->drawLine(G_CX, G_CY, ptX(G_R - 9, needleDeg), ptY(G_R - 9, needleDeg));
  d->drawDisc(G_CX, G_CY, 2);

  // Time sits fully BELOW the pivot, so nothing draws on top of it
  // (the old text baseline was at the pivot's height, so the digits, needle
  // and pivot all overprinted each other).
  d->setFont(u8g2_font_ncenB14_tr);
  d->drawStr((128 - d->getStrWidth(timeText)) / 2, 51, timeText); // clear of the hint footer (row 54+)

  // Corners are free (the arc never reaches them): what this is, top-left;
  // its state (e.g. PAUSED), top-right. The bottom row belongs to the footer.
  d->setFont(u8g2_font_5x7_tr);
  if (label  && label[0]  != '\0') d->drawStr(2, 8, label);
  if (status && status[0] != '\0') d->drawStr(126 - d->getStrWidth(status), 8, status);
}

void drawEditableTime(U8G2* d, int baselineY, const char* left, const char* right, int highlight) {
  d->setFont(u8g2_font_ncenB18_tr);
  const int GAP = 5; // clear space each side of the colon
  int lw = d->getStrWidth(left), cw = d->getStrWidth(":"), rw = d->getStrWidth(right);
  int a  = d->getAscent();
  int lx = (128 - (lw + GAP + cw + GAP + rw)) / 2;
  int cx = lx + lw + GAP;
  int rx = cx + cw + GAP;

  for (int f = 1; f <= 2; f++) {
    int fx = (f == 1) ? lx : rx;
    int fw = (f == 1) ? lw : rw;
    if (highlight == f) {
      d->setDrawColor(1);
      d->drawRBox(fx - 3, baselineY - a - 3, fw + 6, a + 7, 2);
      d->setDrawColor(0);
    }
    d->drawStr(fx, baselineY, (f == 1) ? left : right);
    d->setDrawColor(1);
  }
  d->drawStr(cx, baselineY, ":");
}
