#include <iostream>
#include <string>

void printHeader()
{
    std::cout << " ,-----. ,---.   ,-----. ,------. ,------. ,---.,--.   ,--. \n"
            << "'  .--./'   .-' '  .-.  '|  .--. '|  .---''   .- '\\  `.'  /  \n"
            << "|  |    `.  `-. |  | |  ||  '--' ||  `--, `.  `-. '.    /   \n"
            << "'  '--'\\.-'    |'  '-'  '|  | --' |  `---..-'    |  |  |    \n"
            << " `-----'`-----'  `-----' `--'     `------'`-----'   `--'    \n"
            << std::endl;
    std::cout << "\033[32mHello, welcome to CSOPESY commandline!\033[0m" << std::endl;
    std::cout << "\033[33mType 'exit' to quit, 'clear' to clear the screen\033[0m" << std::endl;
}

void processCommand(const std::string& command)
{
    if (command == "initialize" ||
        command == "screen" ||
        command == "scheduler-test" ||
        command == "scheduler-stop" ||
        command == "report-util")
        {
            std::cout << command << " command recognized. Doing something." << std::endl;
        }
    
    else if (command == "clear")
    {
#ifdef _WIN32 /*If on windows*/
        system("cls");
#else /*If on Linux/macOS*/
        system("clear");
#endif
        printHeader();
    }

    else if(command == "exit")
    {
        std::cout <<"Terminating command line emulator." << std::endl;
        exit(0);
    }

    else 
    {
        std::cout <<"Please enter a valid command." << std::endl;
    }
}

int main()
{
    printHeader();
    std:: string command;

    while(true)
    {
        std::cout <<"Enter a command: ";
        std::getline(std::cin, command);
        processCommand(command);
    }

    return 0;
}