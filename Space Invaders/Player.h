#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include "Sprite.h"
#include "Buffer.h"
#include <cstdint>

enum class PlayerState {
    NORMAL,
    INVINCIBLE,
    POWERED_UP,
    DEAD
};

class Player : public Entity {
private:
    size_t lives;
    bool invincible;
    size_t invincibilityTimer;
    size_t powerUpTimer;
    PlayerState state;
    size_t score;
    size_t shootCooldown;
    size_t currentShootCooldown;

public:
    // constructor
    Player(size_t startX, size_t startY, const Sprite* sprite, size_t initialLives = 5);
    
    // disable copy
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    // allow move
    Player(Player&&) = default;
    Player& operator=(Player&&) = default;

    // destructor
    ~Player() = default;

    // other functions
    void drawPlayer(Buffer& buffer, uint32_t colour) const;
    void movePlayer(int direction, size_t bufferWidth);
    void reset(size_t startX, size_t startY, size_t initialLives = 5);
    void loseLife();
    size_t getLives() const;
    void setLives(size_t l);
    size_t getPlayerX() const;
    size_t getPlayerY() const;
    void setPlayerX(size_t newX);

    // New methods
    void setInvincible(bool invincible);
    void updateInvincibility();
    void applyPowerUp(int type);
    void updatePowerUp();
    PlayerState getState() const;
    void setState(PlayerState newState);
    void addScore(size_t points);
    size_t getScore() const;
    bool canShoot() const;
    void setShootCooldown(size_t cooldown);
    void resetShootCooldown();
    bool isReadyToShoot() const;
    void update(double dt) override;
    void hit() { if (!invincible) { loseLife(); setInvincible(true); } }

    bool isInvincible() const;
    size_t getInvincibilityTimer() const;

private:
    void drawPowerUpEffect(Buffer& buffer, uint32_t colour) const;
};

#endif