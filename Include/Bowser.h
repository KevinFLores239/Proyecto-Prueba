#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <queue>
#include <vector>

class Bowser
{
public:
    explicit Bowser(const sf::Vector2f& position)
    {
        for (std::size_t index = 0; index < birthTextures_.size(); ++index)
        {
            loaded_ = loadTextureWithoutWhiteBackground(
                          birthTextures_[index], birthImagePaths_[index]) &&
                      loaded_;
        }

        if (loaded_)
        {
            setBirthFrame(0, position);
        }

        sf::Image normalSheet;
        normalLoaded_ = normalSheet.loadFromFile(
            "Images/Estado Normal/Estado Normal.png");
        if (normalLoaded_)
        {
            makeWhiteBackgroundTransparent(normalSheet);
            unsigned int sourceX = 0;
            for (std::size_t index = 0; index < normalFrames_.size(); ++index)
            {
                sf::Image frame;
                frame.create(normalFrameWidths_[index], frameHeight_, sf::Color::Transparent);
                frame.copy(normalSheet, 0, 0,
                           sf::IntRect(sourceX, 0, normalFrameWidths_[index], frameHeight_), true);
                normalLoaded_ = normalFrames_[index].loadFromImage(frame) && normalLoaded_;
                sourceX += normalFrameWidths_[index];
            }
        }

        sf::Image eatingSheet;
        eatingLoaded_ = eatingSheet.loadFromFile(
            "Images/Comiendo/Comiendo.png");
        if (eatingLoaded_)
        {
            makeWhiteBackgroundTransparent(eatingSheet);
            unsigned int sourceX = 0;
            for (std::size_t index = 0; index < eatingFrames_.size(); ++index)
            {
                sf::Image frame;
                frame.create(eatingFrameWidths_[index], eatingFrameHeight_,
                             sf::Color::Transparent);
                frame.copy(eatingSheet, 0, 0,
                           sf::IntRect(sourceX, 0, eatingFrameWidths_[index],
                                       eatingFrameHeight_), true);
                eatingLoaded_ = eatingFrames_[index].loadFromImage(frame) &&
                                eatingLoaded_;
                sourceX += eatingFrameWidths_[index];
            }
        }

        sf::Image sleepingSheet;
        sleepingLoaded_ = sleepingSheet.loadFromFile(
            "Images/Dormir/Dormido.png");
        if (sleepingLoaded_)
        {
            makeWhiteBackgroundTransparent(sleepingSheet);
            unsigned int sourceX = 0;
            for (std::size_t index = 0; index < sleepingFrames_.size(); ++index)
            {
                sf::Image frame;
                frame.create(sleepingFrameWidths_[index], sleepingFrameHeight_,
                             sf::Color::Transparent);
                frame.copy(sleepingSheet, 0, 0,
                           sf::IntRect(sourceX, 0, sleepingFrameWidths_[index],
                                       sleepingFrameHeight_), true);
                sleepingLoaded_ = sleepingFrames_[index].loadFromImage(frame) &&
                                  sleepingLoaded_;
                sourceX += sleepingFrameWidths_[index];
            }
        }

        sf::Image awakeningSheet;
        awakeningLoaded_ = awakeningSheet.loadFromFile(
            "Images/Despertar/Despertar.png");
        if (awakeningLoaded_)
        {
            makeWhiteBackgroundTransparent(awakeningSheet);
            unsigned int sourceX = 0;
            for (std::size_t index = 0; index < awakeningFrames_.size(); ++index)
            {
                sf::Image frame;
                frame.create(awakeningFrameWidths_[index], awakeningFrameHeight_,
                             sf::Color::Transparent);
                frame.copy(awakeningSheet, 0, 0,
                           sf::IntRect(sourceX, 0, awakeningFrameWidths_[index],
                                       awakeningFrameHeight_), true);
                awakeningLoaded_ = awakeningFrames_[index].loadFromImage(frame) &&
                                   awakeningLoaded_;
                sourceX += awakeningFrameWidths_[index];
            }
        }

        sf::Image deathSheet;
        deathLoaded_ = deathSheet.loadFromFile("Images/Morir/Muerte.png");
        if (deathLoaded_)
        {
            makeWhiteBackgroundTransparent(deathSheet);
            unsigned int sourceX = 0;
            for (std::size_t index = 0; index < deathFrames_.size(); ++index)
            {
                sf::Image frame;
                frame.create(deathFrameWidths_[index], deathFrameHeight_,
                             sf::Color::Transparent);
                frame.copy(deathSheet, 0, 0,
                           sf::IntRect(sourceX, 0, deathFrameWidths_[index],
                                       deathFrameHeight_), true);
                deathLoaded_ = deathFrames_[index].loadFromImage(frame) &&
                               deathLoaded_;
                sourceX += deathFrameWidths_[index];
            }
        }
    }

