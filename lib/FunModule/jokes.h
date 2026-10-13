#pragma once

// Local, offline jokes as setup/punchline pairs - FunModule shows the setup
// first, then reveals the punchline on the next tap, per the "comedic
// timing" design decision (better than dumping both at once).
struct Joke {
  const char* setup;
  const char* punchline;
};

static const Joke JOKES[] = {
  { "Why don't scientists trust atoms?", "Because they make up everything!" },
  { "Why did the scarecrow win an award?", "He was outstanding in his field!" },
  { "What do you call fake spaghetti?", "An impasta!" },
  { "Why did the bicycle fall over?", "It was two tired!" },
  { "What do you call a bear with no teeth?", "A gummy bear!" },
  { "Why can't your nose be 12 inches long?", "Because then it'd be a foot!" },
  { "What did one wall say to the other?", "I'll meet you at the corner!" },
  { "Why did the coffee file a police report?", "It got mugged!" },
  { "How do you organize a space party?", "You planet!" },
  { "Why don't skeletons fight each other?", "They don't have the guts!" },
};
static const int JOKE_COUNT = sizeof(JOKES) / sizeof(JOKES[0]);
