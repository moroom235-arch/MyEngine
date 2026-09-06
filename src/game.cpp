#include "game.h"
#include <iostream>
#include <algorithm>

Game::Game() 
    : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Space Shooter - موروم 235", sf::Style::Close | sf::Style::Titlebar),
      playerSpeed(400.0f),
      playerLives(3),
      playerInvulnerable(false),
      invulnerabilityTimer(0.0f),
      invulnerabilityDuration(2.0f),
      fireRate(0.2f),
      fireTimer(0.0f),
      enemySpawnTimer(0.0f),
      enemySpawnInterval(1.5f),
      starSpawnTimer(0.0f),
      starSpawnInterval(0.1f),
      powerUpSpawnTimer(0.0f),
      powerUpSpawnInterval(10.0f),
      score(0),
      level(1),
      enemiesKilled(0),
      enemiesToNextLevel(10),
      gameOver(false),
      gameStarted(false),
      paused(false),
      rng(std::random_device()()),
      floatDist(0.0f, 1.0f),
      intDist(0, 100)
{
    // Set up view
    view.setSize(WINDOW_WIDTH, VIEW_HEIGHT);
    view.setCenter(WINDOW_WIDTH / 2.0f, VIEW_HEIGHT / 2.0f);
    window.setView(view);
    window.setFramerateLimit(60);

    // Load fonts
    if (!font.loadFromFile("assets/fonts/arial.ttf")) {
        // Fallback to default font
        if (!font.loadFromFile("/usr/share/fonts/truetype/arial.ttf")) {
            if (!font.loadFromFile("/usr/share/fonts/TTF/DejaVuSans.ttf")) {
                std::cerr << "Failed to load font, using default" << std::endl;
            }
        }
    }

    // Create placeholder textures programmatically
    createPlaceholderTextures();

    // Initialize player
    initPlayer();

    // Initialize UI
    initUI();

    // Initialize audio (will use placeholder sounds)
    initAudio();
}

