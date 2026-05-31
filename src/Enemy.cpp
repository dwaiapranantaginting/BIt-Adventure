#include "Enemy.h"
#include <iostream>
#include <cmath>
#include <algorithm>

// ================= PELURU MUSUH =================
// ================= PELURU MUSUH =================
EnemyProjectile::EnemyProjectile(float x, float y, float dirX, float dirY, const sf::Texture& tex, bool facingRight) 
    : sprite(tex) 
{
    // Set origin peluru ke tengah
    sprite.setOrigin({tex.getSize().x / 2.f, tex.getSize().y / 2.f});
    sprite.setPosition({x, y});

    // --- KECILKAN UKURAN PELURU DI SINI ---
    // Ubah angka 0.4f (40%) ini kalau masih kurang kecil atau kurang besar
    float skalaPeluru = 0.4f; 

    // Sesuaikan arah hadap gambar peluru sekaligus ukurannya
    if (facingRight) sprite.setScale({-skalaPeluru, skalaPeluru});
    else sprite.setScale({skalaPeluru, skalaPeluru});

    float speed = 80.f; // Peluru jauh lebih lambat
    velocity = {dirX * speed, dirY * speed};
}

void EnemyProjectile::update(float dt) {
    lifetime -= dt;
    if (lifetime <= 0.f) alive = false;
    sprite.move(velocity * dt);
}

void EnemyProjectile::draw(sf::RenderTarget& target) {
    if (alive) target.draw(sprite);
}

sf::FloatRect EnemyProjectile::getBounds() {
    return sprite.getGlobalBounds();
}

// ================= MUSUH BIASA =================
Enemy::Enemy(float x, float y) : sprite(tIdle) {
    bool l1 = tIdle.loadFromFile("assets/sprites/enemy_idle.png"); // Sesuaikan namamu
    bool l2 = tFly.loadFromFile("assets/sprites/enemy_fly.png");
    bool l3 = tAttack.loadFromFile("assets/sprites/enemy_attack.png");
    bool l4 = tDeath.loadFromFile("assets/sprites/enemy_death.png");
    
    // UBAH INI DENGAN NAMA GAMBAR PELURU MUSUHMU
    bool l5 = tProj.loadFromFile("assets/sprites/enemy_bullet.png");

    sprite.setTexture(tIdle);
    
    int startLebar = tIdle.getSize().x / 4; 
    int startTinggi = tIdle.getSize().y;

    sprite.setTextureRect(sf::IntRect({0, 0}, {startLebar, startTinggi}));
    sprite.setOrigin({startLebar / 2.f, startTinggi / 2.f});
    sprite.setPosition({x, y});
}

void Enemy::update(float dt, sf::Vector2f playerPos) {
    if (isDead) {
        updateAnimation(dt);
        return;
    }

    if (attackCooldown > 0.f) attackCooldown -= dt;

    sf::Vector2f myPos = sprite.getPosition();
    float dx = playerPos.x - myPos.x;
    float dy = playerPos.y - myPos.y;
    float dist = std::sqrt(dx*dx + dy*dy);

    facingRight = (dx > 0.f);
    flipSprite();

    if (!isAttacking) {
        if (dist <= attackRange && attackCooldown <= 0.f) {
            state = EnemyState::ATTACK;
            isAttacking = true;
            currentFrame = 0;
            velocity = {0.f, 0.f};
            
            float dirX = dx / dist;
            float dirY = dy / dist;
            
            // Melempar tekstur peluru ke proyektil
            projectiles.emplace_back(myPos.x, myPos.y, dirX, dirY, tProj, facingRight);
            
            attackCooldown = 4.0f;
        } 
        else if (dist <= chaseRange && dist > attackRange) {
            state = EnemyState::FLY;
            velocity.x = (dx / dist) * moveSpeed;
            velocity.y = (dy / dist) * moveSpeed; 
        } 
        else {
            state = EnemyState::IDLE;
            velocity = {0.f, 0.f};
        }
    }

    sprite.move(velocity * dt);
    updateAnimation(dt);

    for (auto& p : projectiles) p.update(dt);
    projectiles.erase(
        std::remove_if(projectiles.begin(), projectiles.end(),
            [](EnemyProjectile& p){ return !p.isAlive(); }),
        projectiles.end());
}

void Enemy::takeDamage() {
    if (isDead) return;
    hp--;
    if (hp <= 0) {
        isDead = true;
        state = EnemyState::DEATH;
        currentFrame = 0;
        velocity = {0.f, 0.f};
    }
}

void Enemy::updateAnimation(float dt) {
    if (state != prevState) {
        currentFrame = 0;
        animTimer = 0.f;
        prevState = state;
    }

    animTimer += dt;
    if (animTimer >= animSpeed) {
        animTimer = 0.f;
        currentFrame++;

        int frameLebar = 0;
        int frameTinggi = 0;

        if (state == EnemyState::IDLE) {
            frameLebar = tIdle.getSize().x / 4;
            frameTinggi = tIdle.getSize().y;
            if (currentFrame > 3) currentFrame = 0; 
            
            sprite.setTexture(tIdle);
            sprite.setTextureRect(sf::IntRect({currentFrame * frameLebar, 0}, {frameLebar, frameTinggi}));
            sprite.setOrigin({frameLebar / 2.f, frameTinggi / 2.f});
        } 
        else if (state == EnemyState::FLY) {
            frameLebar = tFly.getSize().x / 4;
            frameTinggi = tFly.getSize().y;
            if (currentFrame > 3) currentFrame = 0;
            
            sprite.setTexture(tFly);
            sprite.setTextureRect(sf::IntRect({currentFrame * frameLebar, 0}, {frameLebar, frameTinggi}));
            sprite.setOrigin({frameLebar / 2.f, frameTinggi / 2.f});
        }
        else if (state == EnemyState::ATTACK) {
            frameLebar = tAttack.getSize().x / 8;
            frameTinggi = tAttack.getSize().y;
            if (currentFrame > 7) {
                isAttacking = false; 
                state = EnemyState::IDLE;
            } else {
                sprite.setTexture(tAttack);
                sprite.setTextureRect(sf::IntRect({currentFrame * frameLebar, 0}, {frameLebar, frameTinggi}));
                sprite.setOrigin({frameLebar / 2.f, frameTinggi / 2.f});
            }
        }
        else if (state == EnemyState::DEATH) {
            frameLebar = tDeath.getSize().x / 6;
            frameTinggi = tDeath.getSize().y;
            sprite.setTexture(tDeath);
            if (currentFrame > 5) {
                currentFrame = 5; 
                isDeathDone = true;
            } else {
                sprite.setTextureRect(sf::IntRect({currentFrame * frameLebar, 0}, {frameLebar, frameTinggi}));
                sprite.setOrigin({frameLebar / 2.f, frameTinggi / 2.f});
            }
        }
    }
}

void Enemy::flipSprite() {
    float skala = 0.45f; 
    
    // KARENA GAMBAR ASLI MENGHADAP KIRI, LOGIKA DIBALIK
    if (facingRight) {
        sprite.setScale({-skala, skala}); // Negatif agar berbalik ke kanan
    } else {
        sprite.setScale({skala, skala});  // Positif agar tetap ke kiri
    }
}

void Enemy::draw(sf::RenderTarget& target) {
    target.draw(sprite);
    for (auto& p : projectiles) p.draw(target);
}

sf::FloatRect Enemy::getBounds() {
    return sprite.getGlobalBounds();
}