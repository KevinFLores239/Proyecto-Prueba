#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <array>
#include <string>

#include "../Include/Bowser.h"

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

    sf::Font font;
    const bool fontLoaded = font.loadFromFile(
        "/System/Library/Fonts/Supplemental/Arial.ttf");
    std::array<sf::Text, 4> menuOptions;
    const std::array<const char*, 4> optionNames = {
        "Comer", "Dormir", "Despertar", "Morir"};
    for (std::size_t index = 0; index < menuOptions.size(); ++index)
    {
        menuOptions[index].setString(optionNames[index]);
        menuOptions[index].setCharacterSize(28);
        menuOptions[index].setFillColor(sf::Color::White);
        menuOptions[index].setPosition(70.0f, 150.0f + index * 65.0f);
        if (fontLoaded)
        {
            menuOptions[index].setFont(font);
        }
    }
    sf::Text lifeLabel("Vida", font, 24);
    sf::Text lifePercentage("100%", font, 20);
    sf::Text gameOverText("GAME OVER", font, 64);
    lifeLabel.setPosition(25.0f, 35.0f);
    lifePercentage.setPosition(210.0f, 70.0f);
    gameOverText.setPosition(360.0f, 70.0f);
    lifeLabel.setFillColor(sf::Color::White);
    lifePercentage.setFillColor(sf::Color::White);
    gameOverText.setFillColor(sf::Color(220, 60, 60));

    bool birthScreen = true;
    bool recovering = false;
    bool sleeping = false;
    bool frozen = false;
    bool awakening = false;
    bool dying = false;
    bool dead = false;
    float sleepKeyTime = 0.0f;
    std::size_t selectedOption = 0;
    float life = maximumLife;
    sf::Clock clock;
    sf::Clock gameOverClock;
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
                    bowser.startNormalAnimation(
                        sf::Vector2f(normalAnimationX, windowHeight / 2.0f));
                }
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
                         selectedOption < 2 && !recovering)
                {
                    recovering = true;
                    sleeping = selectedOption == 1;
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
        if (!birthScreen)
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

        window.clear(birthScreen ? sf::Color(40, 120, 70) : sf::Color(28, 35, 48));
        if (!birthScreen)
        {
            sf::RectangleShape menuPanel(sf::Vector2f(280.0f, windowHeight));
            menuPanel.setFillColor(sf::Color(20, 25, 35));
            window.draw(menuPanel);
            for (std::size_t index = 0; index < menuOptions.size(); ++index)
            {
                menuOptions[index].setFillColor(
                    index == selectedOption ? sf::Color(255, 190, 60)
                                             : sf::Color::White);
                if (fontLoaded)
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
        bowser.draw(window);
        window.display();
    }

    return 0;
}