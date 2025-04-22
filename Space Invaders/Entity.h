#ifndef ENTITY_H
#define ENTITY_H

#include <cstddef>
#include "Buffer.h"
#include "Sprite.h"

class Entity {
protected:
    struct Point {
        size_t x;
        size_t y;
    };

    struct Bounds {
        size_t x;
        size_t y;
        size_t width;
        size_t height;

        bool intersects(const Bounds& other) const {
            return !(x + width <= other.x ||
                    other.x + other.width <= x ||
                    y + height <= other.y ||
                    other.y + other.height <= y);
        }
    };

    Point position;
    const Sprite* sprite;
    float moveSpeed;
    Bounds bounds;

public:
    Entity(size_t x, size_t y, const Sprite* spr, float speed = 1.0f);
    virtual ~Entity() = default;

    // Disable copy
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    // Allow move
    Entity(Entity&&) = default;
    Entity& operator=(Entity&&) = default;

    void draw(Buffer& buffer, uint32_t color) const;
    void moveX(int dx);
    void moveY(int dy);
    void setPosition(size_t x, size_t y);
    void setSprite(const Sprite* spr);
    void setMoveSpeed(float speed);

    size_t getX() const;
    size_t getY() const;
    float getMoveSpeed() const;
    const Sprite* getSprite() const;
    const Bounds& getBounds() const;

    bool checkCollision(const Entity& other) const;

protected:
    void updateBounds();

    // Update
    virtual void update(double dt) = 0;
};

#endif 