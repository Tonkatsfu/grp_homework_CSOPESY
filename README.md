Developers:
1. Matthew Chua - S22
2. Ian Gabriel De Jesus - S22
3. Joemar Lapasaran - S22
4. Neo Monserrat - S22

### Compilation and Execution Steps for CMD

1. **Open Command Prompt (CMD)**  
   Navigate to your project folder:
   ```bash
   cd path\to\your\project\directory

2. Compile and run the program by entering the following:
   ```bash
   g++ main.cpp menu_processor.cpp scheduler.cpp screen_processor.cpp initialize.cpp cpu_tick.cpp cpu_tick_global.cpp -o my_app.exe -std=c++17 -pthread && CLI.exe

### Compilation and Execution Steps for VS code:
1. **Open VS Code**  
   Navigate to your project folder:
   ```bash
   cd path\to\your\project\directory
   ```bash
2. Compile and run the program by entering the following:
   ```bash
   g++ main.cpp menu_processor.cpp scheduler.cpp screen_processor.cpp initialize.cpp cpu_tick.cpp cpu_tick_global.cpp -std=c++17 -pthread; if ($?) { ./a.exe }

### Instructions
1. After compiling, type initialize to set up essential parameters (CPU, scheduler type, quantum cycles, frequency) and start the whole functionality of the program