void Game::createPlaceholderTextures() {
    // Player texture (triangle spaceship)
    playerTexture.create(64, 64, sf::Color::Transparent);
    sf::Image playerImage;
    playerImage.create(64, 64, sf::Color::Transparent);
    
    // Draw a simple spaceship
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            if (y > 32 && x > 16 && x < 48 && (y - 32) < (48 - x) && (y - 32) < (x - 16)) {
                playerImage.setPixel(x, y, sf::Color(0, 200, 255));
            }
            if (y > 48 && x > 24 && x < 40) {
                playerImage.setPixel(x, y, sf::Color(255, 100, 0));
            }
        }
    }
    playerTexture.update(playerImage);
    player.setTexture(playerTexture);
    player.setOrigin(32, 32);
    player.setScale(1.5f, 1.5f);

    // Bullet texture
    bulletTexture.create(8, 32, sf::Color::Transparent);
    sf::Image bulletImage;
    bulletImage.create(8, 32, sf::Color::Transparent);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 8; x++) {
            bulletImage.setPixel(x, y, sf::Color(255, 255, 0));
        }
    }
    bulletTexture.update(bulletImage);

    // Enemy textures (3 types)
    for (int i = 0; i < 3; i++) {
        enemyTextures.emplace_back();
        enemyTextures[i].create(48, 48, sf::Color::Transparent);
        sf::Image enemyImg;
        enemyImg.create(48, 48, sf::Color::Transparent);
        
        sf::Color enemyColor;
        switch(i) {
            case 0: enemyColor = sf::Color(255, 50, 50); break; // Red - basic
            case 1: enemyColor = sf::Color(200, 100, 255); break; // Purple - medium
            case 2: enemyColor = sf::Color(255, 200, 50); break; // Yellow - hard
        }
        
        // Draw a simple enemy ship
        for (int y = 0; y < 48; y++) {
            for (int x = 0; x < 48; x++) {
                float dist = std::sqrt((x - 24) * (x - 24) + (y - 24) * (y - 24));
                if (dist < 20 && dist > 12) {
                    enemyImg.setPixel(x, y, enemyColor);
                }
                if (dist < 8) {
                    enemyImg.setPixel(x, y, sf::Color(255, 255, 255));
                }
            }
        }
        enemyTextures[i].update(enemyImg);
    }

    // Star texture
    starTexture.create(4, 4, sf::Color::Transparent);
    sf::Image starImg;
    starImg.create(4, 4, sf::Color::Transparent);
    starImg.setPixel(0, 0, sf::Color::White);
    starImg.setPixel(1, 1, sf::Color::White);
    starImg.setPixel(2, 2, sf::Color::White);
    starImg.setPixel(3, 3, sf::Color::White);
    starTexture.update(starImg);

    // Explosion texture
    explosionTexture.create(64, 64, sf::Color::Transparent);
    sf::Image explosionImg;
    explosionImg.create(64, 64, sf::Color::Transparent);
    for (int i = 0; i < 20; i++) {
        float angle = floatDist(rng) * 3.14159f * 2;
        float dist = floatDist(rng) * 30;
        int x = 32 + std::cos(angle) * dist;
        int y = 32 + std::sin(angle) * dist;
        if (x >= 0 && x < 64 && y >= 0 && y < 64) {
            sf::Color color(intDist(rng) % 2 ? sf::Color::Red : sf::Color::Yellow);
            explosionImg.setPixel(x, y, color);
            explosionImg.setPixel(x + 1, y, color);
            explosionImg.setPixel(x, y + 1, color);
        }
    }
    explosionTexture.update(explosionImg);

    // Power-up textures
    sf::Color powerUpColors[3] = {sf::Color::Green, sf::Color::Blue, sf::Color::Magenta};
    for (int i = 0; i < 3; i++) {
        powerUpTextures[i].create(32, 32, sf::Color::Transparent);
        sf::Image pupImg;
        pupImg.create(32, 32, sf::Color::Transparent);
        for (int y = 0; y < 32; y++) {
            for (int x = 0; x < 32; x++) {
                float dist = std::sqrt((x - 16) * (x - 16) + (y - 16) * (y - 16));
                if (dist < 12 && dist > 6) {
                    pupImg.setPixel(x, y, powerUpColors[i]);
                }
                if (dist < 4) {
                    pupImg.setPixel(x, y, sf::Color::White);
                }
            }
        }
        powerUpTextures[i].update(pupImg);
    }
}

void Game::initPlayer() {
    player.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 100.0f);
    playerVelocity = sf::Vector2f(0, 0);
}

void Game::initUI() {
    // Score text
    scoreText.setFont(font);
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(20, 20);
    updateScoreDisplay();

    // Lives text
    livesText.setFont(font);
    livesText.setCharacterSize(24);
    livesText.setFillColor(sf::Color::White);
    livesText.setPosition(20, 50);
    updateLivesDisplay();

    // Level text
    levelText.setFont(font);
    levelText.setCharacterSize(24);
    levelText.setFillColor(sf::Color::White);
    levelText.setPosition(20, 80);
    updateLevelDisplay();

    // Game over text
    gameOverText.setFont(font);
    gameOverText.setCharacterSize(64);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setString("GAME OVER");
    sf::FloatRect bounds = gameOverText.getLocalBounds();
    gameOverText.setOrigin(bounds.width / 2, bounds.height / 2);
    gameOverText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 100);

    // Restart text
    restartText.setFont(font);
    restartText.setCharacterSize(32);
    restartText.setFillColor(sf::Color::White);
    restartText.setString("Press R to Restart");
    bounds = restartText.getLocalBounds();
    restartText.setOrigin(bounds.width / 2, bounds.height / 2);
    restartText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 50);

    // Title text
    titleText.setFont(font);
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color(0, 200, 255));
    titleText.setString("SPACE SHOOTER");
    bounds = titleText.getLocalBounds();
    titleText.setOrigin(bounds.width / 2, bounds.height / 2);
    titleText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 100);

    // Start text
    startText.setFont(font);
    startText.setCharacterSize(28);
    startText.setFillColor(sf::Color::White);
    startText.setString("Press SPACE to Start");
    bounds = startText.getLocalBounds();
    startText.setOrigin(bounds.width / 2, bounds.height / 2);
    startText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 50);

    // Health bar
    healthBarBackground.setSize(sf::Vector2f(204, 24));
    healthBarBackground.setFillColor(sf::Color(50, 50, 50));
    healthBarBackground.setPosition(WINDOW_WIDTH - 220, 20);
    
    healthBar.setSize(sf::Vector2f(200, 20));
    healthBar.setFillColor(sf::Color::Green);
    healthBar.setPosition(WINDOW_WIDTH - 218, 22);
}

