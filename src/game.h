#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <random>
#include <memory>

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float deltaTime);
    void render();
    void spawnEnemy();
    void spawnStar();
    void checkCollisions();
    void resetGame();
    void updateScoreDisplay();
    void updateLivesDisplay();
    void updateLevelDisplay();
    void playExplosion(sf::Vector2f position);

    sf::RenderWindow window;
    sf::View view;

    // Player
    sf::Sprite player;
    sf::Texture playerTexture;
    sf::Vector2f playerVelocity;
    float playerSpeed;
    int playerLives;
    bool playerInvulnerable;
    float invulnerabilityTimer;
    float invulnerabilityDuration;

    // Bullets
    struct Bullet {
        sf::Sprite sprite;
        sf::Vector2f velocity;
        float lifetime;
        float maxLifetime;
    };
    std::vector<Bullet> bullets;
    sf::Texture bulletTexture;
    float fireRate;
    float fireTimer;

    // Enemies
    struct Enemy {
        sf::Sprite sprite;
        sf::Vector2f velocity;
        int type;
        float spawnTimer;
        int health;
    };
    std::vector<Enemy> enemies;
    std::vector<sf::Texture> enemyTextures;
    float enemySpawnTimer;
    float enemySpawnInterval;

    // Stars (background)
    struct Star {
        sf::Sprite sprite;
        sf::Vector2f velocity;
    };
    std::vector<Star> stars;
    sf::Texture starTexture;
    float starSpawnTimer;
    float starSpawnInterval;

    // Explosions
    struct Explosion {
        sf::Sprite sprite;
        float timer;
        float duration;
    };
    std::vector<Explosion> explosions;
    sf::Texture explosionTexture;

    // Power-ups
    struct PowerUp {
        sf::Sprite sprite;
        sf::Vector2f velocity;
        int type; // 0: health, 1: rapid fire, 2: shield
    };
    std::vector<PowerUp> powerUps;
    sf::Texture powerUpTextures[3];
    float powerUpSpawnTimer;
    float powerUpSpawnInterval;

    // Effects
    struct Particle {
        sf::CircleShape shape;
        sf::Vector2f velocity;
        float lifetime;
        float maxLifetime;
    };
    std::vector<Particle> particles;

    // UI
    sf::Font font;
    sf::Text scoreText;
    sf::Text livesText;
    sf::Text levelText;
    sf::Text gameOverText;
    sf::Text restartText;
    sf::Text titleText;
    sf::Text startText;
    sf::RectangleShape healthBar;
    sf::RectangleShape healthBarBackground;

    // Audio
    sf::SoundBuffer shootBuffer;
    sf::SoundBuffer explosionBuffer;
    sf::SoundBuffer powerUpBuffer;
    sf::SoundBuffer gameOverBuffer;
    sf::Sound shootSound;
    sf::Sound explosionSound;
    sf::Sound powerUpSound;
    sf::Sound gameOverSound;
    sf::Music backgroundMusic;

    // Game state
    int score;
    int level;
    int enemiesKilled;
    int enemiesToNextLevel;
    bool gameOver;
    bool gameStarted;
    bool paused;

    // Random
    std::mt19937 rng;
    std::uniform_real_distribution<float> floatDist;
    std::uniform_int_distribution<int> intDist;

    // Window size
    static constexpr int WINDOW_WIDTH = 1280;
    static constexpr int WINDOW_HEIGHT = 720;
    static constexpr float VIEW_HEIGHT = 720.0f;
};
