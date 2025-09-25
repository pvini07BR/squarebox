<img src="https://github.com/pvini07BR/squarebox/blob/main/assets/logo.svg">

## What is this?

This is a 2D, Minecraft-like game made completely with the C language and [Raylib](https://www.raylib.com).

Unlike other 2D sandbox games, this game is more focused in being an actual sandbox game with fun stuff instead of being a survival game.

This game has square chunks, which means it's "infinite" in both X and Y axes.

**NOTE**: This game still uses floats as coordinates so when you get too far from the origin, the game starts crumbling apart.

There is no available build to download due to the everchanging nature of this project.
See how to compile this project in [here](#how-to-compile).

I don't have too much plans on what to add in this game, so if you have a suggestion, feel free to do so!

But here is a list of all the features this game currently has, and what I plan on adding:

✅ is complete, 🟩 is W.I.P. or incomplete, ⬜ is not implemented at all, but planned.

- ✅ Can place blocks on foreground and background layers
- ✅ Special blocks like stairs, slabs and trapdoors (there is also vertical slabs, unlike a certain famous 3D voxel game.)
- ✅ Decorative blocks like grass and flowers (that's it)
- ✅ Sand that falls down
- ✅ Item system (containers, chests, etc.)
- ✅ Item entities (that's the only kind of entity in the game besides the player)
- ✅ Liquids (only water is added)
- ✅ Lighting system
- 🟩 World generation (there's only a generic noise world gen and it does not have any trees yet)
- ⬜ Structure generation
- ⬜ World saving
- ⬜ Moving structures (so you can make elevators)
- ⬜ Some redstone-like circuit system (with logic gates, pistons, buttons, pressure plates, etc.)
- ⬜ Proper coordinate system using fixed points
- ⬜ Multiplayer
- ⬜ Modding

## Controls

- WASD or Arrow keys to move.
- Press W, Up Arrow key or Space to jump or swim.
- Press F to toggle flying.
- Press Shift to move slower, and press Ctrl to move faster.
- Press E to open the creative menu.
- Press Q to drop an holding item to the ground.
- Press Z or X to cycle between possible states for a block (like stairs and slabs).

## Known bugs

- The collision system for the entities is currently not 100% robust, so entities might still clip through blocks, and it doesn't work well under low framerates.
- You can go up in blocks with small notches like stairs and slabs, but the code for this isn't very robust so it might feel wrong.
- Beware of crashes and memory leaks: This game has been entirely coded in C without any heavy testing, so be aware of that. but I did make some effort to make it crash and memory leak free. You can always report a issue in GitHub, you know that.

## How to compile 

# Windows (Visual Studio/MSVC)

You can simply clone this repo and open it with Visual Studio. It will automatically configure the project.
Then, on the top, make sure to select the correct executable (squarebox.exe).

# Linux (or GCC)

You will need CMake installed on your system.

Create a build folder on the same directory as the cloned repo, and run this command:

```cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON```

The ``-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`` argument is optional, use it if you wish to have code completion with clangd working in your code editor.

This project has only been tested on Windows with MSVC and Linux with GCC.
Any other compilers or operating systems is not confirmed nor garanteed to work, but you can try anyway.

## Credits

All arts and programming has been done by me, pvini07BR.

This game has been made using the [Raylib](https://www.raylib.com) library, made by raysan5.