void Game::initAudio() {
    // We'll generate simple placeholder sounds
    // In a real game, you'd load actual sound files
}

void Game::run() {
    sf::Clock clock;
    
    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        
        processEvents();
        update(deltaTime);
        render();
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                if (gameOver) {
                    window.close();
                } else if (gameStarted) {
                    paused = !paused;
                }
            }

            if (event.key.code == sf::Keyboard::R && gameOver) {
                resetGame();
            }

            if (event.key.code == sf::Keyboard::Space) {
                if (!gameStarted && !gameOver) {
                    gameStarted = true;
                } else if (gameStarted && !gameOver && !paused) {
                    // Fire
                    if (fireTimer >= fireRate) {
                        fireTimer = 0.0f;
                        
                        Bullet bullet;
                        bullet.sprite.setTexture(bulletTexture);
                        bullet.sprite.setOrigin(4, 0);
                        bullet.sprite.setPosition(player.getPosition().x, player.getPosition().y - 30);
                        bullet.sprite.setScale(1.5f, 1.5f);
                        bullet.velocity = sf::Vector2f(0, -800);
                        bullet.lifetime = 0.0f;
                        bullet.maxLifetime = 2.0f;
                        bullets.push_back(bullet);

                        // Play shoot sound
                        shootSound.play();

                        // Create muzzle flash particles
                        for (int i = 0; i < 5; i++) {
                            Particle p;
                            p.shape.setRadius(2);
                            p.shape.setFillColor(sf::Color(255, 255, 200));
                            p.velocity = sf::Vector2f(
                                (floatDist(rng) - 0.5f) * 200,
                                -100 - floatDist(rng) * 100
                            );
                            p.shape.setPosition(player.getPosition().x - 2, player.getPosition().y - 30);
                            p.lifetime = 0.0f;
                            p.maxLifetime = 0.3f;
                            particles.push_back(p);
                        }
                    }
                }
            }

            if (event.key.code == sf::Keyboard::P && gameStarted && !gameOver) {
                paused = !paused;
            }
        }
    }
}

