#include "Game.h"
#include "Player.h"
#include "Alien.h"
#include "Bullet.h"
#include "SpriteAnimation.h"
#include <iostream>
#include <algorithm>
#include <string>
#include <chrono>
#include <memory>
#include <random>
#include <cstring>
#include <GLFW/glfw3.h>
#include <unordered_map>

// Helper function to replace std::make_unique
template<typename T, typename... Args>
std::unique_ptr<T> make_unique_helper(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

// Constructor
Game::Game(size_t w, size_t h, const Sprite* pSprite) 
    : width(w)
    , height(h)
    , num_aliens(0)
    , num_bullets(0)
    , num_alien_bullets(0)
    , level(1)
    , buffer(w, h)
    , is_running(false)
    , player_sprite(pSprite)
    , bulletSprite(nullptr)
    , state(GameState::MENU)
    , score(0)
    , lastFrameTime(0.0)
    , frameTime(0.0)
    , window(nullptr)
    , _baseAlienMoveInterval(0.5f)
    , baseAlienFireMin(2.0f)
    , baseAlienFireMax(4.0f)
    , alienBaseMoveInterval(_baseAlienMoveInterval)
    , _alienRng{ std::random_device{}() }
    , _alienFireDistance{ baseAlienFireMin, baseAlienFireMax }         // random interval between 0.5s and 1.5s
    , _alienFireTimer{ 0.0f }
    , _nextAlienFireInterval { _alienFireDistance(_alienRng)}
{
    alienSprites.resize(4, nullptr);
    alienAnimations.resize(4, nullptr);
}

void Game::setWindow(GLFWwindow* win) {
    window = win;
}

void Game::start() {
    is_running = true;
    level = 1;
    _inLevelTransition = true;
    state = GameState::LEVEL_TRANSITION;
    _levelTransitionStart = glfwGetTime();
    lastFrameTime = glfwGetTime();
    initGame();
}

void Game::stop() {
    is_running = false;
    state = GameState::GAME_OVER;
}

void Game::pause() {
    if (is_running) {
        state = GameState::PAUSED;
    }
}

void Game::resume() {
    if (state == GameState::PAUSED) {
        state = GameState::PLAYING;
    }
}

void Game::reset() {
    score = 0;
    level = 1;
    _inLevelTransition = true;
    _levelTransitionStart = glfwGetTime();
    bullets.clear();
    alien_bullets.clear();
    aliens.clear();
    initGame();
}

void Game::initGame() {


    is_running = true;
    state = GameState::PLAYING;
    score = 0;

    if (!player_sprite) {
        std::cerr << "[FATAL] player_sprite is NULL!" << std::endl;
        exit(1);
    }
    // Initialize player at center bottom
    size_t player_y = 30; // closer to bottom
    player = make_unique_helper<Player>(width / 2 - player_sprite->getSpriteWidth() / 2, player_y, player_sprite);
    player->setShootCooldown(5);    // ~9 frames between shots (~0.25s at 60FPS)
    player->resetShootCooldown();

    // Clear existing entities
    bullets.clear();
    alien_bullets.clear();
    aliens.clear();

    // Spawn initial aliens
    spawnAliens();

    _alienFireTimer = 0.0f;
    _nextAlienFireInterval = baseAlienFireMax * 2.0f;
    alienMoveTimer = 0.0f;
    alienMovingRight = true;
    alienNeedToDrop = false;
    
}

void Game::spawnAliens() {
    const int ROWS = 5;
    const int COLS = 11;
    std::cout << "[spawnAliens] spawning " << ROWS << " rows of aliens\n";
    const size_t START_X = 24;
    const size_t START_Y = 200;
    const size_t SPACING_X = 16;
    const size_t SPACING_Y = 16;

    for (int row = 0; row < ROWS; ++row) {
        AlienType type;
        if(row < 1)
        {
            type = AlienType::TYPE_A;
        }
        else if(row < 3)
        {
            type = AlienType::TYPE_B;
        }
        else
        {
            type = AlienType::TYPE_C;
        }

        for (int col = 0; col < COLS; ++col) {
            size_t x = START_X + col * SPACING_X;
            size_t y = START_Y - row * SPACING_Y;

            if (alienSprites[size_t(type)] && alienAnimations[size_t(type)]) {
                createAlien(x, y, type);
            }
        }
    }
    num_aliens = aliens.size();
}

void Game::createAlien(int x, int y, AlienType type) {
    size_t index = static_cast<size_t>(type);
    if (index < alienSprites.size()) {
        auto alien = make_unique_helper<Alien>(
            static_cast<size_t>(x), 
            static_cast<size_t>(y), 
            alienSprites[index],
            type
        );
        if (alien && index < alienAnimations.size()) {
            alien->setDeathAnimation(const_cast<SpriteAnimation*>(alienAnimations[index]));
        }
        aliens.push_back(std::move(alien));
    }
}

void Game::updateGame() 
{
    if (state != GameState::PLAYING) return;

    // Update player
    if (player) {
        player->update(frameTime);
    }

    updateAliens();
    updateBullets();
    checkCollisions();
    cleanupAliens();
}

void Game::updateAliens() {
    if (aliens.empty()) return;

    float move_interval = alienBaseMoveInterval;

    alienMoveTimer += frameTime;
    if (alienMoveTimer >= move_interval) {
        alienMoveTimer = 0.0f;

        // Check boundaries
        size_t leftmost = width;
        size_t rightmost = 0;
        size_t alien_width = aliens[0]->getSprite()->getSpriteWidth();  // assume at least one alien exists
        for (const auto& alien : aliens) {
            if (!alien) continue;
            size_t x = alien->getX();
            leftmost = std::min(leftmost, x);
            rightmost = std::max(rightmost, x);
        }

        // Change direction if needed
        if (alienMovingRight && rightmost + alien_width >= width - 10) {
            alienMovingRight = false;
            alienNeedToDrop = true;
        } else if (!alienMovingRight && leftmost <= 10) {
            alienMovingRight = true;
            alienNeedToDrop = true;
        }

        // Move horizontally
        for (const auto& alien : aliens) {
            if (!alien) continue;
            if(alienNeedToDrop)
            {
                alien->setPosition(alien->getX(), alien->getY() - 8);
            }
            else
            {
                alien->setPosition(alien->getX() + (alienMovingRight ? 5 : -5), alien->getY());
            }
            alien->update(frameTime);
        }

        if(alienNeedToDrop)
        {
            alienNeedToDrop = false;
        }

        // Update animations
        for (const auto& alien : aliens) {
            if (alien) {
                alien->update(frameTime);
            }
        }
    }
}

void Game::updateBullets() {
    // Update player bullets
    for (auto& bullet : bullets) {
        if (bullet) {
            bullet->update(frameTime);
        }
    }

    // Update alien bullets
    for (auto& bullet : alien_bullets) {
        if (bullet) {
            bullet->update(frameTime);
        }
    }

    // Alien shooting logic
    _alienFireTimer += frameTime;
    if (_alienFireTimer >= _nextAlienFireInterval) 
    {
        _alienFireTimer = 0.0f;
        _nextAlienFireInterval = _alienFireDistance(_alienRng);

        // pick a random live alien
        std::unordered_map<int,size_t> bottom;
        for(size_t i = 0; i < aliens.size(); ++i) 
        {
            if(!aliens[i] || aliens[i]->isAlienDead()) continue;
            int x = aliens[i]->getX();
            if(!bottom.count(x) || aliens[i]->getY() < aliens[bottom[x]]->getY())
            {
                bottom[x] = i;
            }
            
        }
        
        if(!bottom.empty()) 
        {
            std::vector<size_t> shooters;
            shooters.reserve(bottom.size());
            for(auto &p : bottom) shooters.push_back(p.second);
            size_t idx = shooters[
            std::uniform_int_distribution<size_t>(0, shooters.size()-1)(_alienRng)];
            spawnAlienBullet(*aliens[idx]);
        }
    }
    cleanupBullets();
}

void Game::checkCollisions() {
    if (!player) return;

    // Check player bullet collisions with aliens
    for (auto bIt = bullets.begin(); bIt != bullets.end(); ) {
        if (!*bIt) {
            bIt = bullets.erase(bIt);
            continue;
        }
        bool hit = false;
        for (auto& alien : aliens) {
            if (alien && !alien->isAlienDead() &&
                checkCollision(**bIt, *alien))
            {
                alien->kill();
                bIt = bullets.erase(bIt);
                score += SCORE_PER_ALIEN;
                hit = true;
                break;
            }
        }
        if (!hit) {
            ++bIt;
        }
    }

    // Check alien bullet collisions with player
    for (auto& bullet : alien_bullets) {
        if (bullet && checkCollision(*bullet, *player)) {
            bullet.reset();
            player->loseLife();
            if(player->getLives() == 0)
            {
                stop();
            }
            break;
        }
    }
}

void Game::renderGame() {
    // Clear buffer with classic Space Invaders green
    buffer.clear(0xFF008000);
    
    if (player) {
        player->draw(buffer, 0xFFFFFFFF);
    }
    
    // std::cout << "Rendering player at: " << player->getX() << ", " << player->getY() << std::endl;

    for (const auto& alien : aliens) {
        if (alien) {
            alien->drawAlien(buffer, 0xFFFFFFFF);
        }
    }
    
    for (const auto& bullet : bullets) {
        if (bullet) {
            bullet->draw(buffer, 0xFFFFFFFF);
        }
    }
    
    for (const auto& bullet : alien_bullets) {
        if (bullet) {
            bullet->draw(buffer, 0xFFFFFFFF);
        }
    }

    drawUI();

}

void Game::handleInput() {
    
    if (state == GameState::MENU && window) 
    {
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) 
        {
          start();      // this will set up level=1, show LEVEL_TRANSITION, etc.
        }
        // swallow all other keys while in MENU
        return;
    }
    
    // 1) catch our “press key to start next level” here
    if (state == GameState::LEVEL_TRANSITION && window) {
        // for example, use SPACE to continue:
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
          state = GameState::PLAYING;
          _inLevelTransition = false;
  
          // re‑spawn the next wave
          aliens.clear();
          spawnAliens();
  
          // reset your movement & fire timers
          alienMoveTimer = 0.0f;
          _alienFireTimer = 0.0f;
          _nextAlienFireInterval = _alienFireDistance(_alienRng);
        }
        // swallow all other input until you transition
        return;
    }
  
      // 2) then your GAME_OVER “PRESS R” logic (if you still have it):
    if (state == GameState::GAME_OVER_PROMPT && window) 
    {
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) 
        {
          reset();
        }
        return;
    }
    
    if (!window || !player) return;
    
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        player->moveX(-1);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        player->moveX(1);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        spawnPlayerBullet();
    }
    
    bool escNow = glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    if (escNow && !prevEscPressed) {
        if (state == GameState::PLAYING) {
            pause();
        }
        else if (state == GameState::PAUSED) {
            resume();
        }
    }
    prevEscPressed = escNow;
}

