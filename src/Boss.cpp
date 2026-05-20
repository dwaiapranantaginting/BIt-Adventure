#include "Boss.h"
#include <iostream>
#include <algorithm>
#include <cmath>

Boss::Boss(float x, float y)
    : sprite(idleTexture)
{
    if (!idleTexture.loadFromFile("assets/sprites/rika_idle.png"))
        std::cerr << "[ERROR] Gagal load boss_idle.png!\n";

    if (!runTexture.loadFromFile("assets/sprites/rika_run.png"))
        std::cerr << "[ERROR] Gagal load boss_run.png!\n";

    if (!attackTexture.loadFromFile("assets/sprites/attacking_rika.png"))
        std::cerr << "[ERROR] Gagal load attacking_rika.png!\n";

    sprite = sf::Sprite(idleTexture);
    sprite.setTextureRect(sf::IntRect({0, 0}, {64, 64}));
    sprite.setOrigin({32.f, 64.f});  // tengah-bawah
    sprite.setPosition({x, y});
}

void Boss::update(float dt, sf::Vector2f playerPos,
                  std::vector<sf::FloatRect>& colliders)
{
    chasePlayer(dt, playerPos);
    sprite.move(velocity * dt);  // tidak ada gravity, tidak ada collision
    updateAnimation(dt);
}

void Boss::chasePlayer(float dt, sf::Vector2f playerPos) {
    sf::Vector2f bossPos = sprite.getPosition();
    float dx = playerPos.x - bossPos.x;
    float dy = playerPos.y - bossPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    facingRight = (dx > 0.f);
    flipSprite();

    // Kalau lagi attacking, jangan gerak
    if (isAttacking) {
        velocity.x = 0.f;
        velocity.y = 0.f;
        return;
    }

    // Cooldown setelah attack selesai
    if (attackCooldown > 0.f) {
        attackCooldown -= dt;
        velocity.x = 0.f;
        velocity.y = 0.f;
        state      = BossState::IDLE;
        return;
    }

    if (dist <= attackRange) {
        velocity.x     = 0.f;
        velocity.y     = 0.f;
        state          = BossState::ATTACK;
        isAttacking    = true;
        attackFrame    = 0;
        currentFrame   = 0;
        animTimer      = 0.f;
        hasDealtDamage = false;
    } else if (dist > 4.f) {
        velocity.x = (dx / dist) * moveSpeed;
        velocity.y = (dy / dist) * moveSpeed;
        state      = BossState::RUN;
    } else {
        velocity.x = 0.f;
        velocity.y = 0.f;
        state      = BossState::IDLE;
    }
}

void Boss::applyGravity(float dt) {
    velocity.y += gravity * dt;
}

void Boss::checkCollisions(std::vector<sf::FloatRect>& colliders) {
    for (auto& bounds : colliders) {
        sf::FloatRect bb = sprite.getGlobalBounds();

        // Shrink hitbox sedikit biar tidak terlalu besar
        float shrinkX   = 16.f;
        float shrinkTop = 8.f;

        bb.position.x += shrinkX;
        bb.size.x     -= shrinkX * 2.f;
        bb.position.y += shrinkTop;
        bb.size.y     -= shrinkTop;

        float bLeft   = bounds.position.x;
        float bRight  = bounds.position.x + bounds.size.x;
        float bTop    = bounds.position.y;
        float bBottom = bounds.position.y + bounds.size.y;

        float eLeft   = bb.position.x;
        float eRight  = bb.position.x + bb.size.x;
        float eTop    = bb.position.y;
        float eBottom = bb.position.y + bb.size.y;

        if (eRight <= bLeft || eLeft >= bRight ||
            eBottom <= bTop || eTop >= bBottom) continue;

        float overlapTop    = eBottom - bTop;
        float overlapBottom = bBottom - eTop;
        float overlapLeft   = eRight  - bLeft;
        float overlapRight  = bRight  - eLeft;

        float minOverlap = std::min({overlapTop, overlapBottom,
                                     overlapLeft, overlapRight});

        if (overlapTop < 12.f && velocity.y >= 0.f)
            minOverlap = overlapTop;

        if (minOverlap == overlapTop) {
            sprite.setPosition(sf::Vector2f(
                sprite.getPosition().x,
                bTop + shrinkTop));
            velocity.y = 0.f;
            isOnGround = true;
        } else if (minOverlap == overlapBottom) {
            sprite.setPosition(sf::Vector2f(
                sprite.getPosition().x,
                sprite.getPosition().y + overlapBottom));
            velocity.y = 0.f;
        } else if (minOverlap == overlapLeft) {
            sprite.setPosition(sf::Vector2f(
                sprite.getPosition().x - overlapLeft,
                sprite.getPosition().y));
        } else if (minOverlap == overlapRight) {
            sprite.setPosition(sf::Vector2f(
                sprite.getPosition().x + overlapRight,
                sprite.getPosition().y));
        }
    }
}

void Boss::updateAnimation(float dt) {
    if (state != prevState) {
        currentFrame = 0;
        animTimer    = 0.f;
        prevState    = state;
    }

    animTimer += dt;
    if (animTimer >= animSpeed) {
        animTimer = 0.f;
        currentFrame++;

        switch (state) {
            case BossState::IDLE:
                if (currentFrame >= 8) currentFrame = 0;
                sprite.setTexture(idleTexture);
                sprite.setTextureRect(
                    sf::IntRect({currentFrame * 64, 0}, {64, 64}));
                break;

            case BossState::RUN:
                if (currentFrame >= 6) currentFrame = 0;
                sprite.setTexture(runTexture);
                sprite.setTextureRect(
                    sf::IntRect({currentFrame * 64, 0}, {64, 64}));
                break;

            case BossState::ATTACK:
                sprite.setTexture(attackTexture);
                if (currentFrame >= 5) {
                    // Animasi attack selesai
                    currentFrame   = 0;
                    isAttacking    = false;
                    hasDealtDamage = false;
                    attackCooldown = attackCooldownMax; // mulai delay
                    state          = BossState::IDLE;
                    prevState      = BossState::IDLE;
                } else {
                    sprite.setTextureRect(
                        sf::IntRect({currentFrame * 64, 0}, {64, 64}));
                }
                break;
        }
    }
}

void Boss::flipSprite() {
    if (facingRight) {
        sprite.setScale({1.f, 1.f});
        sprite.setOrigin({32.f, 64.f});
    } else {
        sprite.setScale({-1.f, 1.f});
        sprite.setOrigin({32.f, 64.f});
    }
}

sf::FloatRect Boss::getBounds() {
    return sprite.getGlobalBounds();
}

bool Boss::shouldDamagePlayer() {
    // Damage di frame ke-2 (frame terakhir sebelum selesai)
    if (isAttacking && currentFrame == 4 && !hasDealtDamage) {
        hasDealtDamage = true;
        return true;
    }
    return false;
}

void Boss::draw(sf::RenderTarget& target) {
    target.draw(sprite);
}

