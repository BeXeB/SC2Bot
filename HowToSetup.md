### Requirements

- CMake
- Visual Studio Build Tools 2022
- Visual Studio, Rider or CLion
- Python

## How to run the project on Windows
### Step 1.
- In /cppmodules/vsstudio/
  - Run generate_vs2022.bat

This generates all the necessary Windows files, via Visual Studio Build Tools, 
such that the Pybind library can be utilized via Python.

### Step 2.
#### If you use Rider:
- Open the generated sc2modules.sln in your IDE
- The module can then be built by running the ALL_BUILD configuration

#### If you use CLion:
- Open the SC2Bot folder as a project
- Navigate to cpp_modules/CMakeLists.txt
- Right-click the file and press "Load CMake Project"
- Change the configuration to "sc2_mcts" and press the build button
  - Note: You will not be able to run this by itself, but it can be built

### Step 3.
- If you built the project with a Release profile
  - there will be a file called sc2_mcts.{your system specs}.pyd
  - This file can be imported into python with:
    - `import sc2_mcts`
  - An example of using the module can be seen in /cppmodules/module_test.py

## How to run the project via Docker and/or Linux
This bot works well with the external repository [local-play-bootstrap](https://github.com/aiarena/local-play-bootstrap)
which was introduced by AIArena. The run.py file in this project therefore adheres
to the integration into that repository.

- If you are using the aforementioned repository, put the SC2Bot into their
"bots" folder
- Then via command line, navigate to the cpp_modules folder of our bot
- Ensure that the vsstudio folder is empty, or only consists of the
"generate_vs2022.bat" file, which will not be used here
- Enter the command 'docker build -t .', which allows you to build the image of
our cpp_modules for the target os architecture, such that it works inside the
container
- Then, via command line, navigate to the local-play-bootstrap folder, in which
you must change the "matches" file, to include a match with SC2Bot
- Lastly, while still using the command line at the root of the local-play-bootstrap
folder, run the 'docker compose up' command. The container should now build, and
the match will start