# **Note**: The Unified SDK is still in development and not ready for use beyond alpha testing and feedback.

# Developing with the Unified SDK

## Prerequisite knowledge

You will need a working knowledge of command line interfaces, CMake and C++ to make a mod with this SDK.

Various configuration files use JSON so an understanding of its syntax is recommended.

Tools and scripts are written in C# so you will need to have an understanding of its syntax to modify them.

Resources to learn these things:
* [Command line](https://learn.microsoft.com/en-us/windows-server/administration/windows-commands/windows-commands)
* [CMake](https://cliutils.gitlab.io/modern-cmake/)
* [C++](https://www.learncpp.com/)
* [C#](https://learn.microsoft.com/en-us/dotnet/csharp/)
* [JSON](https://www.w3schools.com/js/js_json_syntax.asp)

> **Make sure to learn these things first so you understand how this SDK works!**

## Overall project structure

The source code for the mod SDK is in the repository that this file is included with. This also includes the source code for the original tools included with the Half-Life SDK as well as `delta.lst` which is used to configure networking settings for the mod. `liblist.gam` is also included.

Mod installations are available on the releases page of this repository.

The source files for game assets are stored in the [Asset Source Files](docs/README.md#developer-resources) repository. This includes sources for all types of models, configuration files, scripts, fgds; anything that's not mod source code.

See the README for more information on what exactly is stored there.

The source code for tools written in C# are stored in the [C# Tools Source Code](docs/README.md#developer-resources) repository. These tools are used to automate some parts of development such as the copying of source files from the assets repository to the game installation, as well as the packaging of mods and installation of game content taken from the original games.

The tools are included with mod installations.

## Setting up the source code

See [Setting up the source code](BUILDING.md)

## Server library name

Historically the name of a mod's server library defaults to `hl` because this is what it is called for Half-Life. Some games and mods change the name to match the game name and others change it to just `server`.

To keep things simple the name has been changed to `server`.

## Launching your mod through Steam

In order to have the game appear in the Steam game list you will have to add a `liblist.gam` file.

The SDK includes `liblist.gam`, located in the `config` directory and named `liblist.gam.in`. This file is a template used to generate the actual file which is copied to the mod directory when you build the `INSTALL` target.

When creating a new mod, change the `game` value to change the display name for your mod. This will be used in Steam and as the window title for the game.

## Development process

Because there is no repository for the mod installation itself assets need to be copied from the assets repository to the game installation.

The [AssetSynchronizer](docs/tools/asset-synchronizer.md) tool automates this. When you run it it will copy any files that are different and will then monitor the assets repository for changes to automatically mirror to the mod directory.

This tool uses a configuration file to control which files are watched and where they are copied.

The [Packager](docs/tools/packager.md) tool is used to create the mod installation archive. This tool uses a configuration file to control which files are included and excluded and creates an archive containing the installation.

Archive filenames include a UTC timestamp to uniquely identify them. On successful completion old archives are automatically removed.

The [Installer](docs/tools/installer.md) tool is used to copy, convert and upgrade maps from Half-Life, Opposing Force and Blue Shift.

Original maps can't be redistributed, and changes made to SDK code require changes to be made to these maps. This tool does that automatically.

The [MapUpgrader](docs/tools/map-upgrader.md) tool is a standalone version of the installer's map upgrade functionality. It can be used to upgrade a map to the Unified SDK, provided that the map was made for the original version of Half-Life and not another mod or game.

Most modders won't need to use these tools, but they are there to streamline the development process as much as possible.