    bool isLoaded() const
    {
        return loaded_;
    }

    void advanceBirthFrame()
    {
        if (!loaded_)
        {
            return;
        }

        if (currentBirthFrame_ < birthTextures_.size() - 1)
        {
            ++currentBirthFrame_;
            setBirthFrame(currentBirthFrame_, sprite_.getPosition());
        }
    }

    bool birthIsComplete() const
    {
        return currentBirthFrame_ == birthTextures_.size() - 1;
    }

    bool normalIsLoaded() const
    {
        return normalLoaded_;
    }

    bool eatingIsLoaded() const
    {
        return eatingLoaded_;
    }

    bool sleepingIsLoaded() const
    {
        return sleepingLoaded_;
    }

    bool awakeningIsLoaded() const
    {
        return awakeningLoaded_;
    }

    bool deathIsLoaded() const
    {
        return deathLoaded_;
    }

    void startNormalAnimation(const sf::Vector2f& position)
    {
        if (!normalLoaded_)
        {
            return;
        }

        currentNormalFrame_ = 0;
        normalAnimationTime_ = 0.0f;
        setNormalFrame(0, position);
    }

    void setNormalAnimationScale(float scale)
    {
        normalScale_ = scale;
        if (normalLoaded_)
        {
            setNormalFrame(currentNormalFrame_, sprite_.getPosition());
        }
    }

    void updateNormalAnimation(float deltaTime)
    {
        if (!normalLoaded_)
        {
            return;
        }

        normalAnimationTime_ += deltaTime;
        if (normalAnimationTime_ < normalFrameTime_)
        {
            return;
        }

        normalAnimationTime_ = 0.0f;
        currentNormalFrame_ = (currentNormalFrame_ + 1) % normalFrameCount_;
        setNormalFrame(currentNormalFrame_, sprite_.getPosition());
    }

    void startEatingAnimation()
    {
        if (eatingLoaded_)
        {
            currentEatingFrame_ = 0;
            eatingAnimationTime_ = 0.0f;
            setEatingFrame(0, sprite_.getPosition());
        }
    }

    void updateEatingAnimation(float deltaTime)
    {
        if (!eatingLoaded_)
        {
            return;
        }

        eatingAnimationTime_ += deltaTime;
        if (eatingAnimationTime_ >= eatingFrameTime_)
        {
            eatingAnimationTime_ = 0.0f;
            currentEatingFrame_ = (currentEatingFrame_ + 1) % eatingFrameCount_;
            setEatingFrame(currentEatingFrame_, sprite_.getPosition());
        }
    }

    void startSleepingAnimation()
    {
        if (sleepingLoaded_)
        {
            currentSleepingFrame_ = 0;
            sleepingAnimationTime_ = 0.0f;
            setSleepingFrame(0, sprite_.getPosition());
        }
    }

    void updateSleepingAnimation(float deltaTime)
    {
        if (!sleepingLoaded_)
        {
            return;
        }

        sleepingAnimationTime_ += deltaTime;
        if (sleepingAnimationTime_ >= sleepingFrameTime_)
        {
            sleepingAnimationTime_ = 0.0f;
            currentSleepingFrame_ =
                (currentSleepingFrame_ + 1) % sleepingFrameCount_;
            setSleepingFrame(currentSleepingFrame_, sprite_.getPosition());
        }
    }

    void freezeSleepingAnimation()
    {
        if (sleepingLoaded_)
        {
            currentSleepingFrame_ = sleepingFrameCount_ - 1;
            setSleepingFrame(currentSleepingFrame_, sprite_.getPosition());
        }
    }

    void startAwakeningAnimation()
    {
        if (awakeningLoaded_)
        {
            currentAwakeningFrame_ = 0;
            awakeningAnimationTime_ = 0.0f;
            setAwakeningFrame(0, sprite_.getPosition());
        }
    }