void Game::spawnBullet(size_t x, size_t y, BulletType type) {
    if (bullets.size() < MAX_BULLETS && bulletSprite) {
        auto bullet = make_unique_helper<Bullet>(x, y, bulletSprite, type);
        bullets.push_back(std::move(bullet));
    }
}

void Game::spawnPlayerBullet() {
    if (!player || !bulletSprite) 
    {
        return;
    }
    if (bullets.size() < MAX_BULLETS)
    {
        spawnBullet(player->getX() + player->getSprite()->getSpriteWidth() / 2, player->getY() + player->getSprite()->getSpriteHeight(), BulletType::PLAYER);
    }
        
}

void Game::spawnAlienBullet(const Alien& alien) {
    if (!bulletSprite || alien_bullets.size() >= MAX_ALIEN_BULLETS) return;
    spawnAlienBullet(alien.getX() + alien.getSprite()->getSpriteWidth() / 2, alien.getY() - alien.getSprite()->getSpriteHeight());
}

void Game::spawnAlienBullet(size_t x, size_t y) {
    if (alien_bullets.size() < MAX_ALIEN_BULLETS && bulletSprite) {
        auto bullet = make_unique_helper<Bullet>(x, y, bulletSprite, BulletType::ALIEN);
        alien_bullets.push_back(std::move(bullet));
    }
}

