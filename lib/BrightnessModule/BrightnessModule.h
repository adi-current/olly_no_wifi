#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include "DisplayModule.h"

// Lets the user adjust the OLED's brightness (contrast) with live preview -
// the screen actually dims/brightens as you turn the encoder, same
// immediate-feedback idea as SetTimeModule's edit fields.
//
// encoder rotate -> adjust brightness by 5% per step, live (not yet saved)
// encoder click  -> confirm: save to NVS via DisplayModule, exit
// touch long-press -> cancel: revert to the value brightness had on entry, exit
class BrightnessModule {
  public:
    void begin(U8G2* display, DisplayModule* displayModule);

    // Call when selected from the menu - remembers the current brightness
    // as the baseline to revert to if the user cancels.
    void start();

    // Returns true when done (saved or cancelled) and the menu should take
    // back over.
    bool update(int rotationDelta, bool encoderClick, bool touchLong);

    void render();

  private:
    U8G2* display = nullptr;
    DisplayModule* displayModule = nullptr;

    uint8_t editPercent = 100;
    uint8_t baselinePercent = 100;

    static const int STEP_PERCENT = 5;
};
