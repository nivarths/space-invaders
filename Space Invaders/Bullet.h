#ifndef BULLET_H
#define BULLET_H

#include "Entity.h"
#include "Sprite.h"
#include <cstdint>

enum class BulletType {
    PLAYER,
    ALIEN,
    SPECIAL
};

class Bullet : public Entity {
private:
    BulletType type;
    size_t damage;
    bool hasTrail;
    size_t trailTimer;

public:
    // constructor
    Bullet(size_t startX, size_t startY, const Sprite* sprite, BulletType bulletType);

    // destructor
    ~Bullet() = default;

    // drawing bullet
    void drawBullet(Buffer& buffer, uint32_t colour) const;

    // move function
    void moveBullet(int direction);

    // reset function
    void reset(size_t startX, size_t startY);

    // New methods
    void setType(BulletType newType);
    BulletType getType() const;
    void setDamage(size_t newDamage);
    size_t getDamage() const;
    void enableTrail(bool enable);
    void updateTrail();
    bool hasTrailEffect() const;
    void update(double dt) override;
};

#endif