void Game::update(float deltaTime) {
    if (paused || !gameStarted) {
        if (!gameStarted) {
            // Animate title on main menu
            static float titleTimer = 0;
            titleTimer += deltaTime;
            float alpha = 128 + 127 * std::sin(titleTimer * 2);
            titleText.setFillColor(sf::Color(0, 200, 255, static_cast<sf::Uint8>(alpha)));
            startText.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));
        }
        return;
    }

    if (gameOver) {
        return;
    }

    // Update timers
    fireTimer += deltaTime;
    enemySpawnTimer += deltaTime;
    starSpawnTimer += deltaTime;
    powerUpSpawnTimer += deltaTime;

    if (playerInvulnerable) {
        invulnerabilityTimer += deltaTime;
        if (invulnerabilityTimer >= invulnerabilityDuration) {
            playerInvulnerable = false;
            invulnerabilityTimer = 0.0f;
            player.setColor(sf::Color::White);
        }
    }

    // Player input
    playerVelocity = sf::Vector2f(0, 0);
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        playerVelocity.x -= playerSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        playerVelocity.x += playerSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        playerVelocity.y -= playerSpeed;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        playerVelocity.y += playerSpeed;
    }

    // Normalize diagonal movement
    if (playerVelocity.x != 0 && playerVelocity.y != 0) {
        float length = std::sqrt(playerVelocity.x * playerVelocity.x + playerVelocity.y * playerVelocity.y);
        playerVelocity = sf::Vector2f(
            playerVelocity.x / length * playerSpeed,
            playerVelocity.y / length * playerSpeed
        );
    }

    // Update player position
    sf::Vector2f newPos = player.getPosition() + playerVelocity * deltaTime;
    
    // Keep player in bounds
    float playerHalfWidth = player.getLocalBounds().width / 2;
    float playerHalfHeight = player.getLocalBounds().height / 2;
    newPos.x = std::max(playerHalfWidth, std::min(float(WINDOW_WIDTH - playerHalfWidth), newPos.x));
    newPos.y = std::max(playerHalfHeight, std::min(float(WINDOW_HEIGHT - playerHalfHeight), newPos.y));
    
    player.setPosition(newPos);

    // Player invulnerability blink effect
    if (playerInvulnerable) {
        float blink = std::sin(invulnerabilityTimer * 20) * 100 + 155;
        player.setColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(blink)));
    }

    // Spawn stars
    while (starSpawnTimer >= starSpawnInterval) {
        starSpawnTimer -= starSpawnInterval;
        spawnStar();
    }

    // Spawn enemies
    while (enemySpawnTimer >= enemySpawnInterval) {
        enemySpawnTimer -= enemySpawnInterval;
        spawnEnemy();
        
        // Decrease spawn interval as level increases
        enemySpawnInterval = std::max(0.3f, 1.5f - level * 0.1f);
    }

    // Spawn power-ups
    if (powerUpSpawnTimer >= powerUpSpawnInterval) {
        powerUpSpawnTimer = 0.0f;
        powerUpSpawnInterval = 10.0f + floatDist(rng) * 5;
        
        PowerUp powerUp;
        powerUp.type = intDist(rng) % 3;
        powerUp.sprite.setTexture(powerUpTextures[powerUp.type]);
        powerUp.sprite.setOrigin(16, 16);
        powerUp.sprite.setPosition(
            floatDist(rng) * (WINDOW_WIDTH - 40) + 20,
            -32
        );
        powerUp.velocity = sf::Vector2f(
            (floatDist(rng) - 0.5f) * 100,
            200 + floatDist(rng) * 100
        );
        powerUps.push_back(powerUp);
    }

    // Update bullets
    for (size_t i = 0; i < bullets.size(); ) {
        bullets[i].lifetime += deltaTime;
        bullets[i].sprite.move(bullets[i].velocity * deltaTime);
        
        if (bullets[i].lifetime >= bullets[i].maxLifetime || 
            bullets[i].sprite.getPosition().y < -50) {
            bullets.erase(bullets.begin() + i);
        } else {
            i++;
        }
    }

    // Update enemies
    for (size_t i = 0; i < enemies.size(); ) {
        enemies[i].sprite.move(enemies[i].velocity * deltaTime);
        
        // Simple enemy AI: move towards player occasionally
        if (intDist(rng) % 100 == 0) {
            sf::Vector2f dir = player.getPosition() - enemies[i].sprite.getPosition();
            float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (length > 0) {
                dir = dir / length;
                enemies[i].velocity = dir * (100 + enemies[i].type * 50);
            }
        }
        
        // Remove enemies that are out of bounds
        if (enemies[i].sprite.getPosition().y > WINDOW_HEIGHT + 100 ||
            enemies[i].sprite.getPosition().x < -50 ||
            enemies[i].sprite.getPosition().x > WINDOW_WIDTH + 50) {
            enemies.erase(enemies.begin() + i);
        } else {
            i++;
        }
    }

    // Update stars
    for (size_t i = 0; i < stars.size(); ) {
        stars[i].sprite.move(stars[i].velocity * deltaTime);
        
        if (stars[i].sprite.getPosition().y > WINDOW_HEIGHT + 20) {
            stars.erase(stars.begin() + i);
        } else {
            i++;
        }
    }

    // Update power-ups
    for (size_t i = 0; i < powerUps.size(); ) {
        powerUps[i].sprite.move(powerUps[i].velocity * deltaTime);
        
        if (powerUps[i].sprite.getPosition().y > WINDOW_HEIGHT + 50) {
            powerUps.erase(powerUps.begin() + i);
        } else {
            i++;
        }
    }

    // Update explosions
    for (size_t i = 0; i < explosions.size(); ) {
        explosions[i].timer += deltaTime;
        
        if (explosions[i].timer >= explosions[i].duration) {
            explosions.erase(explosions.begin() + i);
        } else {
            // Fade out explosion
            float alpha = 255 * (1 - explosions[i].timer / explosions[i].duration);
            explosions[i].sprite.setColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));
            i++;
        }
    }

    // Update particles
    for (size_t i = 0; i < particles.size(); ) {
        particles[i].lifetime += deltaTime;
        particles[i].shape.move(particles[i].velocity * deltaTime);
        particles[i].velocity.y += 200 * deltaTime; // Gravity
        particles[i].shape.setRadius(particles[i].shape.getRadius() * (1 - deltaTime * 2));
        
        if (particles[i].lifetime >= particles[i].maxLifetime || 
            particles[i].shape.getRadius() < 0.1f) {
            particles.erase(particles.begin() + i);
        } else {
            // Fade out
            sf::Color c = particles[i].shape.getFillColor();
            float alpha = c.a * (1 - particles[i].lifetime / particles[i].maxLifetime);
            particles[i].shape.setFillColor(sf::Color(c.r, c.g, c.b, static_cast<sf::Uint8>(alpha)));
            i++;
        }
    }

    // Check collisions
    checkCollisions();

    // Update UI
    updateScoreDisplay();
    updateLivesDisplay();
    updateLevelDisplay();

    // Update health bar
    float healthPercent = playerLives / 3.0f;
    healthBar.setSize(sf::Vector2f(200 * healthPercent, 20));
    
    // Change health bar color based on health
    if (healthPercent > 0.5f) {
        healthBar.setFillColor(sf::Color::Green);
    } else if (healthPercent > 0.25f) {
        healthBar.setFillColor(sf::Color::Yellow);
    } else {
        healthBar.setFillColor(sf::Color::Red);
    }

    // Check for level up
    if (enemiesKilled >= enemiesToNextLevel) {
        level++;
        enemiesKilled = 0;
        enemiesToNextLevel = 10 + level * 5;
        playerLives = std::min(3, playerLives + 1);
        
        // Clear all enemies
        enemies.clear();
        
        // Spawn bonus power-up
        PowerUp powerUp;
        powerUp.type = intDist(rng) % 3;
        powerUp.sprite.setTexture(powerUpTextures[powerUp.type]);
        powerUp.sprite.setOrigin(16, 16);
        powerUp.sprite.setPosition(WINDOW_WIDTH / 2, -32);
        powerUp.velocity = sf::Vector2f(0, 200);
        powerUps.push_back(powerUp);
        
        // Create level up particles
        for (int i = 0; i < 50; i++) {
            Particle p;
            p.shape.setRadius(3);
            p.shape.setFillColor(sf::Color(0, 200, 255));
            p.velocity = sf::Vector2f(
                (floatDist(rng) - 0.5f) * 400,
                (floatDist(rng) - 0.5f) * 400
            );
            p.shape.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
            p.lifetime = 0.0f;
            p.maxLifetime = 2.0f;
            particles.push_back(p);
        }
    }

    // Game over check
    if (playerLives <= 0) {
        gameOver = true;
        gameOverSound.play();
    }
}