    bool updateAwakeningAnimation(float deltaTime)
    {
        if (!awakeningLoaded_)
        {
            return true;
        }

        awakeningAnimationTime_ += deltaTime;
        if (awakeningAnimationTime_ < awakeningFrameTime_)
        {
            return false;
        }

        awakeningAnimationTime_ = 0.0f;
        if (currentAwakeningFrame_ < awakeningFrameCount_ - 1)
        {
            ++currentAwakeningFrame_;
            setAwakeningFrame(currentAwakeningFrame_, sprite_.getPosition());
            return false;
        }

        return true;
    }

    void startDeathAnimation()
    {
        if (deathLoaded_)
        {
            currentDeathFrame_ = 0;
            deathAnimationTime_ = 0.0f;
            setDeathFrame(0, sprite_.getPosition());
        }
    }

    bool updateDeathAnimation(float deltaTime)
    {
        if (!deathLoaded_)
        {
            return true;
        }

        deathAnimationTime_ += deltaTime;
        if (deathAnimationTime_ < deathFrameTime_)
        {
            return false;
        }

        deathAnimationTime_ = 0.0f;
        if (currentDeathFrame_ < deathFrameCount_ - 1)
        {
            ++currentDeathFrame_;
            setDeathFrame(currentDeathFrame_, sprite_.getPosition());
            return false;
        }

        return true;
    }

    void draw(sf::RenderWindow& window) const
    {
        if (loaded_)
        {
            window.draw(sprite_);
        }
    }

    sf::Vector2f getPosition() const
    {
        return sprite_.getPosition();
    }

    void setPosition(const sf::Vector2f& position)
    {
        sprite_.setPosition(position);
    }

private:
    static bool isWhiteBackgroundPixel(const sf::Color& color)
    {
        return color.r >= 245 && color.g >= 245 && color.b >= 245;
    }

    static void makeWhiteBackgroundTransparent(sf::Image& image)
    {
        const sf::Vector2u size = image.getSize();
        if (size.x == 0 || size.y == 0)
        {
            return;
        }

        std::queue<sf::Vector2u> pixelsToVisit;
        std::vector<bool> visited(size.x * size.y, false);
        const auto addIfBackground = [&](unsigned int x, unsigned int y) {
            const std::size_t index = y * size.x + x;
            if (!visited[index] &&
                isWhiteBackgroundPixel(image.getPixel(x, y)))
            {
                visited[index] = true;
                pixelsToVisit.push(sf::Vector2u(x, y));
            }
        };

        for (unsigned int x = 0; x < size.x; ++x)
        {
            addIfBackground(x, 0);
            addIfBackground(x, size.y - 1);
        }
        for (unsigned int y = 1; y + 1 < size.y; ++y)
        {
            addIfBackground(0, y);
            addIfBackground(size.x - 1, y);
        }

        while (!pixelsToVisit.empty())
        {
            const sf::Vector2u pixel = pixelsToVisit.front();
            pixelsToVisit.pop();
            const sf::Color color = image.getPixel(pixel.x, pixel.y);
            image.setPixel(pixel.x, pixel.y,
                           sf::Color(color.r, color.g, color.b, 0));

            if (pixel.x > 0)
            {
                addIfBackground(pixel.x - 1, pixel.y);
            }
            if (pixel.x + 1 < size.x)
            {
                addIfBackground(pixel.x + 1, pixel.y);
            }
            if (pixel.y > 0)
            {
                addIfBackground(pixel.x, pixel.y - 1);
            }
            if (pixel.y + 1 < size.y)
            {
                addIfBackground(pixel.x, pixel.y + 1);
            }
        }
    }

    static bool loadTextureWithoutWhiteBackground(sf::Texture& texture,
                                                   const char* path)
    {
        sf::Image image;
        if (!image.loadFromFile(path))
        {
            return false;
        }

        makeWhiteBackgroundTransparent(image);
        return texture.loadFromImage(image);
    }

    void setBirthFrame(std::size_t frame, const sf::Vector2f& position)
    {
        sprite_.setTexture(birthTextures_[frame], true);
        sprite_.setScale(0.45f, 0.45f);
        sprite_.setOrigin(
            birthTextures_[frame].getSize().x / 2.0f,
            birthTextures_[frame].getSize().y / 2.0f);
        sprite_.setPosition(position);
    }

    void setNormalFrame(std::size_t frame, const sf::Vector2f& position)
    {
        sprite_.setTexture(normalFrames_[frame], true);
        sprite_.setScale(normalScale_, normalScale_);
        sprite_.setOrigin(
            normalFrameWidths_[frame] / 2.0f, frameHeight_ / 2.0f);
        sprite_.setPosition(position);
    }

