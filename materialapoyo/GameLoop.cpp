#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <JuegoCatCross.h>
#include <Obstaculo.h>
#include <Coleccionable.h>
#include <Gatito.h>

// Constants
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int PLAYER_SIZE = 50;
const int OBSTACLE_SIZE = 50;
int OBSTACLE_SPEED = 0.98; // Reduce obstacle speed further

// Ajustes para lógica de cuadrícula
const int GRID_COLS = WINDOW_WIDTH / OBSTACLE_SIZE;
const int GRID_ROWS = WINDOW_HEIGHT / OBSTACLE_SIZE;

// Player class
class Player {
public:
    sf::RectangleShape shape;
    int gridX, gridY;

    Player() {
        shape.setSize(sf::Vector2f(PLAYER_SIZE, PLAYER_SIZE));
        shape.setFillColor(sf::Color::Green);
        gridX = GRID_COLS / 2;
        gridY = GRID_ROWS - 1;
        shape.setPosition(gridX * OBSTACLE_SIZE, gridY * OBSTACLE_SIZE);
    }

    void move(int dx, int dy) {
        int newGridX = gridX + dx;
        int newGridY = gridY + dy;
        if (newGridX >= 0 && newGridX < GRID_COLS && newGridY >= 0 && newGridY < GRID_ROWS) {
            gridX = newGridX;
            gridY = newGridY;
            shape.setPosition(gridX * OBSTACLE_SIZE, gridY * OBSTACLE_SIZE);
        }
    }
};

// Obstacle class
class Obstacle {
public:
    sf::RectangleShape shape;
    int gridX, gridY;
    int direction; // 1: derecha, -1: izquierda

    Obstacle(int gx, int gy, int dir) : gridX(gx), gridY(gy), direction(dir) {
        shape.setSize(sf::Vector2f(OBSTACLE_SIZE, OBSTACLE_SIZE));
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(gridX * OBSTACLE_SIZE, gridY * OBSTACLE_SIZE);
    }

    void move() {
        gridX += direction;
        if (gridX < 0) gridX = GRID_COLS - 1;
        if (gridX >= GRID_COLS) gridX = 0;
        shape.setPosition(gridX * OBSTACLE_SIZE, gridY * OBSTACLE_SIZE);
    }
};

int main() {
    srand(static_cast<unsigned>(time(0)));

    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Crossy Road");

    // Start screen
    sf::Font font;
    if (!font.loadFromFile("assets/fonts/Platinum Sign.ttf")) {
        std::cerr << "Error: No se pudo cargar la fuente." << std::endl;
        return -1;
    }

    sf::Text title("CROSSY ROAD", font, 50);
    title.setFillColor(sf::Color::White);
    title.setPosition(WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 100);

    sf::Text instructions("Presiona Enter para comenzar", font, 30);
    instructions.setFillColor(sf::Color::White);
    instructions.setPosition(WINDOW_WIDTH / 2 - 200, WINDOW_HEIGHT / 2);

    while (window.isOpen()) {
        sf::Event event;
        bool startGame = false;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
                startGame = true;
                break;
            }
        }

        if (startGame) {
            break;
        }

        window.clear();
        window.draw(title);
        window.draw(instructions);
        window.display();
    }

    Player player;
    std::vector<Obstacle> obstacles;

    // Generar obstáculos: uno por fila (excepto la fila inicial del jugador)
    for (int row = 0; row < GRID_ROWS - 1; ++row) {
        if (row == GRID_ROWS - 1) continue; // No obstáculo en la fila inicial del jugador
        int col = rand() % GRID_COLS;
        int dir = (rand() % 2 == 0) ? 1 : -1;
        obstacles.emplace_back(col, row, dir);
    }

    int level = 1; // Track the current level
    float obstacleInterval = 0.35f; // segundos entre movimientos de obstáculos (ajusta para más lento o rápido)
    static sf::Clock obstacleClock;
    int lastPlayerRow = player.gridY;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // Movimiento por evento (no mantener presionada la tecla)
        bool moved = false;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            player.move(0, -1);
            moved = true;
            sf::sleep(sf::milliseconds(120));
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            player.move(0, 1);
            moved = true;
            sf::sleep(sf::milliseconds(120));
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            player.move(-1, 0);
            moved = true;
            sf::sleep(sf::milliseconds(120));
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            player.move(1, 0);
            moved = true;
            sf::sleep(sf::milliseconds(120));
        }

        // Progresión de nivel: si el jugador avanza hacia arriba, sube el nivel y aumenta la velocidad
        if (player.gridY < lastPlayerRow) {
            level++;
            obstacleInterval = std::max(0.08f, obstacleInterval - 0.03f);
            lastPlayerRow = player.gridY;
            // Cuando el jugador llega a la fila superior, reinicia abajo y genera nuevos obstáculos
            if (player.gridY == 0) {
                player.gridY = GRID_ROWS - 1;
                player.shape.setPosition(player.gridX * OBSTACLE_SIZE, player.gridY * OBSTACLE_SIZE);
                obstacles.clear();
                for (int row = 0; row < GRID_ROWS - 1; ++row) {
                    if (row == GRID_ROWS - 1) continue;
                    int col = rand() % GRID_COLS;
                    int dir = (rand() % 2 == 0) ? 1 : -1;
                    obstacles.emplace_back(col, row, dir);
                }
                lastPlayerRow = player.gridY;
            }
        }

        // Mover obstáculos
        if (obstacleClock.getElapsedTime().asSeconds() > obstacleInterval) {
            for (auto& obstacle : obstacles) {
                obstacle.move();
            }
            obstacleClock.restart();
        }

        // Colisión
        bool collisionDetected = false;
        for (const auto& obstacle : obstacles) {
            if (player.gridX == obstacle.gridX && player.gridY == obstacle.gridY) {
                collisionDetected = true;
                break;
            }
        }
        if (collisionDetected) {
            std::cout << "Game Over!" << std::endl;
            break;
        }

        // Render
        window.clear();
        window.draw(player.shape);
        for (const auto& obstacle : obstacles) {
            window.draw(obstacle.shape);
        }
        window.display();
    }

    std::cout << "Debug: Juego terminado." << std::endl;

    return 0;
}
