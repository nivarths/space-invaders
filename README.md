# space-invaders
[IN PROGRESS] A retro-style Space Invaders clone built from scratch using C++ and OOD principles, OpenGL (GLFW + GLEW)

Currently implemented:
- Rendering: OpenGL + pixel buffer, sprites, shaders
- Player: Movement, shooting, sprite drawing, lives system (hit detection, game over on death)
- Aliens: Different types, animation, movement, death, respawn
- Collisions: Bullet-alien, alien bullet-player, score increases
- Alien bullets: Fired randomly from living aliens, with cooldown timer
- UI: Score, lives, "GAME OVER", restart prompt
- Input: Movement, fire (space), restart (R), quit (Esc)
- Reset logic: Clean restart with reset_game() function
- Levels with increasing difficulty
- Main menu screen
- Pause feature
- OOD Principles through classes for major features

Need to implement:
- High score tracker
- Power-ups
