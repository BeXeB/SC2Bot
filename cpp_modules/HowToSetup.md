### Requirements

- cmake
- visual studio build tools
- visual studio or rider
- python

### How to build the python module

- in /cppmodules/vsstudio/
  - Run generate_vs2022.bat
  - Then open the generated sc2modules.sln in your IDE
- The module can then be built by running the ALL_BUILD configuration
- If you built the project with a Release profile
  - there will be a file called pybind11module.{your system specs}.pyd
  - This file can be imported into python with:
    - `import pybind11module`
  - An example of using the module can be seen in /cppmodules/module_test.py
