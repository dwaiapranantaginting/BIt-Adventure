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
    void takeDamage(float knockbackDirX = 0.f);
    int          getHealth() { return health; }
    bool isDying()    { return isDead; }
    bool isDeathDone(){ return deathDone; }

private:
    void applyGravity(float dt);
    void handleMovement(float dt);
    void handleJump();
    void checkCollisions(std::vector<sf::FloatRect>& colliders);
    void updateAnimation(float dt);
    void flipSprite();
    void updateDeathAnimation(float dt);

    sf::Texture  texture;
    sf::Texture  runTexture;
    sf::Texture  jumpTexture;
    sf::Sprite   sprite;

    sf::Vector2f velocity;
    float gravity   = 500.f;
    float moveSpeed = 90.f;
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

    float landingTimer = 0.f;
    float landingTime  = 0.12f; // berapa lama frame landing ditampilkan
    bool  isLanding    = false;
    bool  wasInAir = false;

    sf::Texture hurtTexture;

    float knockbackTimer    = 1.5f;
    float knockbackDuration = 1.0f; // durasi terpental
    bool  isKnockedBack     = false;

    sf::Texture deathTexture;

    bool  isDead       = false;
    bool  deathDone    = false;
    float deathTimer   = 0.f;
    float deathSpeed   = 0.1f; // kecepatan tiap frame death
    int   deathFrame   = 0;
};