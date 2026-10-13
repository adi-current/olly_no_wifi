#pragma once
#include <Arduino.h>
#include <U8g2lib.h>
#include "face_corners.h"

// Eye openness presets. OPEN/HALF_CLOSED/CLOSED are just named heights -
// nothing stops you from calling setEyeOpenness() with any raw value for
// smoother custom animation later (e.g. a slow droopy-tired eye).
enum class EyeStyle {
  OPEN,
  HALF_CLOSED,
  CLOSED
};

// FaceModule owns the procedural parts of Olly's face: the border and the
// eyes. It draws directly onto whatever U8G2 display object it's given, at
// an origin offset you choose (so it can sit next to the status icon strip
// without knowing anything about that strip).
//
// Brows/mouth/whiskers are NOT here yet - they're still the original
// hand-drawn bitmap pieces (see face_static_parts.h), drawn separately by
// DisplayModule for now. They'll move into this module the same way the
// eyes did, once we design BrowStyle/MouthStyle.
class FaceModule {
  public:
    // Call once from DisplayModule::begin(). originX/originY let the face
    // sit anywhere on the panel (e.g. offset past the status strip).
    void begin(U8G2* display, int originX, int originY);

    // Static drawing
    void drawBorder();
    void drawEyes(EyeStyle style);

    // Low-level eye control, used internally by drawEyes() and by the
    // blink animation - exposed publicly too, in case you want to drive
    // eye height from something else later (e.g. a "surprised" snap-open).
    void drawEyesRaw(int openness);

    // Non-blocking blink animation. Call this every frame from
    // DisplayModule::update(), right before or after drawEyes(). It handles
    // its own timing internally (via millis()) and will occasionally blink
    // on its own; drawEyes() is skipped automatically while a blink is
    // in progress so the two don't fight over eye height.
    void updateBlink();

    // Manually kick off a blink right now (e.g. in response to a touch).
    // No-op if a blink is already in progress.
    void triggerBlink();

    // True while updateBlink() is mid-animation - drawEyes() calls made
    // during this window are ignored so the blink isn't interrupted.
    bool isBlinking() const;

  private:
    U8G2* display = nullptr;
    int originX = 0;
    int originY = 0;

    // Face canvas geometry, measured from the original 96x64 hand-drawn
    // reference bitmap - keeps the procedural version at the same size/
    // position so it drops in as a straight swap.
    static const int FACE_X = 6, FACE_Y = 7, FACE_W = 84, FACE_H = 53;
    static const int CORNER_W = 19, CORNER_H = 15; // original hand-drawn stepped corner tile
    static const int EYE_W = 16, EYE_H_OPEN = 22;
    static const int LEFT_EYE_CX = 31, RIGHT_EYE_CX = 65, EYE_CY = 34;

    // Blink state machine (non-blocking - no delay() calls)
    enum class BlinkPhase { IDLE, CLOSING, OPENING };
    BlinkPhase blinkPhase = BlinkPhase::IDLE;
    unsigned long blinkPhaseStart = 0;
    unsigned long nextAutoBlinkAt = 0;

    static const unsigned long BLINK_CLOSE_MS = 70;
    static const unsigned long BLINK_OPEN_MS = 90;
    static const unsigned long BLINK_MIN_GAP_MS = 2500;
    static const unsigned long BLINK_MAX_GAP_MS = 6000;

    unsigned long randomBlinkGap() const;
};
