#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <array>
#include <limits>
#include <queue>
#include <random>
#include <string>
#include <vector>

#include "../Include/Bowser.h"

struct GridPosition
{
    int column;
    int row;
};

struct PathNode
{
    GridPosition position;
    int cost;
    int priority;
};

struct PathNodeCompare
{
    bool operator()(const PathNode& first, const PathNode& second) const
    {
        return first.priority > second.priority;
    }
};

constexpr int mapColumns = 20;
constexpr int mapRows = 15;
constexpr float tileSize = 40.0f;
using BlockedGrid = std::array<std::array<bool, mapColumns>, mapRows>;

struct MapObstacle
{
    sf::Texture texture;
    sf::Sprite sprite;
    GridPosition cell;
};

struct Mushroom
{
    sf::Texture texture;
    sf::Sprite sprite;
    GridPosition cell;
    bool active = false;
    float timeRemaining = 0.0f;
};

const std::array<std::string, mapRows>& getCollisionMap()
{
    static const std::array<std::string, mapRows> collisionMap = {
        "....................",
        "....................",
        "....................",
        "....................",
        "....................",
        "....................",
        "....................",
        "....................",
        "....................",
        "....................",
        "....................",
        "....................",
        "....................",
        "....................",
        "...................."};
    return collisionMap;
}

bool isWalkable(const GridPosition& position, const BlockedGrid& blocked)
{
    if (position.column < 0 || position.column >= mapColumns ||
        position.row < 0 || position.row >= mapRows)
    {
        return false;
    }

        return getCollisionMap()[position.row][position.column] != '#' &&
            !blocked[position.row][position.column];
}

int manhattanDistance(const GridPosition& first, const GridPosition& second)
{
    return std::abs(first.column - second.column) +
           std::abs(first.row - second.row);
}

