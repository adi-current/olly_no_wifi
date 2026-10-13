#include "FunModule.h"
#include "quotes.h"
#include "jokes.h"
#include <esp_system.h> // esp_random() - hardware RNG, no seeding needed

void FunModule::begin(U8G2* d) {
  display = d;
}

void FunModule::pickNewQuote() {
  if (QUOTE_COUNT <= 1) {
    currentQuoteIndex = 0;
    return;
  }
  int newIndex;
  do {
    newIndex = esp_random() % QUOTE_COUNT;
  } while (newIndex == currentQuoteIndex); // avoid showing the same one twice in a row
  currentQuoteIndex = newIndex;
}

void FunModule::pickNewJoke() {
  if (JOKE_COUNT <= 1) {
    currentJokeIndex = 0;
  } else {
    int newIndex;
    do {
      newIndex = esp_random() % JOKE_COUNT;
    } while (newIndex == currentJokeIndex);
    currentJokeIndex = newIndex;
  }
  punchlineRevealed = false;
}

void FunModule::start(FunType type) {
  currentType = type;
  if (type == FunType::MOTIVATION) {
    pickNewQuote();
  } else {
    pickNewJoke();
  }
}

bool FunModule::update(int rotationDelta, bool actionPressed, bool backPressed) {
  if (backPressed) return true; // universal back

  if (currentType == FunType::MOTIVATION) {
    if (actionPressed) {
      pickNewQuote();
    }
  } else {
    if (actionPressed) {
      if (!punchlineRevealed) {
        punchlineRevealed = true;
      } else {
        pickNewJoke(); // tapping again after the punchline moves on to a fresh joke
      }
    }
  }
  return false;
}

void FunModule::render() {
  if (!display) return;
  display->setFont(u8g2_font_6x10_tr);

  if (currentType == FunType::MOTIVATION) {
    renderWrapped(QUOTES[currentQuoteIndex], 12, 11, 124);
    display->drawStr(2, 62, "(tap for another)");
  } else {
    if (!punchlineRevealed) {
      renderWrapped(JOKES[currentJokeIndex].setup, 12, 11, 124);
      display->drawStr(2, 62, "(tap for punchline)");
    } else {
      renderWrapped(JOKES[currentJokeIndex].punchline, 12, 11, 124);
      display->drawStr(2, 62, "(tap for next joke)");
    }
  }
}

void FunModule::renderWrapped(const char* text, int startY, int lineHeight, int maxWidth) {
  char buffer[128];
  strncpy(buffer, text, sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';

  int y = startY;
  String line = "";
  char* word = strtok(buffer, " ");

  while (word != nullptr) {
    String candidate = (line.length() == 0) ? String(word) : (line + " " + word);
    if (display->getStrWidth(candidate.c_str()) > maxWidth && line.length() > 0) {
      display->drawStr(2, y, line.c_str());
      y += lineHeight;
      line = word;
    } else {
      line = candidate;
    }
    word = strtok(nullptr, " ");
  }
  if (line.length() > 0) {
    display->drawStr(2, y, line.c_str());
  }
}
