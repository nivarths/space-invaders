#ifndef SPRITEANIMATION_H
#define SPRITEANIMATION_H

#include "Sprite.h"
#include <cstddef>

class SpriteAnimation {
private:
    double elapsedSeconds = 0.0;

public:
    bool loop         = false;
    size_t num_frames = 0;
    size_t frame_duration = 0;
    Sprite** frames   = nullptr;

    SpriteAnimation() = default;
    ~SpriteAnimation() { delete[] frames; }

    // no copies
    SpriteAnimation(const SpriteAnimation&) = delete;
    SpriteAnimation& operator=(const SpriteAnimation&) = delete;

    // allow moves
    SpriteAnimation(SpriteAnimation&& o) noexcept
      : loop(o.loop),
        num_frames(o.num_frames),
        frame_duration(o.frame_duration),
        frames(o.frames),
        elapsedSeconds(o.elapsedSeconds)
    {
        o.frames = nullptr;
    }
    SpriteAnimation& operator=(SpriteAnimation&& o) noexcept {
        if (this != &o) {
            delete[] frames;
            loop = o.loop;
            num_frames = o.num_frames;
            frame_duration = o.frame_duration;
            frames = o.frames;
            elapsedSeconds = o.elapsedSeconds;
            o.frames = nullptr;
        }
        return *this;
    }

    // Advance one “tick” (for frame‑based stepping)
    void advance();

    // Reset the *tick* counter (not the elapsedSeconds)
    void reset() { advance(); /* or zero your internal tick if you used one */ }

    // Time‑based update (dt in seconds):
    void update(double dt);

    // Ask if the non‑looping animation has run its course:
    bool isFinished() const;

    // Ask for the current frame (note: only one declaration!)
    const Sprite* getCurrentFrame() const;
};

#endif // SPRITEANIMATION_H
