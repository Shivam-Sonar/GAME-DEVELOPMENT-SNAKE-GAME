//snake.cpp
#include <SFML/Audio.hpp>
#include <deque>
#include <random>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <iostream>

using Vec2 = sf::Vector2i;

static const int WINDOW_W = 800;
static const int WINDOW_H = 600;
static const int CELL = 20;
static const int COLS = WINDOW_W / CELL; // 40
static const int ROWS = WINDOW_H / CELL; // 30

// Utility RNG
std::mt19937 rng(std::random_device{}());

class Snake {
public:
    enum Direction { Up, Down, Left, Right };

    Snake() { reset(); }

    void reset() {
        body.clear();
        int midx = COLS/2, midy = ROWS/2;
        body.push_back({midx, midy});
        body.push_back({midx-1, midy});
        body.push_back({midx-2, midy});
        dir = Right;
        grow_segments = 0;
        alive = true;
    }

    void setDirection(Direction d) {
        // prevent reversing into itself
        if ((dir == Left && d == Right) || (dir == Right && d == Left) ||
            (dir == Up && d == Down) || (dir == Down && d == Up)) {
            return;
        }
        dir = d;
    }

    void move() {
        if (!alive) return;
        Vec2 head = body.front();
        Vec2 next = head;
        switch (dir) {
            case Up:    next.y -= 1; break;
            case Down:  next.y += 1; break;
            case Left:  next.x -= 1; break;
            case Right: next.x += 1; break;
        }
        // Wrap-around? (we'll treat hitting walls as death)
        if (next.x < 0 || next.x >= COLS || next.y < 0 || next.y >= ROWS) {
            alive = false; return;
        }
        // self-collision
        for (auto &seg : body) {
            if (seg == next) { alive = false; return; }
        }
        body.push_front(next);
        if (grow_segments > 0) {
            --grow_segments; // keep tail (i.e., grow)
        } else {
            body.pop_back();
        }
    }

    void grow(int segs = 1) { grow_segments += segs; }

    bool occupies(const Vec2 &p) const {
        for (auto &b : body) if (b == p) return true;
        return false;
    }

    const Vec2& head() const { return body.front(); }
    std::size_t size() const { return body.size(); }
    bool isAlive() const { return alive; }

    void render(sf::RenderWindow& win, sf::RectangleShape& cellShape) {
        // head a brighter color
        cellShape.setFillColor(sf::Color(0, 200, 0));
        cellShape.setPosition(body.front().x * CELL, body.front().y * CELL);
        win.draw(cellShape);

        cellShape.setFillColor(sf::Color(0, 150, 0));
        for (size_t i = 1; i < body.size(); ++i) {
            cellShape.setPosition(body[i].x * CELL, body[i].y * CELL);
            win.draw(cellShape);
        }
    }

private:
    std::deque<Vec2> body;
    Direction dir = Right;
    int grow_segments = 0;
    bool alive = true;
};

class Food {
public:
    Food() { pos = {0,0}; }

    void spawn(const Snake& snake) {
        std::uniform_int_distribution<int> dx(0, COLS-1), dy(0, ROWS-1);
        Vec2 p;
        int attempts = 0;
        do {
            p.x = dx(rng);
            p.y = dy(rng);
            attempts++;
            // in extremely rare case where board is full, break
            if (attempts > 10000) break;
        } while (snake.occupies(p));
        pos = p;
    }

    const Vec2& position() const { return pos; }

    void render(sf::RenderWindow& win, sf::RectangleShape& shape) {
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(pos.x * CELL, pos.y * CELL);
        win.draw(shape);
    }

private:
    Vec2 pos;
};

int loadHighScore(const std::string& fname) {
    std::ifstream ifs(fname);
    if (!ifs) return 0;
    int v = 0;
    ifs >> v;
    return v;
}

