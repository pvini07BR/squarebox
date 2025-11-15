<p align="center">
	<img src="https://github.com/pvini07BR/squarebox/blob/main/assets/logo.svg">
</p>

<img src="https://github.com/pvini07BR/squarebox/blob/main/assets/showcase.png">

# What is this?

This is a 2D, Minecraft-like game made completely with the C language and [Raylib](https://www.raylib.com).

Unlike other 2D sandbox games, this game is more focused in being an actual sandbox game with fun stuff instead of being a survival game.

This game has square chunks, which means it's "infinite" in both X and Y axes.

**NOTE**: This game still uses floats as coordinates so when you get too far from the origin, the game starts crumbling apart.

There is no available build to download due to the everchanging nature of this project.
See how to compile this project on the [How to Compile](#how-to-compile) section below.

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
- ✅ World saving (beware that the world and chunk file formats may change)
- 🟩 World generation (there's only a generic noise world gen and it does not have any trees yet)
- 🟩 Some redstone-like circuit system (with logic gates, pistons, buttons, pressure plates, etc.)
- ⬜ Particles
- ⬜ Waterlogged blocks
- ⬜ Structure generation
- ⬜ Moving structures (so you can make elevators)
- ⬜ Chat and Commands
- ⬜ Proper coordinate system using fixed points
- ⬜ Multiplayer
- ⬜ Modding

# Controls

- WASD or Arrow keys to move.
- Press W, Up Arrow key or Space to jump or swim.
- Press F to toggle flying mode.
- Press Shift to move slower, and press Ctrl to move faster.
- Press E to open the creative menu.
- Press Q to drop an holding item to the ground.
- Press Z or X to cycle between possible states for a block (like stairs and slabs).
- Press Tab to change between foreground and background placing modes.

- Mouse left click to destroy a block, and right click to place a block at the cursor location.
- Scroll wheel to scroll through items in the hotbar.
- Ctrl + Scroll wheel to zoom in or out.

- Press F1 to show/hide game UI.
- Press F2 to take a screenshot (it will be saved as screenshot.png on the game's directory)
- Press F3 to show/hide debug info.
- Press F11 to toggle fullscreen mode (borderless window).

## For controller/gamepad:

**Warning: The current controller support implementation is very simple and has not been tested intensively. Your controller and/or operating system might not be supported.**

UI:
- Tilt the right stick to move the virtual cursor.
- Press A on Xbox, Cross on PlayStation or B on Nintendo to interact with an UI element.
- Press B on Xbox, Circle on PlayStation or A on Nintendo to go back to previous menu.

Gameplay:
- Tilt the left stick to move.
- Press A on Xbox, Cross on PlayStation or B on Nintendo to jump.
- Press Y on Xbox, Triangle on PlayStation or X on Nintendo to toggle flying mode.
- Press the left stick to move faster, and press the right stick to move slower.
- Press X on Xbox, Square on PlayStation or Y on Nintendo to open the creative menu.
- Press B on Xbox, Circle on PlayStation or A on Nintendo to drop the holding item to the ground.
- Press Left or Right on the D-Pad to cycle between possible states for a block (like stairs and slabs).
- Press Back/Change View on Xbox, Select/Touch Pad on PlayStation or Minus on Nintendo to change between foreground and background placing modes.

- Press left trigger to place a block, and right trigger to destroy a block at the cursor location.
- Press LB/RB on Xbox, L1/L2 on PlayStation or L/R on Nintendo to scroll through items on the hotbar.
- Press Up or Down on the D-Pad to zoom in or out.

Inventory:
- Press A on Xbox, Cross on PlayStation or B on Nintendo to pick up an item.

- Function keys behaviors are not implemented for the controller.

**Warning²: Currently, there is no way of changing keybinds or remap the controller buttons. They are hardcoded.**

# Known bugs

- The collision system for the entities is currently not 100% robust, so entities might still clip through blocks.
- Beware of crashes, bugs and memory leaks: This game has been entirely coded in C without any heavy testing, so be aware of that. but I did make some effort to make it crash and memory leak free. You can always report a issue in GitHub, you know that.

# How to compile 

**__Windows (Visual Studio/MSVC)__**

You can simply clone this repo and open it with Visual Studio. It will automatically configure the project.
Then, on the top, make sure to select the correct executable (squarebox.exe).

**__Linux (or GCC)__**

You will need CMake installed on your system.

Create a build folder on the same directory as the cloned repo, and run this command:

```cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON```

The ``-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`` argument is optional, use it if you wish to have code completion with clangd working in your code editor.

Then, simply ``cd build`` and run ``make``. Then run the compiled executable with ``./squarebox``.

This project has only been tested on Windows with MSVC and Linux with GCC.
Any other compilers or operating systems is not confirmed nor garanteed to work, but you can try anyway.

# Debugging Features

In the .vscode folder, there is a settings file for debugging the game on Visual Studio Code. Just press F5 and it will compile and launch the executable and debug it.

There is also the LOAD_WORLD define macro you can set to directly load into that specified world without going through the menu. This is useful for debugging anything on the game itself.

To set this macro, you run the command:

```cmake -B build -DLOAD_WORLD="worldname"```

``worldname`` is the name of the world you want to load into. If the world does not exist or fails to load, it will display an error on the console and throw you to the main menu.

# Credits

All arts and programming has been done by me, pvini07BR.

- This game has been made using the [Raylib](https://www.raylib.com) library, made by raysan5.
- Using [microui](https://github.com/rxi/microui) for the UI of this game, and [microui-raylib](https://github.com/marionauta/microui-raylib) to implement it for Raylib.