    void setEatingFrame(std::size_t frame, const sf::Vector2f& position)
    {
        sprite_.setTexture(eatingFrames_[frame], true);
        sprite_.setScale(0.45f, 0.45f);
        sprite_.setOrigin(
            eatingFrameWidths_[frame] / 2.0f, eatingFrameHeight_ / 2.0f);
        sprite_.setPosition(position);
    }

    void setSleepingFrame(std::size_t frame, const sf::Vector2f& position)
    {
        sprite_.setTexture(sleepingFrames_[frame], true);
        sprite_.setScale(0.32f, 0.32f);
        sprite_.setOrigin(
            sleepingFrameWidths_[frame] / 2.0f, sleepingFrameHeight_ / 2.0f);
        sprite_.setPosition(position);
    }

    void setAwakeningFrame(std::size_t frame, const sf::Vector2f& position)
    {
        sprite_.setTexture(awakeningFrames_[frame], true);
        sprite_.setScale(0.32f, 0.32f);
        sprite_.setOrigin(
            awakeningFrameWidths_[frame] / 2.0f, awakeningFrameHeight_ / 2.0f);
        sprite_.setPosition(position);
    }

    void setDeathFrame(std::size_t frame, const sf::Vector2f& position)
    {
        sprite_.setTexture(deathFrames_[frame], true);
        sprite_.setScale(0.32f, 0.32f);
        sprite_.setOrigin(
            deathFrameWidths_[frame] / 2.0f, deathFrameHeight_ / 2.0f);
        sprite_.setPosition(position);
    }

    static constexpr std::array<const char*, 6> birthImagePaths_ = {
        "Images/Nacer/Nacer 1.png",
        "Images/Nacer/Nacer 2.png",
        "Images/Nacer/Nacer 3.png",
        "Images/Nacer/Nacer 4.png",
        "Images/Nacer/Nacer 5.png",
        "Images/Nacer/Nacer 6.png"};
    static constexpr int frameHeight_ = 268;
    static constexpr int normalFrameCount_ = 5;
    static constexpr float normalFrameTime_ = 0.24f;
    static constexpr std::array<unsigned int, 5> normalFrameWidths_ = {
        270, 270, 270, 270, 268};
    static constexpr int eatingFrameCount_ = 3;
    static constexpr int eatingFrameHeight_ = 262;
    static constexpr float eatingFrameTime_ = 0.28f;
    static constexpr std::array<unsigned int, 3> eatingFrameWidths_ = {
        282, 282, 284};
    static constexpr int sleepingFrameCount_ = 3;
    static constexpr int sleepingFrameHeight_ = 768;
    static constexpr float sleepingFrameTime_ = 0.45f;
    static constexpr std::array<unsigned int, 3> sleepingFrameWidths_ = {
        469, 469, 470};
    static constexpr int awakeningFrameCount_ = 3;
    static constexpr int awakeningFrameHeight_ = 768;
    static constexpr float awakeningFrameTime_ = 0.45f;
    static constexpr std::array<unsigned int, 3> awakeningFrameWidths_ = {
        469, 469, 470};
    static constexpr int deathFrameCount_ = 3;
    static constexpr int deathFrameHeight_ = 358;
    static constexpr float deathFrameTime_ = 0.45f;
    static constexpr std::array<unsigned int, 3> deathFrameWidths_ = {
        446, 445, 445};

    std::array<sf::Texture, 6> birthTextures_;
    std::array<sf::Texture, 5> normalFrames_;
    std::array<sf::Texture, 3> eatingFrames_;
    std::array<sf::Texture, 3> sleepingFrames_;
    std::array<sf::Texture, 3> awakeningFrames_;
    std::array<sf::Texture, 3> deathFrames_;
    sf::Sprite sprite_;
    std::size_t currentBirthFrame_ = 0;
    int currentNormalFrame_ = 0;
    float normalAnimationTime_ = 0.0f;
    float normalScale_ = 0.45f;
    float eatingAnimationTime_ = 0.0f;
    int currentEatingFrame_ = 0;
    float sleepingAnimationTime_ = 0.0f;
    int currentSleepingFrame_ = 0;
    float awakeningAnimationTime_ = 0.0f;
    int currentAwakeningFrame_ = 0;
    float deathAnimationTime_ = 0.0f;
    int currentDeathFrame_ = 0;
    bool loaded_ = true;
    bool normalLoaded_ = false;
    bool eatingLoaded_ = false;
    bool sleepingLoaded_ = false;
    bool awakeningLoaded_ = false;
    bool deathLoaded_ = false;
};