Olly graphics patch - changed files only (copy over your lib/ folders).

lib/TimerEngine/gauge_render.cpp/.h   gauge rewritten; new drawEditableTime()
lib/MenuModule/MenuModule.cpp         counter, scrollbar, taller highlight (descenders)
lib/DisplayModule/DisplayModule.cpp   12h AM/PM badge no longer clipped
lib/TimerModule/TimerModule.cpp       uses drawEditableTime
lib/SetTimeModule/SetTimeModule.cpp   uses drawEditableTime, hints shortened
lib/ExerciseReminderModule/...cpp     hints shortened (were wider than the screen)
lib/PomodoroModule/PomodoroModule.cpp PAUSED shown in the caption slot
