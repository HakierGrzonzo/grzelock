#pragma once

struct Settings
{
    unsigned int resX;
    unsigned int resY;
    const char* background;
    const char* font;
};

void lockScreen(Settings settings);