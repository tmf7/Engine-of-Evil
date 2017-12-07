--Engine of Evil--

Project built using Microsoft Visual Studio Express 2017 for Windows Desktop,
as well as Simple Directmedia layer 2.0.3 and a few of its extensions.

The goal is have this engine built as a library to be linked against for user-generated games.
It currently builds as a standalone executable with mini-test game data/behaviors.

Master branch tracks the current stable build.
Updates branch tracks changes in progress (not necessarily a stable build)
Both include my original source code.
Art assets are from opengameart.org.

To get the program running:
1) Pull or clone this repository and either:

2a) Run one of the executables in the Builds directory

OR:

2b) Open the MSVC project
3b) Build and execute


Alternatively:
--------------
1) Create a new project with your IDE of choice
2) Follow the instructions for downloading and including SDL2, SDL_ttf, and SDL_image in your project at:
https://www.libsdl.org/download-2.0.php
https://www.libsdl.org/projects/SDL_ttf/
https://www.libsdl.org/projects/SDL_image/
respectively.
	
Notes:
------
-> MSVC Compiler/linker resource directories are configured relative to the solution directory
-> SDL_ttf is used as a font handling extension to SDL2
-> SDL_Image is used to load image file types beyond bitmaps
-> SDL_Mixer is used to read soude files