std::vector<GridPosition> findPath(const GridPosition& start,
                                   const GridPosition& goal,
                                   const BlockedGrid& blocked)
{
    if (!isWalkable(start, blocked) || !isWalkable(goal, blocked))
    {
        return {};
    }

    std::priority_queue<PathNode, std::vector<PathNode>, PathNodeCompare> open;
    std::array<std::array<int, mapColumns>, mapRows> costs;
    std::array<std::array<GridPosition, mapColumns>, mapRows> parents;
    for (int row = 0; row < mapRows; ++row)
    {
        costs[row].fill(std::numeric_limits<int>::max());
        for (int column = 0; column < mapColumns; ++column)
        {
            parents[row][column] = {-1, -1};
        }
    }

    costs[start.row][start.column] = 0;
    open.push({start, 0, manhattanDistance(start, goal)});
    const std::array<GridPosition, 4> directions = {
        GridPosition{1, 0}, GridPosition{-1, 0},
        GridPosition{0, 1}, GridPosition{0, -1}};

    while (!open.empty())
    {
        const GridPosition current = open.top().position;
        open.pop();
        if (current.column == goal.column && current.row == goal.row)
        {
            break;
        }

        for (const GridPosition& direction : directions)
        {
            const GridPosition neighbor = {
                current.column + direction.column,
                current.row + direction.row};
            if (!isWalkable(neighbor, blocked))
            {
                continue;
            }

            const int newCost = costs[current.row][current.column] + 1;
            if (newCost < costs[neighbor.row][neighbor.column])
            {
                costs[neighbor.row][neighbor.column] = newCost;
                parents[neighbor.row][neighbor.column] = current;
                const int priority = newCost + manhattanDistance(neighbor, goal);
                open.push({neighbor, newCost, priority});
            }
        }
    }

    if (costs[goal.row][goal.column] == std::numeric_limits<int>::max())
    {
        return {};
    }

    std::vector<GridPosition> path;
    GridPosition current = goal;
    while (!(current.column == start.column && current.row == start.row))
    {
        path.push_back(current);
        current = parents[current.row][current.column];
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}

sf::Vector2f gridToWorld(const GridPosition& position)
{
    return sf::Vector2f(
        position.column * tileSize + tileSize / 2.0f,
        position.row * tileSize + tileSize / 2.0f);
}

GridPosition worldToGrid(const sf::Vector2f& position)
{
    return {
        std::clamp(static_cast<int>(std::lround(
                        (position.x - tileSize / 2.0f) / tileSize)),
                   0, mapColumns - 1),
        std::clamp(static_cast<int>(std::lround(
                        (position.y - tileSize / 2.0f) / tileSize)),
                   0, mapRows - 1)};
}

int main()
{
    const unsigned int windowWidth = 800;
    const unsigned int windowHeight = 600;
    const float normalAnimationX = 600.0f;
    const float maximumLife = 100.0f;
    const float lifeLossPerSecond = 4.0f;
    const float lifeRecoveryPerSecond = 18.0f;
    const float sleepRecoveryPerSecond = 8.0f;
    const float freezeDelay = 3.0f;

    sf::RenderWindow window(
        sf::VideoMode(windowWidth, windowHeight), "Juego de Bowser");
    window.setFramerateLimit(60);

    Bowser bowser(sf::Vector2f(windowWidth / 2.0f, windowHeight / 2.0f));
    if (!bowser.isLoaded())
    {
        std::cerr << "No se pudieron cargar los sprites de nacimiento de Bowser.\n";
        return 1;
    }
    if (!bowser.normalIsLoaded() || !bowser.eatingIsLoaded() ||
        !bowser.sleepingIsLoaded() || !bowser.awakeningIsLoaded() ||
        !bowser.deathIsLoaded())
    {
        std::cerr << "No se pudieron cargar las animaciones de Bowser.\n";
        return 1;
    }

    sf::Texture mapTexture;
    if (!mapTexture.loadFromFile("Images/Maps/Mapa.jpeg"))
    {
        std::cerr << "No se pudo cargar Images/Maps/Mapa.jpeg.\n";
        return 1;
    }
    sf::Sprite mapBackground(mapTexture);
    const sf::Vector2u mapImageSize = mapTexture.getSize();
    mapBackground.setScale(
        static_cast<float>(windowWidth) / mapImageSize.x,
        static_cast<float>(windowHeight) / mapImageSize.y);

    sf::Texture menuBackgroundTexture;
    if (!menuBackgroundTexture.loadFromFile("Images/FondoMenu.png"))
    {
        std::cerr << "No se pudo cargar Images/FondoMenu.png.\n";
        return 1;
    }
    sf::Sprite menuBackground(menuBackgroundTexture);
    const sf::Vector2u menuImageSize = menuBackgroundTexture.getSize();
    menuBackground.setScale(
        static_cast<float>(windowWidth) / menuImageSize.x,
        static_cast<float>(windowHeight) / menuImageSize.y);

    sf::Music mapMusic;
    const bool mapMusicLoaded = mapMusic.openFromFile("Docs/Musica/Musica.mp3");
    if (mapMusicLoaded)
    {
        mapMusic.setLoop(true);
        mapMusic.setVolume(70.0f);
    }
    else
    {
        std::cerr << "No se pudo cargar Docs/Musica/Musica.mp3.\n";
    }

    constexpr std::size_t obstacleCount = 20;
    std::array<MapObstacle, obstacleCount> mapObstacles;
    const std::array<const char*, 3> obstaclePaths = {
        "Images/Obstaculos/Obstaculo 1.png",
        "Images/Obstaculos/Obstaculo 2.png",
        "Images/Obstaculos/Obstaculo 3.png"};
    for (std::size_t index = 0; index < mapObstacles.size(); ++index)
    {
        if (!mapObstacles[index].texture.loadFromFile(
            obstaclePaths[index % obstaclePaths.size()]))
        {
            std::cerr << "No se pudo cargar un obstaculo del mapa.\n";
            return 1;
        }
        mapObstacles[index].sprite.setTexture(mapObstacles[index].texture);
        const sf::Vector2u obstacleSize = mapObstacles[index].texture.getSize();
        const float obstacleScale = std::min(
            (tileSize - 4.0f) / obstacleSize.x,
            (tileSize - 4.0f) / obstacleSize.y);
        mapObstacles[index].sprite.setScale(obstacleScale, obstacleScale);
        mapObstacles[index].sprite.setOrigin(
            obstacleSize.x / 2.0f, obstacleSize.y / 2.0f);
    }

    BlockedGrid blocked{};
    std::mt19937 randomGenerator(std::random_device{}());
    std::uniform_int_distribution<int> randomColumn(2, mapColumns - 3);
    std::uniform_int_distribution<int> randomRow(2, mapRows - 4);
    const GridPosition bowserStart = {8, 12};
    const auto placeObstacles = [&]()
    {
        for (auto& blockedRow : blocked)
        {
            blockedRow.fill(false);
        }

        for (MapObstacle& obstacle : mapObstacles)
        {
        GridPosition cell;
        do
        {
            cell = {randomColumn(randomGenerator), randomRow(randomGenerator)};
        } while (!isWalkable(cell, blocked) ||
                 (cell.column == bowserStart.column &&
                  cell.row == bowserStart.row));

        obstacle.cell = cell;
        blocked[cell.row][cell.column] = true;
        obstacle.sprite.setPosition(gridToWorld(cell));
        }
    };
    placeObstacles();

    Mushroom mushroom;
    if (!mushroom.texture.loadFromFile("Images/Comida/Hongo.png"))
    {
        std::cerr << "No se pudo cargar Images/Comida/Hongo.png.\n";
        return 1;
    }
    mushroom.sprite.setTexture(mushroom.texture);
    const sf::Vector2u mushroomSize = mushroom.texture.getSize();
    const float mushroomScale = std::min(
        (tileSize - 4.0f) / mushroomSize.x,
        (tileSize - 4.0f) / mushroomSize.y);
    mushroom.sprite.setScale(mushroomScale, mushroomScale);
    mushroom.sprite.setOrigin(mushroomSize.x / 2.0f,
                              mushroomSize.y / 2.0f);

    const auto placeMushroom = [&]()
    {
        GridPosition cell;
        do
        {
            cell = {randomColumn(randomGenerator), randomRow(randomGenerator)};
        } while (!isWalkable(cell, blocked) ||
                 (cell.column == bowserStart.column &&
                  cell.row == bowserStart.row) ||
                 (mushroom.active && cell.column == mushroom.cell.column &&
                  cell.row == mushroom.cell.row));

        mushroom.cell = cell;
        mushroom.active = true;
        mushroom.timeRemaining = 8.0f;
        mushroom.sprite.setPosition(gridToWorld(cell));
    };
    placeMushroom();

    sf::Font font;
    const bool fontLoaded = font.loadFromFile(
        "/System/Library/Fonts/Supplemental/Arial.ttf");
    sf::Font bowserFont;
    sf::Font menuFont;
    sf::Font gameOverFont;
    const bool bowserFontLoaded = bowserFont.loadFromFile(
        "Asset/fonts/SuperMario256.ttf");
    const bool menuFontLoaded = menuFont.loadFromFile(
        "Asset/fonts/Crunchy Time.ttf");
    const bool gameOverFontLoaded = gameOverFont.loadFromFile(
        "Asset/fonts/Crushed.ttf");
    std::array<sf::Text, 5> menuOptions;
    const std::array<const char*, 5> optionNames = {
        "Comenzar juego (Enter)", "Comer", "Dormir", "Despertar", "Morir"};
    for (std::size_t index = 0; index < menuOptions.size(); ++index)
    {
        menuOptions[index].setString(optionNames[index]);
        menuOptions[index].setCharacterSize(28);
        menuOptions[index].setFillColor(sf::Color::White);
        menuOptions[index].setPosition(50.0f, 130.0f + index * 55.0f);
        if (menuFontLoaded || fontLoaded)
        {
            menuOptions[index].setFont(menuFontLoaded ? menuFont : font);
        }
    }
    sf::Text lifeLabel(
        "Vida", menuFontLoaded ? menuFont : font, 24);
    sf::Text lifePercentage("100%", font, 20);
    sf::Text gameOverText(
        "GAME OVER", gameOverFontLoaded ? gameOverFont : font, 64);
    lifeLabel.setPosition(25.0f, 35.0f);
    lifePercentage.setPosition(210.0f, 70.0f);
    const sf::FloatRect menuGameOverBounds = gameOverText.getLocalBounds();
    gameOverText.setPosition(
        540.0f - menuGameOverBounds.width / 2.0f,
        70.0f - menuGameOverBounds.top);
    lifeLabel.setFillColor(sf::Color::White);
    lifePercentage.setFillColor(sf::Color::White);
    gameOverText.setFillColor(sf::Color(220, 60, 60));

    bool birthScreen = true;
    bool mapScreen = false;
    bool mapGameOver = false;
    bool recovering = false;
    bool sleeping = false;
    bool frozen = false;
    bool awakening = false;
    bool dying = false;
    bool dead = false;
    float sleepKeyTime = 0.0f;
    std::size_t selectedOption = 0;
    float life = maximumLife;
    std::vector<GridPosition> path;
    std::size_t pathIndex = 0;
    constexpr float movementSpeed = 125.0f;
    constexpr float routePreviewDuration = 0.8f;
    float routePreviewTime = 0.0f;
    sf::Clock clock;
    sf::Clock gameOverClock;
    const auto returnToMainMenu = [&]()
    {
        mapScreen = false;
        mapGameOver = false;
        life = maximumLife;
        selectedOption = 0;
        recovering = false;
        sleeping = false;
        frozen = false;
        awakening = false;
        dying = false;
        dead = false;
        if (mapMusicLoaded && mapMusic.getStatus() != sf::Music::Playing)
        {
            mapMusic.play();
        }
        mushroom.active = false;
        path.clear();
        pathIndex = 0;
        routePreviewTime = 0.0f;
        bowser.setNormalAnimationScale(0.45f);
        bowser.startNormalAnimation(
            sf::Vector2f(normalAnimationX, windowHeight / 2.0f));
    };
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed && birthScreen &&
                event.key.code == sf::Keyboard::Enter)
            {
                bowser.advanceBirthFrame();
                if (bowser.birthIsComplete())
                {
                    birthScreen = false;
                    if (mapMusicLoaded &&
                        mapMusic.getStatus() != sf::Music::Playing)
                    {
                        mapMusic.play();
                    }
                    bowser.startNormalAnimation(
                        sf::Vector2f(normalAnimationX, windowHeight / 2.0f));
                }
            }
            else if (event.type == sf::Event::KeyPressed && !birthScreen && mapScreen)
            {
                if (event.key.code == sf::Keyboard::Escape ||
                    (mapGameOver && event.key.code == sf::Keyboard::Enter))
                {
                    returnToMainMenu();
                }
            }
            else if (event.type == sf::Event::MouseButtonPressed &&
                     !birthScreen && mapScreen &&
                     event.mouseButton.button == sf::Mouse::Left)
            {
                const GridPosition goal = worldToGrid(sf::Vector2f(
                    static_cast<float>(event.mouseButton.x),
                    static_cast<float>(event.mouseButton.y)));
                const GridPosition currentCell =
                    worldToGrid(bowser.getPosition());
                path = findPath(currentCell, goal, blocked);
                pathIndex = path.size() > 1 ? 1 : path.size();
                routePreviewTime = path.empty() ? 0.0f : routePreviewDuration;
            }
            else if (event.type == sf::Event::KeyPressed && !birthScreen)
            {
                if (!dead && !dying && frozen && event.key.code == sf::Keyboard::Num3)
                {
                    frozen = false;
                    awakening = true;
                    bowser.startAwakeningAnimation();
                }
                else if (!dead && !dying && !frozen && event.key.code == sf::Keyboard::Up)
                {
                    selectedOption = (selectedOption + menuOptions.size() - 1) %
                                     menuOptions.size();
                }
                else if (!dead && !dying && !frozen && event.key.code == sf::Keyboard::Down)
                {
                    selectedOption = (selectedOption + 1) % menuOptions.size();
                }
                else if (!dead && !dying && !frozen && event.key.code == sf::Keyboard::Num1 &&
                         !recovering && !awakening)
                {
                    recovering = true;
                    sleeping = false;
                    bowser.startEatingAnimation();
                }
                else if (!dead && !dying && !frozen && event.key.code == sf::Keyboard::Num2 &&
                         !recovering && !awakening)
                {
                    recovering = true;
                    sleeping = true;
                    sleepKeyTime = 0.0f;
                    bowser.startSleepingAnimation();
                }
                else if (!dead && !dying && !frozen && event.key.code == sf::Keyboard::Enter &&
                         selectedOption == 0 && !recovering)
                {
                    mapScreen = true;
                    mapGameOver = false;
                    life = maximumLife;
                    placeObstacles();
                    placeMushroom();
                    path.clear();
                    routePreviewTime = 0.0f;
                    bowser.setPosition(gridToWorld(bowserStart));
                    bowser.setNormalAnimationScale(0.13f);
                    bowser.startNormalAnimation(gridToWorld(bowserStart));
                }
                else if (!dead && !dying && !frozen && event.key.code == sf::Keyboard::Enter &&
                         selectedOption >= 1 && selectedOption <= 2 && !recovering)
                {
                    recovering = true;
                    sleeping = selectedOption == 2;
                    sleepKeyTime = 0.0f;
                    if (sleeping)
                    {
                        bowser.startSleepingAnimation();
                    }
                    else
                    {
                        bowser.startEatingAnimation();
                    }
                }
            }
        }

        const float deltaTime = clock.restart().asSeconds();
        if (mapScreen && !mapGameOver)
        {
            bowser.updateNormalAnimation(deltaTime);
            life = std::max(0.0f, life - lifeLossPerSecond * deltaTime);

            if (mushroom.active)
            {
                mushroom.timeRemaining -= deltaTime;
                if (mushroom.timeRemaining <= 0.0f)
                {
                    placeMushroom();
                }
            }

            const sf::Vector2f mushroomPosition = gridToWorld(mushroom.cell);
            const sf::Vector2f bowserPosition = bowser.getPosition();
            const sf::Vector2f mushroomDifference = bowserPosition -
                                                     mushroomPosition;
            const float mushroomDistanceSquared =
                mushroomDifference.x * mushroomDifference.x +
                mushroomDifference.y * mushroomDifference.y;
            if (mushroom.active && mushroomDistanceSquared <= 1.0f)
            {
                life = maximumLife;
                placeMushroom();
            }

            if (life <= 0.0f)
            {
                mapGameOver = true;
                path.clear();
                routePreviewTime = 0.0f;
            }

            if (routePreviewTime > 0.0f)
            {
                routePreviewTime = std::max(0.0f,
                                            routePreviewTime - deltaTime);
            }
            else if (pathIndex < path.size())
            {
                const sf::Vector2f currentPosition = bowser.getPosition();
                const sf::Vector2f targetPosition = gridToWorld(path[pathIndex]);
                const sf::Vector2f difference = targetPosition - currentPosition;
                const float distance = std::sqrt(
                    difference.x * difference.x + difference.y * difference.y);
                const float step = movementSpeed * deltaTime;
                if (distance <= step)
                {
                    if (std::abs(difference.x) > 0.01f)
                    {
                        bowser.faceLeft(difference.x < 0.0f);
                    }
                    bowser.setPosition(targetPosition);
                    ++pathIndex;
                }
                else if (distance > 0.0f)
                {
                    if (std::abs(difference.x) > 0.01f)
                    {
                        bowser.faceLeft(difference.x < 0.0f);
                    }
                    bowser.setPosition(currentPosition +
                                       difference * (step / distance));
                }
            }
            else if (!path.empty())
            {
                path.clear();
            }
        }
        else if (!birthScreen)
        {
            if (dying)
            {
                if (bowser.updateDeathAnimation(deltaTime))
                {
                    dying = false;
                    dead = true;
                }
            }
            else if (awakening)
            {
                if (bowser.updateAwakeningAnimation(deltaTime))
                {
                    awakening = false;
                    life = maximumLife * 0.20f;
                    bowser.startNormalAnimation(
                        sf::Vector2f(normalAnimationX, windowHeight / 2.0f));
                }
            }
            else if (frozen)
            {
                life = std::min(maximumLife,
                                life + sleepRecoveryPerSecond * deltaTime);
            }
            else if (recovering)
            {
                life = std::min(maximumLife,
                                life + (sleeping ? sleepRecoveryPerSecond
                                                 : lifeRecoveryPerSecond) * deltaTime);
                if (sleeping)
                {
                    bowser.updateSleepingAnimation(deltaTime);
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2))
                    {
                        sleepKeyTime += deltaTime;
                        if (sleepKeyTime >= freezeDelay)
                        {
                            frozen = true;
                            bowser.freezeSleepingAnimation();
                        }
                    }
                    else
                    {
                        sleepKeyTime = 0.0f;
                    }
                }
                else
                {
                    bowser.updateEatingAnimation(deltaTime);
                }
                if (life >= maximumLife && !frozen)
                {
                    recovering = false;
                    sleeping = false;
                    bowser.startNormalAnimation(
                        sf::Vector2f(normalAnimationX, windowHeight / 2.0f));
                }
            }
            else if (!dead)
            {
                life = std::max(0.0f, life - lifeLossPerSecond * deltaTime);
                if (life <= 0.0f)
                {
                    dying = true;
                    recovering = false;
                    sleeping = false;
                    frozen = false;
                    bowser.startDeathAnimation();
                    gameOverClock.restart();
                }
                else
                {
                    bowser.updateNormalAnimation(deltaTime);
                }
            }
        }

        window.clear(sf::Color::Black);
        if (mapScreen)
        {
            window.draw(mapBackground);

            if (!path.empty())
            {
                sf::RectangleShape routeTile(
                    sf::Vector2f(tileSize - 6.0f, tileSize - 6.0f));
                routeTile.setFillColor(sf::Color(220, 30, 30, 90));
                routeTile.setOutlineColor(sf::Color::Red);
                routeTile.setOutlineThickness(3.0f);
                 const std::size_t firstPendingStep = routePreviewTime > 0.0f
                                                  ? 0
                                                  : pathIndex;
                 for (std::size_t index = firstPendingStep;
                     index < path.size(); ++index)
                {
                    const sf::Vector2f center = gridToWorld(path[index]);
                    routeTile.setPosition(
                        center.x - routeTile.getSize().x / 2.0f,
                        center.y - routeTile.getSize().y / 2.0f);
                    window.draw(routeTile);
                }
            }

            for (const MapObstacle& obstacle : mapObstacles)
            {
                window.draw(obstacle.sprite);
            }

            if (mushroom.active)
            {
                window.draw(mushroom.sprite);
            }

            sf::Text mapTitle(
                "Mapa de juego", menuFontLoaded ? menuFont : font, 28);
            mapTitle.setPosition(20.0f, 15.0f);
            mapTitle.setFillColor(sf::Color(255, 245, 210));
            mapTitle.setOutlineThickness(2.0f);
            mapTitle.setOutlineColor(sf::Color(60, 25, 15));
            if (fontLoaded)
            {
                window.draw(mapTitle);
            }

            sf::Text mapLifeLabel(
                "Vida", menuFontLoaded ? menuFont : font, 18);
            mapLifeLabel.setPosition(20.0f, 48.0f);
            mapLifeLabel.setFillColor(sf::Color(255, 245, 210));
            mapLifeLabel.setOutlineThickness(2.0f);
            mapLifeLabel.setOutlineColor(sf::Color(60, 25, 15));
            window.draw(mapLifeLabel);

            sf::RectangleShape mapLifeBackground(sf::Vector2f(220.0f, 18.0f));
            mapLifeBackground.setPosition(75.0f, 50.0f);
            mapLifeBackground.setFillColor(sf::Color(55, 25, 25, 220));
            window.draw(mapLifeBackground);

            sf::RectangleShape mapLifeBar(
                sf::Vector2f(220.0f * life / maximumLife, 18.0f));
            mapLifeBar.setPosition(75.0f, 50.0f);
            mapLifeBar.setFillColor(life > 30.0f
                                        ? sf::Color(70, 220, 90)
                                        : sf::Color(220, 55, 45));
            window.draw(mapLifeBar);

            sf::Text mapLifePercentage(
                std::to_string(static_cast<int>(std::round(life))) + "%",
                font, 16);
            mapLifePercentage.setPosition(165.0f, 48.0f);
            mapLifePercentage.setFillColor(sf::Color::White);
            mapLifePercentage.setOutlineThickness(1.0f);
            mapLifePercentage.setOutlineColor(sf::Color::Black);
            if (fontLoaded)
            {
                window.draw(mapLifePercentage);
            }

            if (mapGameOver && (fontLoaded || gameOverFontLoaded))
            {
                sf::RectangleShape gameOverPanel(
                    sf::Vector2f(620.0f, 190.0f));
                gameOverPanel.setPosition(90.0f, 190.0f);
                gameOverPanel.setFillColor(sf::Color(20, 10, 10, 225));
                gameOverPanel.setOutlineThickness(3.0f);
                gameOverPanel.setOutlineColor(sf::Color(220, 55, 45));
                window.draw(gameOverPanel);

                sf::Text mapGameOverText("GAME OVER", font, 64);
                if (gameOverFontLoaded)
                {
                    mapGameOverText.setFont(gameOverFont);
                }
                const sf::FloatRect gameOverBounds =
                    mapGameOverText.getLocalBounds();
                mapGameOverText.setPosition(
                    400.0f - gameOverBounds.width / 2.0f,
                    210.0f - gameOverBounds.top);
                mapGameOverText.setFillColor(sf::Color(240, 65, 55));
                window.draw(mapGameOverText);

                sf::Text returnToMenuText(
                    "Presiona ENTER para regresar al menu principal",
                    menuFontLoaded ? menuFont : font, 22);
                const sf::FloatRect returnBounds = returnToMenuText.getLocalBounds();
                returnToMenuText.setPosition(
                    400.0f - returnBounds.width / 2.0f,
                    315.0f - returnBounds.top);
                returnToMenuText.setFillColor(sf::Color::White);
                window.draw(returnToMenuText);
            }

            sf::Text mapInstructions("ESC: volver al menu", font, 18);
            mapInstructions.setPosition(20.0f, 570.0f);
            mapInstructions.setFillColor(sf::Color(255, 245, 210));
            mapInstructions.setOutlineThickness(2.0f);
            mapInstructions.setOutlineColor(sf::Color(60, 25, 15));
            if (fontLoaded)
            {
                window.draw(mapInstructions);
            }
        }
        else if (!birthScreen)
        {
            window.draw(menuBackground);
            for (std::size_t index = 0; index < menuOptions.size(); ++index)
            {
                sf::RectangleShape optionPanel(sf::Vector2f(330.0f, 48.0f));
                optionPanel.setPosition(25.0f, 120.0f + index * 55.0f);
                optionPanel.setFillColor(sf::Color(0, 0, 0, 220));
                optionPanel.setOutlineThickness(index == selectedOption ? 2.0f : 1.0f);
                optionPanel.setOutlineColor(index == selectedOption
                                                ? sf::Color(255, 190, 60)
                                                : sf::Color(90, 90, 90));
                window.draw(optionPanel);

                menuOptions[index].setFillColor(sf::Color::White);
                if (menuFontLoaded || fontLoaded)
                {
                    window.draw(menuOptions[index]);
                }
            }

            sf::RectangleShape lifeBackground(sf::Vector2f(230.0f, 24.0f));
            lifeBackground.setPosition(25.0f, 70.0f);
            lifeBackground.setFillColor(sf::Color(80, 80, 90));
            window.draw(lifeBackground);

            sf::RectangleShape lifeBar(
                sf::Vector2f(230.0f * life / maximumLife, 24.0f));
            lifeBar.setPosition(25.0f, 70.0f);
            lifeBar.setFillColor(recovering ? sf::Color(80, 220, 110)
                                        : sf::Color(220, 70, 70));
            window.draw(lifeBar);

            lifePercentage.setString(
                std::to_string(static_cast<int>(std::round(life))) + "%");
            if (fontLoaded)
            {
                window.draw(lifeLabel);
                window.draw(lifePercentage);
            }

            if ((dying || dead) && fontLoaded)
            {
                window.draw(gameOverText);
            }
        }
        if (dying || dead)
        {
            if (gameOverClock.getElapsedTime().asSeconds() >= 3.0f)
            {
                window.close();
            }
        }
        if ((birthScreen || !mapScreen) && (fontLoaded || bowserFontLoaded))
        {
            sf::Text bowserTitle(
                "Super Bowser", bowserFontLoaded ? bowserFont : font, 36);
            const sf::FloatRect bowserTitleBounds = bowserTitle.getLocalBounds();
            bowserTitle.setPosition(
                540.0f - bowserTitleBounds.width / 2.0f,
                95.0f - bowserTitleBounds.top);
            if (birthScreen)
            {
                bowserTitle.setPosition(
                    400.0f - bowserTitleBounds.width / 2.0f,
                    35.0f - bowserTitleBounds.top);
            }
            bowserTitle.setFillColor(sf::Color(255, 190, 60));
            window.draw(bowserTitle);
        }
        if (!mapScreen || !mapGameOver)
        {
            bowser.draw(window);
        }
        window.display();
    }

    return 0;
}