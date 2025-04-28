# space-invaders
[IN PROGRESS] A retro-style Space Invaders clone built from scratch using C++ and OOD principles and OpenGL (GLFW + GLEW)

![Screenshot 2025-04-22 194557](https://github.com/user-attachments/assets/40358b2a-d8c7-4bfb-a26f-c5fc55cf9b6c)

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
![Screenshot 2025-04-22 194705](https://github.com/user-attachments/assets/d2a6c9a5-17c9-4010-877c-36a088988269)