void saveHighScore(const std::string& fname, int score) {
    std::ofstream ofs(fname);
    if (!ofs) return;
    ofs << score << "\n";
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_W, WINDOW_H), "Snake Game");
    window.setFramerateLimit(60);

    // Shapes
    sf::RectangleShape cellShape(sf::Vector2f((float)CELL - 1.0f, (float)CELL - 1.0f));
    cellShape.setOutlineThickness(0);

    // Font & text
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cerr << "Failed to load font arial.ttf. Place a TTF named arial.ttf in the executable folder.\n";
        // we'll continue but text won't display nicely
    }
    sf::Text scoreText("", font, 18);
    scoreText.setPosition(8, 6);
    scoreText.setFillColor(sf::Color::White);

    sf::Text pauseText("PAUSED\nPress Space to Resume", font, 36);
    pauseText.setFillColor(sf::Color::Yellow);
    pauseText.setPosition(WINDOW_W/2 - 160, WINDOW_H/2 - 40);

    sf::Text gameOverText("GAME OVER\nPress R to Restart", font, 36);
    gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setPosition(WINDOW_W/2 - 170, WINDOW_H/2 - 40);

    // Sounds
    sf::SoundBuffer eatBuf, overBuf;
    sf::Sound eatSound, overSound;
    if (eatBuf.loadFromFile("eat.wav")) {
        eatSound.setBuffer(eatBuf);
    } else {
        std::cerr << "eat.wav not found. No eat sound will play.\n";
    }
    if (overBuf.loadFromFile("gameover.wav")) {
        overSound.setBuffer(overBuf);
    } else {
        std::cerr << "gameover.wav not found. No gameover sound will play.\n";
    }

    // Background music (optional)
    sf::Music bgm;
    if (!bgm.openFromFile("bgm.ogg")) {
        // optional; no message to stdout beyond cerr above
    } else {
        bgm.setLoop(true);
        bgm.setVolume(30.f);
        bgm.play();
    }

    Snake snake;
    Food food;
    food.spawn(snake);

    int score = 0;
    const std::string HS_FILE = "highscore.txt";
    int highScore = loadHighScore(HS_FILE);

    // timing and speed
    float baseSpeed = 8.0f; // moves per second
    float speed = baseSpeed;
    float moveTimer = 0.f;

    bool paused = false;
    bool showGrid = false;

    sf::Clock clock;

    // helper lambda: increase speed every 5 points by 10%
    auto updateSpeed = [&]() {
        int increments = score / 5;
        speed = baseSpeed * std::pow(1.10f, (float)increments);
        // cap speed so it doesn't go insane
        if (speed > 45.f) speed = 45.f;
    };

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        moveTimer += dt;

        // events
        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) window.close();
            if (ev.type == sf::Event::KeyPressed) {
                if (ev.key.code == sf::Keyboard::Up) snake.setDirection(Snake::Up);
                else if (ev.key.code == sf::Keyboard::Down) snake.setDirection(Snake::Down);
                else if (ev.key.code == sf::Keyboard::Left) snake.setDirection(Snake::Left);
                else if (ev.key.code == sf::Keyboard::Right) snake.setDirection(Snake::Right);
                else if (ev.key.code == sf::Keyboard::Space) paused = !paused;
                else if (ev.key.code == sf::Keyboard::G) showGrid = !showGrid;
                else if (ev.key.code == sf::Keyboard::Add || ev.key.code == sf::Keyboard::Equal) {
                    float v = bgm.getVolume();
                    bgm.setVolume(std::min(100.f, v + 5.f));
                } else if (ev.key.code == sf::Keyboard::Hyphen) {
                    float v = bgm.getVolume();
                    bgm.setVolume(std::max(0.f, v - 5.f));
                } else if (ev.key.code == sf::Keyboard::R && !snake.isAlive()) {
                    // restart
                    snake.reset();
                    score = 0;
                    updateSpeed();
                    food.spawn(snake);
                    paused = false;
                    bgm.play();
                }
            }
        }

        // update (movement)
        if (!paused && snake.isAlive()) {
            float secsPerMove = 1.0f / speed;
            if (moveTimer >= secsPerMove) {
                moveTimer -= secsPerMove;
                snake.move();

                if (!snake.isAlive()) {
                    // game over
                    overSound.play();
                    if (score > highScore) { highScore = score; saveHighScore(HS_FILE, highScore); }
                } else {
                    // check if ate food
                    if (snake.head() == food.position()) {
                        eatSound.play();
                        snake.grow(1);
                        score += 1;
                        updateSpeed();
                        // spawn new food
                        food.spawn(snake);
                    }
                }
            }
        }

        // draw
        window.clear(sf::Color(12,12,12));

        // optional grid
        if (showGrid) {
            sf::VertexArray lines(sf::Lines);
            for (int x = 0; x <= COLS; ++x) {
                lines.append(sf::Vertex(sf::Vector2f(float(x*CELL), 0), sf::Color(40,40,40)));
                lines.append(sf::Vertex(sf::Vector2f(float(x*CELL), float(WINDOW_H)), sf::Color(40,40,40)));
            }
            for (int y = 0; y <= ROWS; ++y) {
                lines.append(sf::Vertex(sf::Vector2f(0, float(y*CELL)), sf::Color(40,40,40)));
                lines.append(sf::Vertex(sf::Vector2f(float(WINDOW_W), float(y*CELL)), sf::Color(40,40,40)));
            }
            window.draw(lines);
        }

        // render snake and food
        snake.render(window, cellShape);
        food.render(window, cellShape);

        // HUD
        std::ostringstream ss;
        ss << "Score: " << score << "  High: " << highScore << "  Speed: " << std::fixed << std::setprecision(1) << speed;
        scoreText.setString(ss.str());
        window.draw(scoreText);

        if (paused) {
            window.draw(pauseText);
        }
        if (!snake.isAlive()) {
            window.draw(gameOverText);
        }

        window.display();
    }

    // Save high score on exit
    if (score > highScore) saveHighScore(HS_FILE, score);

    return 0;
}