void Game::spawnEnemy() {
    Enemy enemy;
    enemy.type = intDist(rng) % 3;
    enemy.health = enemy.type + 1;
    enemy.sprite.setTexture(enemyTextures[enemy.type]);
    enemy.sprite.setOrigin(24, 24);
    
    // Spawn at top or sides
    int spawnSide = intDist(rng) % 3;
    switch (spawnSide) {
        case 0: // Top
            enemy.sprite.setPosition(
                floatDist(rng) * (WINDOW_WIDTH - 40) + 20,
                -40
            );
            enemy.velocity = sf::Vector2f(
                (floatDist(rng) - 0.5f) * 100,
                100 + floatDist(rng) * 100
            );
            break;
        case 1: // Left
            enemy.sprite.setPosition(
                -40,
                floatDist(rng) * (WINDOW_HEIGHT - 40) + 20
            );
            enemy.velocity = sf::Vector2f(
                100 + floatDist(rng) * 100,
                (floatDist(rng) - 0.5f) * 100
            );
            break;
        case 2: // Right
            enemy.sprite.setPosition(
                WINDOW_WIDTH + 40,
                floatDist(rng) * (WINDOW_HEIGHT - 40) + 20
            );
            enemy.velocity = sf::Vector2f(
                -(100 + floatDist(rng) * 100),
                (floatDist(rng) - 0.5f) * 100
            );
            break;
    }
    
    // Scale based on type
    float scale = 1.0f + enemy.type * 0.3f;
    enemy.sprite.setScale(scale, scale);
    
    enemies.push_back(enemy);
}

