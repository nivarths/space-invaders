# space-invaders
[IN PROGRESS] A retro-style Space Invaders clone built from scratch using C++, OpenGL (GLFW + GLEW)

Currently implemented:
- Rendering: OpenGL + pixel buffer, sprites, shaders
- Player: Movement, shooting, sprite drawing
- Aliens: Types A/B/C, animation, movement, death, respawn
- Collisions: Bullet-alien, score increases
- UI: Score, "GAME OVER", restart prompt
- Input: Movement, fire (space), restart (R), quit (Esc)
- Reset logic: Clean restart with reset_game()

Need to implement:
- Alien bullets
- Player lives system (hit detection, game over on death)
- Alien firing logic (timed or random)
- Optional polish: sounds, levels, main menu, UFOs, barriers, high score
