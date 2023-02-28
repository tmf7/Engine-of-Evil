# Engine of Evil

## Background

Project built using Microsoft Visual Studio Express 2017 for Windows Desktop,
as well as Simple DirectMedia Layer 2.0.3 and a few of its extensions.

The goal is have this engine built as a library to be linked against for user-generated games.
It currently builds as a standalone executable with mini-test game data/behaviors.

Art assets are from opengameart.org.

## A Few Examples

Learn more at https://www.tomfreehill.com

![EoE Entity Controls](C:\Users\Tom Freehill\Desktop\GIFS\EoE_WaypointEntityControl.gif)

![EoE Collision](C:\Users\Tom Freehill\Desktop\GIFS\EoE_Collision.gif)

![EoE Navigation](C:\Users\Tom Freehill\Desktop\GIFS\EoE_Navigation.gif)

![EoE 3D Renderbounds](C:\Users\Tom Freehill\Desktop\GIFS\EoE_RenderBounds.gif)

![EoE Entity Prefabs](C:\Users\Tom Freehill\Desktop\GIFS\EoE_EntityPrefabs.gif)



## Runtime Keyboard + Mouse Controls

The below controls are all setup for debug and testing purposes, and as such are non-exhaustive of all the uses of the Engine of Evil input system.

### Camera Controls

| Key Binding          | Function                                                     |
| -------------------- | ------------------------------------------------------------ |
| =/- or mouse3 scroll | Camera Zoom in/out                                           |
| spacebar             | center view on 0th selected entity (commented out in Camera.cpp Line 64) |
| WASD                 | Pan Camera up, down, left, and right                         |

### Player Controls

| Key Binding | Function                                                     |
| ----------- | ------------------------------------------------------------ |
| mouse1      | If NO entities are selected, then click-and-drag around one or more entities to select them |
| mouse1      | If ANY entities are selected, then single-click will add waypoints where entity/entities should walk |
| spacebar    | clear current entity group selection                         |
| m           | toggle selected entity group pathing state (compass, wall follow, manual) |
| r           | clear pathfinding trail (only if KNOWN_MAP_CLEAR debug flag is set) |
| Arrow Keys  | Move selected entity group up, down, left, and right         |

### Debug Controls

| Key Binding | Function                                                     |
| ----------- | ------------------------------------------------------------ |
| tab         | show / hide debug flags                                      |
| shift       | select debug flag from list (red is selected, white is not selected) |
| ctrl        | toggle selected debug flag state (true / false)              |



## Branch Information

* **Master**: tracks the current stable build
* **Updates**: tracks changes in progress (not necessarily a stable build)
* **ComponentModel**: Experimental branch using a dynamic Component Aggregation model where a GameObject becomes a handle for a collection of any number and type of components. (not necessarily a stable build, but some great lessons have been learned, for example this [GetComponent question I answered on StackOverflow](https://stackoverflow.com/questions/44105058/implementing-component-system-from-unity-in-c/48152454#48152454))
* **GameObjects**: Originally used for GameObject feature development, since merged into master
* **CrossProject**: Originally used for integrating features from some of my other projects, since merged into master



## How to Build

1) Download or fork this repository
2) Open the Visual Studio solution (you may need to update the solution you your version of VS)
3) Ensure the build configuration is set to x86 because the SLD2 libraries are compiled for x86
4) To load different maps modify Map.cpp line 36 ```return LoadMap("Graphics/Maps/EvilMaze.emap");``` to point to your map of choice.
5) Build and run
   * If you run the exe from the builds directory make sure to copy the **Graphics** and **Audio** folders into your build directory (I didn't bother writing a post-build script that does this for you).
6) Feel free to edit and build on what I've created, or even experiment with making a game with the Engine of Evil.
   * Just please share whatever you make with the world, and have fun!

### Alternatively:

1) Create a new project with your IDE of choice
2) Follow the instructions for downloading and including SDL2, SDL_ttf, and SDL_image in your project at:
	https://www.libsdl.org/download-2.0.php
	https://www.libsdl.org/projects/SDL_ttf/
	https://www.libsdl.org/projects/SDL_image/
	respectively.
	

### Build Notes:

* MSVC Compiler/linker resource directories are configured relative to the solution directory
* SDL_ttf is used as a font handling extension to SDL2
* SDL_Image is used to load image file types beyond bitmaps
* SDL_Mixer is used to read sound files

## Resource Files

The Engine of Evil uses several runtime-loaded resource definition files. This table covers all Engine of Evil file types and where the code that loads then into memory can be found. The parsing functions listed all contain a breakdown of their respective file formats and additional info. In some cases further information can be found in external engine files, which are also listed in this table.

| File Extension | Function                                                     | Parsing Code                                                 |
| -------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| .eimg          | Image file. Bounding boxes within an image file that inform the renderer which pixels to sample on the GPU textures  (eg: sprite sheets) | ```eImageManager::LoadAndGet```                              |
| .bimg          | Batch image file. A group of .eimg files to batch load       | ```eResourceManager<type>::BatchLoad```                      |
| .etls          | Tileset file. Tiles used when building the static and dynamic tiles of a layered tilemap. Defines subframe within the image definition, collision, renderbounds. | ```eTileImpl::LoadTileset```                                 |
| .emap          | Map file. Defines the layout of the game world, including where tiles from a tile set (.etls) should be on the game world grid, the size of the game world grid, and instance data for spawned entity prefabs. Note: Both the collision world and render world use the same grid size for spatial partitioning but use different data sets. | ```eMap::LoadMap```                                          |
| .eprf          | Entity prefab file. The components and their properties of a uniquely spawnable entity. All components are optional. The master branch has a hard-coded set of components per entity, however the ComponentModel branch is a work-in-progress to a component aggregation model. | ```eEntityPrefabManager::LoadAndGet```                       |
| .bprf          | Batch entity prefab file. A group of .eprf files to batch load | ```eResourceManager<type>::BatchLoad```                      |
| .eanim         | Animation file. The source image (or images) and the subframe timing of a sprite animation. Note that time is normalized by default [0.0-1.0]. A single animation can be a composite of several different image files, see **AnimationInfo.eanim** for more information. | ```eAnimationManager::LoadAndGet```                          |
| .banim         | Batch animation file. A group of .eanim files to batch load  | ```eResourceManager<type>::BatchLoad```                      |
| .ectrl         | Animation Controller file. A state machine composed of stand-alone Animation States and composite animation Blend State, connected by State Transitions, and controlled by atomic Controller parameters. Parameter types include: int, float, bool, and trigger. See **animationControllerInfo.ectrl** for more information. | ```eAnimationControllerManager::LoadAndGet```                |
| .ttf           | Font file. Standard outline font files loaded using SDL_ttf  | ```eRenderer::Init``` and ```TTF_OpenFont```                 |
| .wav           | Audio file. Standard windows audio file loaded using SDL_mixer | ```eGameLocal::Init```, ```eMusic::Load```, ```eSoundFx::Load```, and ```Mix_LoadWAV``` |