void Game::spawnStar() {
    Star star;
    star.sprite.setTexture(starTexture);
    star.sprite.setOrigin(2, 2);
    star.sprite.setPosition(
        floatDist(rng) * WINDOW_WIDTH,
        -10
    );
    star.velocity = sf::Vector2f(0, floatDist(rng) * 100 + 50);
    
    // Random brightness
    float brightness = floatDist(rng) * 0.5f + 0.5f;
    star.sprite.setColor(sf::Color(
        static_cast<sf::Uint8>(255 * brightness),
        static_cast<sf::Uint8>(255 * brightness),
        static_cast<sf::Uint8>(255 * brightness)
    ));
    
    stars.push_back(star);
}

void Game::checkCollisions() {
    // Bullet-Enemy collisions
    for (size_t b = 0; b < bullets.size(); b++) {
        sf::FloatRect bulletBounds = bullets[b].sprite.getGlobalBounds();
        
        for (size_t e = 0; e < enemies.size(); e++) {
            sf::FloatRect enemyBounds = enemies[e].sprite.getGlobalBounds();
            
            if (bulletBounds.intersects(enemyBounds)) {
                // Hit!
                enemies[e].health--;
                bullets.erase(bullets.begin() + b);
                
                // Create hit particles
                for (int i = 0; i < 5; i++) {
                    Particle p;
                    p.shape.setRadius(2);
                    p.shape.setFillColor(sf::Color(255, 200, 50));
                    p.velocity = sf::Vector2f(
                        (floatDist(rng) - 0.5f) * 200,
                        (floatDist(rng) - 0.5f) * 200
                    );
                    p.shape.setPosition(
                        enemyBounds.left + enemyBounds.width / 2,
                        enemyBounds.top + enemyBounds.height / 2
                    );
                    p.lifetime = 0.0f;
                    p.maxLifetime = 0.5f;
                    particles.push_back(p);
                }
                
                if (enemies[e].health <= 0) {
                    // Enemy destroyed
                    playExplosion(enemies[e].sprite.getPosition());
                    
                    // Add score based on enemy type
                    score += (enemies[e].type + 1) * 100 * level;
                    enemiesKilled++;
                    
                    // Random chance to spawn power-up
                    if (intDist(rng) % 10 == 0) {
                        PowerUp powerUp;
                        powerUp.type = intDist(rng) % 3;
                        powerUp.sprite.setTexture(powerUpTextures[powerUp.type]);
                        powerUp.sprite.setOrigin(16, 16);
                        powerUp.sprite.setPosition(
                            enemies[e].sprite.getPosition().x,
                            enemies[e].sprite.getPosition().y
                        );
                        powerUp.velocity = sf::Vector2f(0, 150);
                        powerUps.push_back(powerUp);
                    }
                    
                    enemies.erase(enemies.begin() + e);
                }
                
                b--;
                break;
            }
        }
    }

    // Player-Enemy collisions
    if (!playerInvulnerable) {
        sf::FloatRect playerBounds = player.getGlobalBounds();
        
        for (size_t e = 0; e < enemies.size(); e++) {
            sf::FloatRect enemyBounds = enemies[e].sprite.getGlobalBounds();
            
            if (playerBounds.intersects(enemyBounds)) {
                // Player hit!
                playerLives--;
                playerInvulnerable = true;
                invulnerabilityTimer = 0.0f;
                player.setColor(sf::Color(255, 0, 0, 200));
                
                playExplosion(player.getPosition());
                
                // Push player back
                sf::Vector2f dir = player.getPosition() - enemies[e].sprite.getPosition();
                if (std::abs(dir.x) + std::abs(dir.y) > 0) {
                    dir = sf::Vector2f(dir.x / (std::abs(dir.x) + std::abs(dir.y)), 
                                     dir.y / (std::abs(dir.x) + std::abs(dir.y)));
                    player.setPosition(player.getPosition() + dir * 100);
                }
                
                enemies.erase(enemies.begin() + e);
                break;
            }
        }
    }

    // Player-PowerUp collisions
    sf::FloatRect playerBounds = player.getGlobalBounds();
    
    for (size_t p = 0; p < powerUps.size(); p++) {
        sf::FloatRect powerUpBounds = powerUps[p].sprite.getGlobalBounds();
        
        if (playerBounds.intersects(powerUpBounds)) {
            // Collect power-up
            powerUpSound.play();
            
            switch (powerUps[p].type) {
                case 0: // Health
                    playerLives = std::min(3, playerLives + 1);
                    break;
                case 1: // Rapid fire
                    fireRate = std::max(0.05f, fireRate - 0.05f);
                    break;
                case 2: // Shield
                    playerInvulnerable = true;
                    invulnerabilityTimer = 0.0f;
                    player.setColor(sf::Color(100, 200, 255, 200));
                    break;
            }
            
            // Create collection particles
            for (int i = 0; i < 10; i++) {
                Particle particle;
                particle.shape.setRadius(2);
                particle.shape.setFillColor(sf::Color(0, 255, 0));
                particle.velocity = sf::Vector2f(
                    (floatDist(rng) - 0.5f) * 300,
                    (floatDist(rng) - 0.5f) * 300
                );
                particle.shape.setPosition(
                    powerUpBounds.left + powerUpBounds.width / 2,
                    powerUpBounds.top + powerUpBounds.height / 2
                );
                particle.lifetime = 0.0f;
                particle.maxLifetime = 0.5f;
                particles.push_back(particle);
            }
            
            powerUps.erase(powerUps.begin() + p);
            break;
        }
    }
}

