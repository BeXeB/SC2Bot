## Requirements

- CMake
- Visual Studio Build Tools 2022
- Visual Studio, Rider or CLion
- Python
- Docker (If it should be used against other bots through [local-play-bootstrap](https://github.com/aiarena/local-play-bootstrap))

## How to run the project on Windows

### Step 1.

Download torchlib for the build configurations that you need and place them in the appropriate directories
Detailed instructions can be seen in [torch.md](./cpp_modules/externals/libtorch/torch.md).

### Step 2.

- In /cppmodules/vsstudio/
  - Run generate_vs2022.bat

This generates all the necessary Windows files, via Visual Studio Build Tools,
such that the Pybind library can be utilized via Python.

### Step 3.

#### If you use Rider/Visual studio:

- Open the generated sc2modules.sln in your IDE
- The module can then be built by running the ALL_BUILD configuration

#### If you use CLion:

- Open the SC2Bot folder as a project
- Navigate to "cpp_modules/CMakeLists.txt"
- Right-click the file and press "**Load CMake Project**"
- Change the configuration to "**sc2_mcts**" and press the build button
  - Note: You will not be able to run this by itself, but it can be built. In order to run it, it must be imported from the python application
- Make sure that all cmake profiles uses the **visual studio** toolchain instead of **mingw**

### Step 4.

- If you built the project with a Release profile
  - there will be a file called sc2_mcts.{your system specs}.pyd
  - This file can be imported into python with:
    - `import sc2_mcts`
  - An example of using the module can be seen in /cppmodules/module_test.py
- Make sure that the directory containing sc2_mcts.pyd is in your PYTHONPATH

## How to run the project via Docker and/or Linux

This bot works well with the external repository [local-play-bootstrap](https://github.com/aiarena/local-play-bootstrap)
which was introduced by AIArena. The run.py file in this project therefore adheres
to the integration into that repository.

- If you are using the aforementioned repository, put the SC2Bot into their
  "bots" folder
- Make sure that the file in `SC2Bot/cpp_modules/entrypoint.sh` has LF line separators
- Open a terminal in SC2Bot and enter the following command
  - `docker-compose up --build`
    - This will compile the cpp code to a python module that can be used in [local-play-bootstrap](https://github.com/aiarena/local-play-bootstrap)
- Navigate to the local-play-bootstrap folder, in which
  you must change the `matches` file, to include a match with SC2Bot
  - The name of the bot in the match must be the same as the folder name of the bot
- Navigate to the root of the local-play-bootstrap folder and enter the following command:
  - `docker-compose up`
    - The container should now build, and start all matches entered in the `matches` file
