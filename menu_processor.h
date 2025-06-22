#ifndef MENU_PROCESSOR_H
#define MENU_PROCESSOR_H

#include "screen_processor.h"
#include "scheduler.h"
#include <iostream>

void printHeader();
void processCommand(const std::string& command);

extern bool terminateProgram;

#endif
