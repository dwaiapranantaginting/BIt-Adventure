#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// ================= PELURU MUSUH =================
class EnemyProjectile {
public:
    // Menerima referensi gambar peluru
    EnemyProjectile(float x, float y, float dirX, float dirY, const sf::Texture& tex, bool facingRight);
    void update(float dt);
    void draw(sf::RenderTarget& target);
    sf::FloatRect getBounds();
    bool isAlive() { return alive; }
    void kill() { alive = false; }

private:
    sf::Sprite sprite;
    sf::Vector2f velocity;
    float lifetime = 1.2f; // Jarak peluru diperpendek (hilang dalam 1.2 detik)
    bool alive = true;
};

// ================= MUSUH BIASA =================
enum class EnemyState { IDLE, FLY, ATTACK, DEATH };

class Enemy {
public:
    Enemy(float x, float y);
    void update(float dt, sf::Vector2f playerPos);
    void draw(sf::RenderTarget& target);
    sf::FloatRect getBounds();
    
    void takeDamage();
    bool isDead = false;
    bool isDeathDone = false;
    
    std::vector<EnemyProjectile>& getProjectiles() { return projectiles; }

private:
    void updateAnimation(float dt);
    void flipSprite();

    // Tambah tekstur peluru (tProj)
    sf::Texture tIdle, tFly, tAttack, tDeath, tProj; 
    sf::Sprite sprite;

    EnemyState state = EnemyState::IDLE;
    EnemyState prevState = EnemyState::IDLE;

    sf::Vector2f velocity;
    float moveSpeed = 35.f; // Gerak musuh diperlambat
    bool facingRight = false;
    int hp = 2; 

    float animTimer = 0.f;
    float animSpeed = 0.15f;
    int currentFrame = 0;

    float chaseRange = 200.f;  // Diperpendek agar muncul di layar dulu
    float attackRange = 120.f; // Hanya menembak kalau sudah cukup dekat
    float attackCooldown = 0.f;
    bool isAttacking = false;

    std::vector<EnemyProjectile> projectiles;
};