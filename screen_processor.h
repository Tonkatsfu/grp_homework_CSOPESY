#ifndef SCREEN_PROCESSOR_H
#define SCREEN_PROCESSOR_H

#include <iostream>
#include <map>
#include <ctime>

struct ScreenDisplay
{
    std::string name;
    std::string processName;
    int currentLine;
    int totalLines;
    time_t creationTime;
};

extern std::map<std::string, ScreenDisplay> activeScreens;
extern std::string currentScreenName;

void ScreenConsoles(const ScreenDisplay& screen);

#endif
