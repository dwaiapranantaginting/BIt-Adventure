#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

enum class PlayerState { IDLE, RUN, JUMP, DUCK, HURT };

class Player {
public:
    Player();

    void         handleInput();
    void         update(float dt, std::vector<sf::FloatRect>& colliders);
    void         setPosition(float x, float y);
    sf::Sprite&  getSprite();
    sf::Vector2f getPosition();
    void         takeDamage();
    int          getHealth() { return health; }

private:
    void applyGravity(float dt);
    void handleMovement(float dt);
    void handleJump();
    void checkCollisions(std::vector<sf::FloatRect>& colliders);
    void updateAnimation(float dt);
    void flipSprite();

    sf::Texture  texture;
    sf::Texture  runTexture;
    sf::Sprite   sprite;

    sf::Vector2f velocity;
    float gravity   = 500.f;
    float moveSpeed = 70.f;
    float jumpForce = -200.f;

    PlayerState state     = PlayerState::IDLE;
    PlayerState prevState = PlayerState::IDLE;
    bool isOnGround       = false;
    bool facingRight      = true;
    bool wantsJump        = false;

    float animTimer    = 0.f;
    float animSpeed    = 0.15f;
    int   currentFrame = 0;

    int maxHealth  = 3;
    int health     = 3;

    float invincTimer  = 0.f;
    float invincTime   = 1.5f;
    bool  isInvincible = false;
};