#include "FaceModule.h"

void FaceModule::begin(U8G2* displayPtr, int ox, int oy) {
  display = displayPtr;
  originX = ox;
  originY = oy;
  nextAutoBlinkAt = millis() + randomBlinkGap();
}

void FaceModule::drawBorder() {
  if (!display) return;
  int x = originX + FACE_X;
  int y = originY + FACE_Y;

  display->setDrawColor(1);

  // 4 corners - original hand-drawn stepped/pixel-art tiles, not smooth arcs
  display->drawXBMP(x, y, FACE_CORNER_TL_WIDTH, FACE_CORNER_TL_HEIGHT, face_corner_tl);
  display->drawXBMP(x + FACE_W - CORNER_W, y, FACE_CORNER_TR_WIDTH, FACE_CORNER_TR_HEIGHT, face_corner_tr);
  display->drawXBMP(x, y + FACE_H - CORNER_H, FACE_CORNER_BL_WIDTH, FACE_CORNER_BL_HEIGHT, face_corner_bl);
  display->drawXBMP(x + FACE_W - CORNER_W, y + FACE_H - CORNER_H, FACE_CORNER_BR_WIDTH, FACE_CORNER_BR_HEIGHT, face_corner_br);

  // Straight edges (2px thick) filling the gap between the 4 corners
  int midX0 = x + CORNER_W;
  int midX1 = x + FACE_W - CORNER_W; // exclusive
  int midY0 = y + CORNER_H;
  int midY1 = y + FACE_H - CORNER_H; // exclusive

  display->drawBox(midX0, y, midX1 - midX0, 2);                      // top
  display->drawBox(midX0, y + FACE_H - 2, midX1 - midX0, 2);         // bottom
  display->drawBox(x, midY0, 2, midY1 - midY0);                      // left
  display->drawBox(x + FACE_W - 2, midY0, 2, midY1 - midY0);         // right
}

void FaceModule::drawEyesRaw(int openness) {
  if (!display) return;

  int cys[2]   = { EYE_CY, EYE_CY };
  int cxs[2]   = { LEFT_EYE_CX, RIGHT_EYE_CX };

  for (int i = 0; i < 2; i++) {
    int cx = originX + cxs[i];
    int cy = originY + cys[i];
    int w = EYE_W;
    int h = openness;

    display->setDrawColor(1);

    if (h <= 3) {
      // Fully (or nearly) closed - just draw the eyelid line.
      display->drawBox(cx - w / 2, cy - 1, w, 2);
      continue;
    }

    int x0 = cx - w / 2;
    int y0 = cy - h / 2;
    int r  = min(4, h / 5);
    display->drawRBox(x0, y0, w, h, r);

    // Pupil - a smaller rounded box punched out of the sclera, offset
    // up-left so a white "crescent" stays visible on the bottom/right,
    // same proportions as the original hand-drawn eye.
    int pw = w * 9 / 16;
    int ph = h * 13 / 22;
    int px = x0 + w * 5 / 16;
    int py = y0 + h * 4 / 22;
    int pr = min(3, ph / 5);
    display->setDrawColor(0);
    display->drawRBox(px, py, pw, ph, pr);

    // Highlight dot
    display->setDrawColor(1);
    display->drawBox(px + pw / 4, py + ph / 8, 2, 2);
  }
}

void FaceModule::drawEyes(EyeStyle style) {
  if (isBlinking()) return; // let the blink animation own eye height for now

  int openness = EYE_H_OPEN;
  switch (style) {
    case EyeStyle::OPEN:        openness = EYE_H_OPEN;     break;
    case EyeStyle::HALF_CLOSED: openness = EYE_H_OPEN / 2; break;
    case EyeStyle::CLOSED:      openness = 0;              break;
  }
  drawEyesRaw(openness);
}

unsigned long FaceModule::randomBlinkGap() const {
  return BLINK_MIN_GAP_MS + random(BLINK_MAX_GAP_MS - BLINK_MIN_GAP_MS);
}

void FaceModule::triggerBlink() {
  if (blinkPhase == BlinkPhase::IDLE) {
    blinkPhase = BlinkPhase::CLOSING;
    blinkPhaseStart = millis();
  }
}

bool FaceModule::isBlinking() const {
  return blinkPhase != BlinkPhase::IDLE;
}

void FaceModule::updateBlink() {
  unsigned long now = millis();

  if (blinkPhase == BlinkPhase::IDLE && now >= nextAutoBlinkAt) {
    triggerBlink();
  }

  switch (blinkPhase) {
    case BlinkPhase::IDLE:
      return; // drawEyes() handles normal rendering

    case BlinkPhase::CLOSING: {
      unsigned long elapsed = now - blinkPhaseStart;
      if (elapsed >= BLINK_CLOSE_MS) {
        blinkPhase = BlinkPhase::OPENING;
        blinkPhaseStart = now;
        drawEyesRaw(0);
      } else {
        int openness = EYE_H_OPEN - (EYE_H_OPEN * (int)elapsed / (int)BLINK_CLOSE_MS);
        drawEyesRaw(openness);
      }
      break;
    }

    case BlinkPhase::OPENING: {
      unsigned long elapsed = now - blinkPhaseStart;
      if (elapsed >= BLINK_OPEN_MS) {
        blinkPhase = BlinkPhase::IDLE;
        nextAutoBlinkAt = now + randomBlinkGap();
        drawEyesRaw(EYE_H_OPEN);
      } else {
        int openness = EYE_H_OPEN * (int)elapsed / (int)BLINK_OPEN_MS;
        drawEyesRaw(openness);
      }
      break;
    }
  }
}
