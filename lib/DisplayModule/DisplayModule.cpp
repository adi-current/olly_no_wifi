
#include "DisplayModule.h"
#include "config.h"

static const char* PREFS_NAMESPACE = "olly_disp";
static const char* PREFS_KEY_24H = "use24h";
static const char* PREFS_KEY_BRIGHTNESS = "bright";

void DisplayModule::begin() {
  Wire.begin(DISPLAY_SDA_PIN, DISPLAY_SCL_PIN);
  display.begin();
  display.setFont(u8g2_font_ncenB08_tr);
  face.begin(&display, STRIP_WIDTH, 0); // face origin sits right of the icon strip
  menu.begin(&display);

  prefs.begin(PREFS_NAMESPACE, true); // read-only
  use24Hour = prefs.getBool(PREFS_KEY_24H, true);       // 24h default, matches the original behavior
  brightnessPercent = prefs.getUChar(PREFS_KEY_BRIGHTNESS, 100); // full brightness default
  prefs.end();
  menu.setClockFormatLabel(use24Hour); // menu item text reflects whatever was loaded, not just the buffer's compiled-in default

  uint8_t contrast = (uint16_t)brightnessPercent * 255 / 100;
  display.setContrast(contrast);
}

bool DisplayModule::update(int rotationDelta, bool confirmPressed, bool backPressed) {
  display.clearBuffer();

  bool featureSelected = menu.update(rotationDelta, confirmPressed, backPressed);

  if (menu.isActive()) {
    // Menu (Group or Feature list) owns the whole screen while open -
    // Home's face/strip are not drawn underneath it.
    menu.render();
  } else {
    // --- Home: status strip (32px) + face, exactly as before ---
    display.drawVLine(STRIP_WIDTH - 1, 0, 64); // divider line, right edge of strip

    // WiFi icon, centered horizontally in the strip, near the top
    // Swaps automatically between connected/disconnected glyphs based on state
    if (wifiConnected) {
      int wifiX = (STRIP_WIDTH - WIFI_ICON_WIDTH) / 2;
      display.drawXBMP(wifiX, 3, WIFI_ICON_WIDTH, WIFI_ICON_HEIGHT, wifi_icon);
    } else {
      int wifiX = (STRIP_WIDTH - WIFI_OFF_ICON_WIDTH) / 2;
      display.drawXBMP(wifiX, 3, WIFI_OFF_ICON_WIDTH, WIFI_OFF_ICON_HEIGHT, wifi_off_icon);
    }

    // Small dot, top-right corner of the strip, while Exercise Reminder is armed
    if (reminderArmed) {
      display.drawDisc(STRIP_WIDTH - 6, 3, 2);
    }

    // Battery icon, centered - a few px further down from the WiFi icon
    // than before, so the two don't feel like they're touching.
    int battX = (STRIP_WIDTH - BATTERY_ICON_WIDTH) / 2;
    display.drawXBMP(battX, 20, BATTERY_ICON_WIDTH, BATTERY_ICON_HEIGHT, battery_icon);

    // Clock, bottom of the strip - biased 1px left so it doesn't crowd the
    // divider line (moving the line itself would risk cutting into the
    // face bitmap, since strip + face exactly fill 128px).
    //
    // Both formats now share the same stacked layout: hour (with trailing
    // ':') on top, minute below. 12h mode additionally gets small AM/PM
    // text next to the minutes - 24h has no AM/PM to show, so
    // that part is skipped there.
    uint8_t displayHour = timeHour;
    if (!use24Hour) {
      displayHour = timeHour % 12;
      if (displayHour == 0) displayHour = 12;
    }
    char hourBuf[4]; // "23:\0" / "12:\0" - always 2 digits + colon
    snprintf(hourBuf, sizeof(hourBuf), "%02u:", displayHour);
    char minBuf[3];
    snprintf(minBuf, sizeof(minBuf), "%02u", timeMinute);

    display.setFont(u8g2_font_8x13B_tr); // one size up from 7x13B
    int hourW = display.getStrWidth(hourBuf);
    int hourX = (STRIP_WIDTH - hourW) / 2 - 1;
    if (hourX < 0) hourX = 0;
    display.drawStr(hourX, 48, hourBuf); // nudged down from 44

    display.setFont(u8g2_font_7x13B_tr); // minute line stays at the previous size
    int minY = 63; // baseline for the minute digits

    if (use24Hour) {
      int minW = display.getStrWidth(minBuf);
      int minX = (STRIP_WIDTH - minW) / 2 - 1;
      if (minX < 0) minX = 0;
      display.drawStr(minX, minY, minBuf);
    } else {
      const char* ampm = (timeHour < 12) ? "AM" : "PM";

      // Minutes + small plain "AM"/"PM" text, centred together as one unit.
      // (Replaces the circled A/P badge: a lone letter was ambiguous and the
      // filled disc was the heaviest shape in the strip.) The text shares the
      // minute digits' baseline so the pair reads as one line.
      const int ampmGap = 3;
      int minW = display.getStrWidth(minBuf);       // minute font is still active
      display.setFont(u8g2_font_5x7_tr);
      int ampmW = display.getStrWidth(ampm);
      int startX = (STRIP_WIDTH - (minW + ampmGap + ampmW)) / 2 - 1;
      if (startX < 0) startX = 0;

      display.drawStr(startX + minW + ampmGap, minY, ampm);
      display.setFont(u8g2_font_7x13B_tr);
      display.drawStr(startX, minY, minBuf);
    }

    // --- Face: border + eyes are now procedural (FaceModule) ---
    face.drawBorder();
    face.drawEyes(EyeStyle::OPEN);   // ignored while a blink is in progress
    face.updateBlink();              // owns eye height during a blink; auto-blinks on a timer too

    // Brows/mouth/whiskers are still the original hand-drawn bitmap pieces,
    // drawn at the same coordinates they occupied in the old full-face
    // bitmap, just offset past the icon strip. These move into FaceModule
    // in a later pass (BrowStyle / MouthStyle), same treatment as the eyes.
    int fx = STRIP_WIDTH;
    display.drawXBMP(fx + 24, 16, BROW_LEFT_WIDTH,  BROW_LEFT_HEIGHT,  brow_left_bmp);
    display.drawXBMP(fx + 59, 16, BROW_RIGHT_WIDTH, BROW_RIGHT_HEIGHT, brow_right_bmp);
    display.drawXBMP(fx + 42, 46, MOUTH_WIDTH,      MOUTH_HEIGHT,      mouth_bmp);
    display.drawXBMP(fx + 14, 41, WHISKER_LEFT_WIDTH,  WHISKER_LEFT_HEIGHT,  whisker_left_bmp);
    display.drawXBMP(fx + 76, 41, WHISKER_RIGHT_WIDTH, WHISKER_RIGHT_HEIGHT, whisker_right_bmp);
  }

  display.sendBuffer();
  return featureSelected;
}