void Game::playExplosion(sf::Vector2f position) {
    explosionSound.play();
    
    Explosion explosion;
    explosion.sprite.setTexture(explosionTexture);
    explosion.sprite.setOrigin(32, 32);
    explosion.sprite.setPosition(position);
    explosion.timer = 0.0f;
    explosion.duration = 0.5f;
    explosion.sprite.setScale(0.5f, 0.5f);
    explosions.push_back(explosion);
    
    // Create explosion particles
    for (int i = 0; i < 20; i++) {
        Particle p;
        p.shape.setRadius(3);
        p.shape.setFillColor(sf::Color(
            intDist(rng) % 2 ? 255 : 200,
            intDist(rng) % 2 ? 100 : 50,
            0
        ));
        p.velocity = sf::Vector2f(
            (floatDist(rng) - 0.5f) * 400,
            (floatDist(rng) - 0.5f) * 400
        );
        p.shape.setPosition(position);
        p.lifetime = 0.0f;
        p.maxLifetime = 1.0f;
        particles.push_back(p);
    }
}

void Game::resetGame() {
    playerLives = 3;
    playerInvulnerable = false;
    invulnerabilityTimer = 0.0f;
    player.setColor(sf::Color::White);
    
    fireRate = 0.2f;
    fireTimer = 0.0f;
    
    score = 0;
    level = 1;
    enemiesKilled = 0;
    enemiesToNextLevel = 10;
    
    gameOver = false;
    gameStarted = false;
    paused = false;
    
    bullets.clear();
    enemies.clear();
    stars.clear();
    explosions.clear();
    powerUps.clear();
    particles.clear();
    
    initPlayer();
    
    enemySpawnTimer = 0.0f;
    enemySpawnInterval = 1.5f;
    starSpawnTimer = 0.0f;
    powerUpSpawnTimer = 0.0f;
    
    updateScoreDisplay();
    updateLivesDisplay();
    updateLevelDisplay();
}

