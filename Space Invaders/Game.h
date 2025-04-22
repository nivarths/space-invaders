#pragma once

#include <vector>
#include <memory>
#include <array>
#include <random>
#include <GLFW/glfw3.h>
#include "Buffer.h"
#include "AlienType.h"
#include "Player.h"
#include "Alien.h"
#include "Bullet.h"
#include "Sprite.h"
#include "SpriteAnimation.h"

// Forward declarations
class Sprite;
class Player;
class Alien;
class Bullet;
class Entity;
class SpriteAnimation;
enum class BulletType;

enum class GameState {
    MENU,
    PLAYING,
    GAME_OVER,
    PAUSED,
    LEVEL_TRANSITION,
    GAME_OVER_FREEZE,
    GAME_OVER_PROMPT
};

class Game {
private:
    // Constants
    static constexpr size_t MAX_BULLETS = 5;
    static constexpr size_t MAX_ALIEN_BULLETS = 1;
    static constexpr size_t SCORE_PER_ALIEN = 10;

    // Member variables
    size_t width;
    size_t height;
    size_t num_aliens;
    size_t num_bullets;
    size_t num_alien_bullets;
    size_t level;
    Buffer buffer;
    bool is_running;
    const Sprite* player_sprite;
    const Sprite* bulletSprite;
    const Sprite* heartSprite;
    bool alienMovingRight = true;
    float alienDropAmount = 10.0f;
    float alienNeedToDrop = false;
    float alienMoveTimer = 0.0f;
    float _baseAlienMoveInterval = 0.5f; // Move every second
    float alienBaseMoveInterval;  
    std::mt19937 _alienRng;
    std::uniform_real_distribution<float> _alienFireDistance;
    float _alienFireTimer;
    float _nextAlienFireInterval;
    double _gameOverStart = 0.0;
    bool _awaitingRestart = false;
    double _levelTransitionStart = 0.0;
    bool _inLevelTransition = false;
    float baseAlienFireMin = 2.0f;
    float baseAlienFireMax = 4.0f;
    float levelTransitionDuration = 3.0f;
    bool prevEscPressed = false;
    
    // Game state
    GameState state;
    size_t score;
    double lastFrameTime;
    double frameTime;
    
    // Game entities
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Alien>> aliens;
    std::vector<std::unique_ptr<Bullet>> bullets;
    std::vector<std::unique_ptr<Bullet>> alien_bullets;
    
    // Game resources
    std::vector<const Sprite*> alienSprites;
    std::vector<const SpriteAnimation*> alienAnimations;
    
    // Window handle
    GLFWwindow* window;

public:
    // Constructor and destructor
    Game(size_t width, size_t height, const Sprite* player_sprite);
    ~Game() = default;

    // Game control
    void start();
    void stop();
    void pause();
    void resume();
    void reset();
    bool isGameRunning() const;
    
    // Game loop
    void run();
    void update();
    void render();
    void renderMenu();
    void handleInput();
    void handleInput(int move_dir, bool fire_pressed);
    
    // Window management
    void setWindow(GLFWwindow* win);
    
    // Buffer access
    size_t getBufferWidth() const;
    size_t getBufferHeight() const;
    uint32_t* getBufferData();

    // Game state access
    GameState getState() const;
    size_t getScore() const;
    size_t getLevel() const;
    size_t getWidth() const;
    size_t getHeight() const;
    void setWidth(size_t w);
    void setHeight(size_t h);

    // Resource management
    void addAlienSprite(AlienType type, const Sprite* sprite);
    void setBulletSprite(const Sprite* sprite);
    void setHeartSprite(const Sprite* sprite);
    void setAlienAnimation(AlienType type, const SpriteAnimation* animation);
    void setFont(const std::unordered_map<char, Sprite>& glyphs);

    // Player access methods
    Player* getPlayer() { return player.get(); }

    void clear(uint32_t color);


private:
    // Game initialization
    void initGame();
    void initializePlayer();
    void spawnAliens();
    void createAlien(int x, int y, AlienType type);

    // Game update
    void updateGame();
    void updateAliens();
    void updateBullets();
    void checkCollisions();
    void cleanupBullets();
    void cleanupAliens();

    // Game rendering
    void renderGame();
    void drawUI();

    // Bullet management
    void spawnBullet(size_t x, size_t y, BulletType type);
    void createBullet(float x, float y, BulletType type);
    void spawnPlayerBullet();
    void spawnAlienBullet(const Alien& alien);
    void spawnAlienBullet(size_t x, size_t y);

    // Collision detection
    bool checkCollision(const Entity& a, const Entity& b) const;

    void enterGameOver();  

    /// called when all aliens are dead  
    void enterLevelTransition();  

    /// draw a blank green screen + “GAME OVER” centered  
    void renderGameOverText();  

    /// draw “PRESS R TO RESTART” centered, on top of blank green  
    void renderRestartPrompt();  

    /// draw a blank green screen + “LEVEL N” centered  
    void renderLevelSplash();
};