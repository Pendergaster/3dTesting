#ifndef GAME_HEADER
#define GAME_HEADER

#define EXPORT/* extern "C" */__declspec(dllexport)

EXPORT void init_game();
EXPORT void update_game();
EXPORT void dispose_game();

#endif // !GAME_HEADER

