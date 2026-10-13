#pragma once
#include <Arduino.h>
#include <U8g2lib.h>

// Owns the 3-level Home -> Group -> Feature menu navigation and renders it.
//
// Deliberately takes plain input signals (rotation/confirm/back) rather than
// reading EncoderModule or TouchModule directly - keeps MenuModule ignorant
// of *which* hardware drives it, matching how FaceModule doesn't know about
// the status strip. Whatever calls update() is responsible for translating
// real input into these three signals.
//
// Project-wide input rule this implements:
//   confirmPressed -> go one level deeper / commit
//   backPressed    -> go one level back
// (Right now, until TouchModule exists, backPressed is a stand-in fed from
// the encoder's long-press. Swap that source later - MenuModule doesn't
// need to change at all.)
class MenuModule {
  public:
    void begin(U8G2* display);

    // Call every loop(). Returns true exactly once, the frame a feature is
    // confirmed at the deepest (Feature) level - read selectedGroup()/
    // selectedItem() that same frame to find out which one.
    bool update(int rotationDelta, bool confirmPressed, bool backPressed);

    // Valid only on the frame update() returned true.
    int selectedGroup() const; // 0 = Timers, 1 = Games, 2 = Settings
    int selectedItem() const;  // index within that group

    // Draws whatever the current menu state is. Only call while isActive()
    // - when false, the caller should be drawing Home (face + strip)
    // instead, not this.
    void render();

    // False = Home (menu not showing, caller owns the screen).
    // True = Group list or Feature list is showing.
    bool isActive() const;

    // Force the menu open from Home. update() already does this internally
    // when confirmPressed fires at Home, so this is only for cases where
    // something other than the normal input path needs to open it.
    void enterMenu();

    // Lets DisplayModule keep the "Clock Format" item's own label in sync
    // with the actual setting (e.g. "Clock Format: 24h") - MenuModule owns
    // menu_data.h's mutable label buffer (it's the only .cpp that includes
    // it), so this is the one narrow door in for updating it from outside.
    void setClockFormatLabel(bool use24h);

  private:
    enum class State { HOME, GROUP, FEATURE };
    State state = State::HOME;

    int groupIndex = 0;
    int itemIndex = 0;

    U8G2* display = nullptr;

    void clampSelection(int& index, int count);
    void renderList(const char* title, const char* const* items, int count, int selected);
};