int DisplayModule::selectedGroup() const { return menu.selectedGroup(); }
int DisplayModule::selectedItem() const { return menu.selectedItem(); }

void DisplayModule::triggerBlink() {
  face.triggerBlink();
}

void DisplayModule::setWifiConnected(bool connected) {
  wifiConnected = connected;
}

void DisplayModule::setBatteryPercent(uint8_t percent) {
  batteryPercent = percent;
}

void DisplayModule::setTime(uint8_t hour, uint8_t minute) {
  timeHour = hour;
  timeMinute = minute;
}

void DisplayModule::setReminderArmed(bool armed) {
  reminderArmed = armed;
}

bool DisplayModule::isMenuActive() {
  return menu.isActive();
}

void DisplayModule::toggleClockFormat() {
  use24Hour = !use24Hour;
  prefs.begin(PREFS_NAMESPACE, false); // read-write
  prefs.putBool(PREFS_KEY_24H, use24Hour);
  prefs.end();
  menu.setClockFormatLabel(use24Hour);
}

void DisplayModule::previewBrightnessPercent(uint8_t percent) {
  if (percent < 1) percent = 1;   // never let it go fully dark - looks like a dead screen
  if (percent > 100) percent = 100;
  brightnessPercent = percent;
  uint8_t contrast = (uint16_t)brightnessPercent * 255 / 100;
  display.setContrast(contrast);
}

void DisplayModule::commitBrightnessPercent(uint8_t percent) {
  previewBrightnessPercent(percent); // clamps + applies, and updates brightnessPercent
  prefs.begin(PREFS_NAMESPACE, false); // read-write
  prefs.putUChar(PREFS_KEY_BRIGHTNESS, brightnessPercent);
  prefs.end();
}