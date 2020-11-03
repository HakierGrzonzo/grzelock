#include <iostream>
#include <vector>
#include <string>
#include "displayer.hpp"

int main (int argc, char** argv)
{   
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++)
    {
        int j = 0;
        std::string argument;
        while (argv[i][j] != '\0') argument.push_back(argv[i][j++]);
        args.push_back(argument);
    }
    res screen;
    if (args.size() != 3)
    {
        screen.x = 1366;
        screen.y = 768;
    }
    else
    {
        try
        {
            screen.x = std::stoi(args[1]);
            screen.y = std::stoi(args[2]);
        }
        catch (const std::invalid_argument& e)
        {
            screen.x = 1366;
            screen.y = 768;
        }
    }
    std::cout << "Setting resolution to " << screen.x << "x" << screen.y << std::endl;
    lockScreen(screen);
    return 0;
}
