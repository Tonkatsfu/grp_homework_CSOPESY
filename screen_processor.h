#ifndef SCREEN_PROCESSOR_H
#define SCREEN_PROCESSOR_H

#include <iostream>
#include <map>
#include <ctime>

struct Process;

struct ScreenDisplay
{
    std::string name;
    std::string processName;
    int currentLine;
    int totalLines;
    time_t creationTime;
};

//extern std::map<std::string, Process*> activeScreens;
extern std::string currentScreenName;

void ScreenConsoles(const Process& process);
void ProcessSMI(const std::string& processName);

#endif
