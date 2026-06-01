#pragma once
#include <SFML/Graphics.hpp>

enum class BossState { IDLE, RUN, ATTACK };

class Boss {
public:
    Boss(float x, float y);

    void update(float dt, sf::Vector2f playerPos,
                std::vector<sf::FloatRect>& colliders);
    void draw(sf::RenderTarget& target);
    bool shouldDamagePlayer();
    sf::FloatRect getBounds();
    void applyKnockback(float dirX);

    void forcePacify();
    void forceWalkRight(float dt);

    bool isPacified = false;

private:
    void applyGravity(float dt);
    void chasePlayer(float dt, sf::Vector2f playerPos);
    void checkCollisions(std::vector<sf::FloatRect>& colliders);
    void updateAnimation(float dt);
    void flipSprite();

    sf::Texture  idleTexture;
    sf::Texture  runTexture;
    sf::Sprite   sprite;

    sf::Vector2f velocity;
    float gravity   = 500.f;
    float moveSpeed = 50.f;  // agak lambat biar terasa berat/mengancam

    BossState state     = BossState::IDLE;
    BossState prevState = BossState::IDLE;
    bool facingRight    = true;
    bool isOnGround     = false;

    float animTimer    = 0.f;
    float animSpeed    = 0.12f;
    int   currentFrame = 0;

    // Jarak minimum sebelum boss mulai kejar player
    float chaseRange = 600.f;

    sf::Texture  attackTexture;  // tambah ini

    bool  isAttacking    = false;
    float attackTimer    = 0.f;
    float attackDuration = 0.f;
    int   attackFrame    = 0;
    bool  hasDealtDamage = false; // pastikan damage hanya sekali per serangan

    float attackRange = 32.f;

    float attackCooldown    = 0.f;
    float attackCooldownMax = 0.8f; 

    bool  isKnockedBack    = false;
    float knockbackTimer   = 0.f;
    float knockbackDuration = 0.4f;
    sf::Vector2f knockbackVelocity;
};