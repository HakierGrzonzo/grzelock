#include <SFML/Graphics/Color.hpp>
#include <SFML/Window/WindowStyle.hpp>
#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <time.h>
#include <stdio.h>
#include "pamHandler.h"
#include <thread>
#include <cstring>

//#define debug

struct res
{
    unsigned int x;
    unsigned int y;
};

struct Settings
{
    unsigned int resX;
    unsigned int resY;
    const char* background;
    const char* font;
};

enum unlockStatus
{
    _null = 0,
    success = 1,
    failure = 2,
    message = 3,
    notChecked = 4
};

struct unlock_callback
{
    unlockStatus status;
    char* message;
};

unlock_callback unlockStatus;

void tryUnlock(char* password, const char* username)
{
    unlockStatus.status = _null;
    unlockStatus.message = nullptr;
    const char* serviceName = "grzelock";
    int result = checkPassword(username, password, serviceName);
    if (result == 0)
    {
        unlockStatus.status = failure;
    }
    else
    {
        unlockStatus.status = success;
    }
}

// Get current date/time, format
const std::string currentDateTime(const char* format = "%H:%M") {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), format, &tstruct);

    return buf;
}

void centerOrigin(sf::Text* textptr, bool onlyY = false)
{
    sf::FloatRect bounds = textptr->getGlobalBounds();
    sf::Vector2f newOrigin;
    if (onlyY)
        newOrigin = sf::Vector2f (0 , bounds.height / 2);
    else
        newOrigin = sf::Vector2f (bounds.width /2, bounds.height / 2);
    textptr->setOrigin(newOrigin);
}

void lockScreen(Settings settings)
{
    sf::ContextSettings settingsSFML;
    settingsSFML.antialiasingLevel = 2;
    res screen;
    screen.x = settings.resX;
    screen.y = settings.resY;
    sf::RenderWindow window(sf::VideoMode(screen.x, screen.y), "grzeLock", sf::Style::Close | sf::Style::Fullscreen, settingsSFML);
    window.setVerticalSyncEnabled(true);
    
    // Handle background

    sf::Texture background;
    if (!background.loadFromFile(settings.background))
    {
        // on failure create something
        background.create(screen.x, screen.y);
        std::cerr << "Failed to open background file " << settings.background << std::endl;
    }
    background.setSmooth(true);

    sf::RectangleShape backgroundShape(sf::Vector2f(screen.x, screen.y));
    backgroundShape.setTexture(&background);


    // handle clock font

    sf::Font clockFont;
    if (!clockFont.loadFromFile(settings.font))
    {
        std::cout << "I can not proceed without font, Exiting!" << std::endl;
        window.close();
        return;
    }

    // handle clock

    sf::Text clock;
    clock.setFont(clockFont);
    clock.setCharacterSize(125);
    //clock.setOrigin(sf::Vector2f(0, 1));
    clock.setPosition(sf::Vector2f(30, screen.y - 175));
    clock.setFillColor(sf::Color::White);

    // handle date
    sf::Text date;
    date.setFont(clockFont);
    date.setCharacterSize(25);
    date.setPosition(sf::Vector2f(40, screen.y - 45));
    date.setFillColor(sf::Color::White);

    // handle password
    sf::Text passwordDisplay;
    passwordDisplay.setFont(clockFont);
    passwordDisplay.setCharacterSize(45);
    //passwordDisplay.setOrigin((sf::Vector2f(0.5, 0.5)));
    passwordDisplay.setPosition(sf::Vector2f(screen.x / 2, screen.y / 2));
    passwordDisplay.setFillColor(sf::Color::White);

    std::string time = "";
    std::string dateVal = "";

    std::string passwordBuffer = "";

    int lastPasswordSize = 0;

    unlockStatus.status = notChecked;
    unlockStatus.message = nullptr;

    std::thread pamInterface;

    while (window.isOpen())
    {
        sf::Event event;
        if (window.pollEvent(event))
        {
            if (event.type == sf::Event::EventType::Closed)
            {
                #ifdef debug
                window.close();
                std::cout << "Closing window, goodbye!" << std::endl;
                break;
                #endif
            }
            else if (event.type == sf::Event::EventType::Resized)
            {
                sf::Vector2u windowSize = window.getSize();
                std::cout << "Resized Now to " << windowSize.x << "x" << windowSize.y << std::endl;
            }
            else if (event.type == sf::Event::EventType::TextEntered && unlockStatus.status == notChecked)
            {
                char keyEntered = (char) event.text.unicode;
                if (passwordBuffer.size() > 0 && keyEntered == '\b')
                    passwordBuffer.pop_back();
                else if (keyEntered != '\r' && keyEntered != '\b')
                {
                    passwordBuffer += keyEntered;
                }
                else if (keyEntered == '\r')
                {
                    try 
                    {
                        pamInterface.join();
                    }
                    catch(std::exception &e)
                    {
                        //std::cerr << e.what() << std::endl;
                    }
                    // on enter pressed
                    char* passwordPtr = strdup(&passwordBuffer[0]); // copy password 
                    const char* username = std::getenv("USER"); // get current username
                    passwordBuffer = ""; // clear password buffer after copy
                    //std::cout << "Starting thread!" << std::endl;
                    pamInterface = std::thread(tryUnlock, passwordPtr, username); //launch auth thread;
                }
            }
        }
        // update time
        std::string newTime = currentDateTime();
        if (time != newTime)
        {
            time = newTime;
            clock.setString(time);
            centerOrigin(&clock, true);
        }
        // update date
        std::string newDate = currentDateTime("%A, %d %B %Y");
        if (newDate != dateVal)
        {
            dateVal = newDate;
            date.setString(dateVal);
            centerOrigin(&clock, true);
        }

        // update passwordDisplay
        if (unlockStatus.status == notChecked)
        {
            if (lastPasswordSize != passwordBuffer.size())
            {
                lastPasswordSize  = passwordBuffer.size();
                std::string passwordPlaceholder = "";
                for (int i = 0; i < lastPasswordSize; i++)
                    passwordPlaceholder.append("*");
                passwordDisplay.setString(passwordPlaceholder);
                centerOrigin(&passwordDisplay);
            }
        }
        else if (unlockStatus.status == failure)
        {
            std::string message = "Unlock failed!";
            lastPasswordSize = 0;
            passwordDisplay.setString(message);
            centerOrigin(&passwordDisplay);
            pamInterface.join();
            unlockStatus.status = notChecked;
        }
        else if (unlockStatus.status == success)
        {
            window.close();
            break;
        }

        window.clear(); 
        // draw stuff
        // Draw Background sprite
        window.draw(backgroundShape);
        window.draw(clock);
        window.draw(date);
        window.draw(passwordDisplay);
        window.display();
    }
    try
    {
        pamInterface.join();
    }
    catch(const std::exception& e)
    {
        //std::cerr << e.what() << std::endl;
    }
    
}