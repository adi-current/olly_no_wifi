#include "BrightnessModule.h"
#include "UiKit.h"

void BrightnessModule::begin(U8G2* d, DisplayModule* dm) {
  display = d;
  displayModule = dm;
}

void BrightnessModule::start() {
  if (displayModule) {
    baselinePercent = displayModule->getBrightnessPercent();
    editPercent = baselinePercent;
  }
}

bool BrightnessModule::update(int rotationDelta, bool encoderClick, bool touchLong) {
  if (touchLong) {
    // Cancel - put brightness back exactly how it was found, no save.
    if (displayModule) displayModule->previewBrightnessPercent(baselinePercent);
    return true;
  }

  if (rotationDelta != 0) {
    int v = (int)editPercent + rotationDelta * STEP_PERCENT;
    if (v < 1) v = 1;     // previewBrightnessPercent also clamps, but keep editPercent in sync
    if (v > 100) v = 100;
    editPercent = (uint8_t)v;
    if (displayModule) displayModule->previewBrightnessPercent(editPercent); // live preview as you turn
  }

  if (encoderClick) {
    if (displayModule) displayModule->commitBrightnessPercent(editPercent); // save to NVS
    return true;
  }

  return false;
}

void BrightnessModule::render() {
  if (!display) return;

  display->setFont(u8g2_font_6x10_tr);
  const char* title = "Brightness";
  int tw = display->getStrWidth(title);
  display->drawStr((128 - tw) / 2, 10, title);

  char pctBuf[6];
  snprintf(pctBuf, sizeof(pctBuf), "%u%%", editPercent);
  display->setFont(u8g2_font_ncenB14_tr);
  int pw = display->getStrWidth(pctBuf);
  display->drawStr((128 - pw) / 2, 30, pctBuf);

  // Gauge bar - outline always full width, fill scales with the percentage
  const int barW = 100;
  const int barH = 10;
  const int barX = (128 - barW) / 2;
  const int barY = 38;
  display->drawFrame(barX, barY, barW, barH);
  int fillW = (int)((barW - 2) * (int)editPercent / 100);
  if (fillW > 0) {
    display->drawBox(barX + 1, barY + 1, fillW, barH - 2);
  }

  drawHint(display, "Turn: Adjust", "Press: Save");
}