void Game::cleanupBullets() {
    // Remove bullets that are out of bounds using explicit iterator types
    using BulletIterator = std::vector<std::unique_ptr<Bullet>>::iterator;
    
    for (BulletIterator it = bullets.begin(); it != bullets.end();) {
        if (!*it || (*it)->getY() >= height) {
            it = bullets.erase(it);
        } else {
            ++it;
        }
    }
    
    for (BulletIterator it = alien_bullets.begin(); it != alien_bullets.end();) {
        if (!*it || (*it)->getY() <= 0) {
            it = alien_bullets.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::cleanupAliens() 
{
    aliens.erase(
        std::remove_if(aliens.begin(), aliens.end(),
          [](auto& a){ return a->isAlienDead(); }
        ),
        aliens.end()
      );
    
    num_aliens = aliens.size();
    
    // Level complete check
    if (num_aliens == 0 && state == GameState::PLAYING) 
    {
        enterLevelTransition();
    }
}
void Game::drawUI() 
{
    // red UI  
    constexpr uint32_t UIColour = 0xFF0000FF;
    constexpr size_t charWidth = 8;
    constexpr size_t charHeight = 8;

    auto fromTop = [&](size_t yFromTop)
    {
        return height - yFromTop - charHeight;
    };

    // Draw score
    buffer.drawText("SCORE", 10, fromTop(10), UIColour);
    buffer.drawText(std::to_string(score).c_str(), 10, fromTop(10 + charHeight + 2), UIColour);
    
    // Draw level
    const char* levelLabel = "LEVEL";
    std::string lvl = std::to_string(level);
    size_t levelLabelWidth = std::strlen(levelLabel) * charWidth;
    size_t lvlWidth = lvl.size() * charWidth;
    const size_t gap = 4;
    const size_t margin = 10;
    size_t totalW = levelLabelWidth + gap + lvlWidth;
    size_t levelX = (width - totalW - margin);
    size_t levelY = fromTop(10);

    buffer.drawText(levelLabel, levelX, levelY, UIColour);
    buffer.drawText(lvl.c_str(), levelX + levelLabelWidth + gap, levelY, UIColour);

    // LIVES label
    const char* livesLabel = "LIVES";
    size_t livesLabelWidth = std::strlen(livesLabel) * charWidth;
    size_t livesLabelX = (width - livesLabelWidth) / 2;
    buffer.drawText(livesLabel, livesLabelX, fromTop(10), UIColour);

    // hearts under it
    size_t playerLives = player->getLives();
    size_t spacing = 2;
    size_t totalWidth = playerLives * heartSprite->getSpriteWidth() + (playerLives > 0 ? (playerLives - 1) * spacing : 0);
    size_t hx0 = (width - totalWidth)/2;
    size_t hy = fromTop(10 + charHeight + 2);

    for (size_t i = 0; i < playerLives; ++i) 
    {
        buffer.drawSprite(*heartSprite, hx0 + i * (heartSprite->getSpriteWidth() + spacing), hy, UIColour);
    }
    
    // if we're currently invincible (just got hit), flash a gray ghost heart
    if (player->isInvincible()) 
    {
        size_t i = playerLives;
        if (i < 5 && (player->getInvincibilityTimer() / 3) % 2 == 0) 
        {
            buffer.drawSprite(*heartSprite, hx0 + i * (heartSprite->getSpriteWidth()+spacing), hy, 0xFF808080);
        }
    }

    // draw CREDIT 00 and bottom line
    constexpr size_t bottomMargin = 4;
    const char* creditLabel = "CREDIT 00";
    size_t creditLabelWidth = std::strlen(creditLabel) * charWidth;
    buffer.drawText(creditLabel, width - creditLabelWidth - bottomMargin, bottomMargin, UIColour);

    size_t groundY = bottomMargin + charHeight + 2;
    for(size_t x = 0; x < width; ++x)
    {
        buffer.drawPixel(x, groundY, UIColour);
    }
    
    // Draw game state messages
    if (state == GameState::PAUSED)
    {
        const char* pauseMessage = "PAUSED";
        buffer.drawText(pauseMessage, (width - std::strlen(pauseMessage) * charWidth) / 2, fromTop((height / 2) - charHeight/2) / 2, UIColour);
    } 
    else if (state == GameState::GAME_OVER) 
    {
        const char* gameOverMessage = "GAME OVER";
        buffer.drawText(gameOverMessage, (width - std::strlen(gameOverMessage) * charWidth) / 2, fromTop((height / 2) - charHeight / 2), UIColour);
    }
}

void Game::run() 
{
    double currentTime = glfwGetTime();
    frameTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;

    handleInput();

    switch(state) 
    {
        case GameState::MENU:
            renderMenu();
            break;
        case GameState::LEVEL_TRANSITION:
          if(currentTime - _levelTransitionStart < levelTransitionDuration) 
          {
            renderLevelSplash();   // draw only “LEVEL N” on blank
          } 
          else 
          {
            state = GameState::PLAYING;
            _inLevelTransition = false;
            aliens.clear();
            spawnAliens();
            alienMoveTimer = 0;
            _alienFireTimer = 0;
            _nextAlienFireInterval = _alienFireDistance(_alienRng);
          }
          break;
        case GameState::PLAYING:
          update(); 
          render();
          if(player->getLives() == 0)
          {
            enterGameOver();
          }
          if(aliens.empty() && !_inLevelTransition)
          {
            enterLevelTransition();
          }
          break;
        case GameState::PAUSED:
          renderGame();   // draws everything (player, aliens, bullets)
          drawUI();       // overlays the “PAUSED” text
          break;
        case GameState::GAME_OVER_FREEZE:
          if (currentTime - _gameOverStart < 5.0) 
          {
            renderGameOverText();  // draw only “GAME OVER” on a blank background
          } 
          else 
          {
            state = GameState::GAME_OVER_PROMPT;
            _awaitingRestart = true;
          }
          break;
  
        case GameState::GAME_OVER_PROMPT:
          renderGameOverText();
          renderRestartPrompt();   // “PRESS R TO RESTART”
          break;
        default: 
        break;
    }
}

GameState Game::getState() const {
    return state;
}

size_t Game::getScore() const {
    return score;
}

size_t Game::getLevel() const {
    return level;
}

bool Game::isGameRunning() const {
    return state == GameState::PLAYING;
}

size_t Game::getBufferWidth() const {
    return buffer.getWidth();
}

size_t Game::getBufferHeight() const {
    return buffer.getHeight();
}

uint32_t* Game::getBufferData() {
    return buffer.getBufferData();
}

void Game::setWidth(size_t w) {
    width = w;
}

void Game::setHeight(size_t h) {
    height = h;
}

size_t Game::getWidth() const {
    return width;
}

size_t Game::getHeight() const {
    return height;
}

void Game::update() {
    handleInput();
    updateGame();
}

void Game::render() {
    renderGame();
}

void Game::handleInput(int move_dir, bool fire_pressed) {
    if (state != GameState::PLAYING || !player) return;
    
    if (move_dir != 0) {
        player->moveX(move_dir);
    }
    
    if (fire_pressed) {
        spawnPlayerBullet();
    }
}

bool Game::checkCollision(const Entity& a, const Entity& b) const {
    return a.getBounds().intersects(b.getBounds());
}

void Game::addAlienSprite(AlienType type, const Sprite* sprite) {
    size_t index = static_cast<size_t>(type);
    if (index < alienSprites.size()) {
        alienSprites[index] = sprite;
    }
}

void Game::setBulletSprite(const Sprite* sprite) {
    bulletSprite = sprite;
}

void Game::setHeartSprite(const Sprite* sprite)
{
    heartSprite = sprite;
}

void Game::setAlienAnimation(AlienType type, const SpriteAnimation* animation) {
    size_t index = static_cast<size_t>(type);
    if (index < alienAnimations.size()) {
        alienAnimations[index] = animation;
    }
}

void Game::clear(uint32_t color)
{
    buffer.clear(color);
}

void Game::setFont(const std::unordered_map<char, Sprite>& glyphs)
{
    buffer.setFontGlyphs(glyphs);
}

void Game::enterGameOver()
{
    state = GameState::GAME_OVER_FREEZE;
    _gameOverStart = glfwGetTime();
    _awaitingRestart = false;
}  

/// called when all aliens are dead  
void Game::enterLevelTransition()
{
    // 1) bump the level
    ++level;

    // 2) enter splash state
    state = GameState::LEVEL_TRANSITION;
    _inLevelTransition = true;
    _levelTransitionStart = glfwGetTime();

    // 3) move faster: shrink the move‑interval
    //    → dividing by sqrt(level) gives diminishing returns
    alienBaseMoveInterval = _baseAlienMoveInterval / std::sqrt(static_cast<float>(level));

    // 4) Fire a bit faster, but clamp so they never fire more than once every 0.2s
    float scale = 1.0f + 0.05f * (level - 1);      // 10% faster per level
    float minI  = baseAlienFireMin  / scale;
    float maxI  = baseAlienFireMax  / scale;
    minI = std::max(1.0f, minI);
    maxI = std::max(minI + 0.1f, maxI);


    _alienFireDistance = std::uniform_real_distribution<float>(minI, maxI);

    // 5) schedule the next shot
    _nextAlienFireInterval = _alienFireDistance(_alienRng);
}

/// draw a blank green screen + “GAME OVER” centered  
void Game::renderGameOverText()
{
    constexpr uint32_t UIColour = 0xFF0000FF;
    buffer.clear(0xFF008000); // classic green
    const char* gameOverMessage = "GAME OVER";
    size_t gameOverMessageX = (width - std::strlen(gameOverMessage) * 8) / 2;
    size_t gameOverMessageY = (height - 8) / 2;
    buffer.drawText(gameOverMessage, gameOverMessageX, gameOverMessageY, UIColour);
}

/// draw “PRESS R TO RESTART” centered, on top of blank green  
void Game::renderRestartPrompt()
{
    constexpr uint32_t UIColour = 0xFF0000FF;
    const char* restartPrompt = "PRESS R TO RESTART";
    size_t restartPromptX = (width - std::strlen(restartPrompt) * 8) / 2;
    size_t restartPromptY = (height - 8) / 2 - 16;  // two lines up
    buffer.drawText(restartPrompt, restartPromptX, restartPromptY, UIColour);
}  

/// draw a blank green screen + “LEVEL N” centered  
void Game::renderLevelSplash()
{
    constexpr uint32_t UIColour = 0xFFFFFFFF;
    buffer.clear(0xFF008000);
    const char* label = "LEVEL";
    std::string num = std::to_string(level);
    size_t lw = std::strlen(label)*8;
    size_t nw = num.size()*8;
    size_t xLabel = (width - (lw + 8 + nw)) / 2;
    size_t y = (height - 8) / 2;
    buffer.drawText(label, xLabel, y, UIColour);
    buffer.drawText(num.c_str(), xLabel + lw + 8, y, UIColour);
}

void Game::renderMenu()
{
    buffer.clear(0xFF008000);

    // draw the title
    const char* title = "SPACE INVADERS";
    size_t titleX = (width - std::strlen(title) * 8) / 2;
    size_t titleY = height / 2 + 16;
    buffer.drawText(title, titleX, titleY, 0xFFFFFFFF);

    // draw the “press space” prompt
    const char* spaceToStartPrompt = "PRESS SPACE TO START";
    size_t spaceToStartPromptX = (width - std::strlen(spaceToStartPrompt) * 8) / 2;
    size_t spaceToStartPromptY = titleY - 16;
    buffer.drawText(spaceToStartPrompt, spaceToStartPromptX, spaceToStartPromptY, 0xFF0000FF);
}