void Game::updateScoreDisplay() {
    scoreText.setString("Score: " + std::to_string(score));
}

void Game::updateLivesDisplay() {
    livesText.setString("Lives: " + std::to_string(playerLives));
}

void Game::updateLevelDisplay() {
    levelText.setString("Level: " + std::to_string(level));
}

void Game::render() {
    window.clear(sf::Color(10, 10, 30));

    // Draw stars (background)
    for (const auto& star : stars) {
        window.draw(star.sprite);
    }

    // Draw particles (behind everything)
    for (const auto& particle : particles) {
        window.draw(particle.shape);
    }

    // Draw bullets
    for (const auto& bullet : bullets) {
        window.draw(bullet.sprite);
    }

    // Draw enemies
    for (const auto& enemy : enemies) {
        window.draw(enemy.sprite);
    }

    // Draw power-ups
    for (const auto& powerUp : powerUps) {
        window.draw(powerUp.sprite);
    }

    // Draw explosions
    for (const auto& explosion : explosions) {
        window.draw(explosion.sprite);
    }

    // Draw player
    window.draw(player);

    // Draw UI
    window.draw(scoreText);
    window.draw(livesText);
    window.draw(levelText);
    window.draw(healthBarBackground);
    window.draw(healthBar);

    // Draw game over screen
    if (gameOver) {
        // Darken background
        sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        overlay.setFillColor(sf::Color(0, 0, 0, 180));
        window.draw(overlay);
        
        window.draw(gameOverText);
        window.draw(restartText);
    }

    // Draw start screen
    if (!gameStarted && !gameOver) {
        // Darken background slightly
        sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        overlay.setFillColor(sf::Color(0, 0, 30, 150));
        window.draw(overlay);
        
        window.draw(titleText);
        window.draw(startText);
    }

    // Draw pause indicator
    if (paused && gameStarted && !gameOver) {
        sf::Text pauseText;
        pauseText.setFont(font);
        pauseText.setString("PAUSED");
        pauseText.setCharacterSize(48);
        pauseText.setFillColor(sf::Color::White);
        sf::FloatRect bounds = pauseText.getLocalBounds();
        pauseText.setOrigin(bounds.width / 2, bounds.height / 2);
        pauseText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);
        window.draw(pauseText);
    }

    window.display();
}
