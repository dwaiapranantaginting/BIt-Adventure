#pragma once
#include <SFML/Graphics.hpp>

enum class BossState { IDLE, RUN };

class Boss {
public:
    Boss(float x, float y);

    void update(float dt, sf::Vector2f playerPos,
                std::vector<sf::FloatRect>& colliders);
    void draw(sf::RenderTarget& target);
    sf::FloatRect getBounds();

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
    float chaseRange = 200.f;
};