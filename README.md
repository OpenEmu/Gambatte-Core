Gambatte
========

This fork of the Gambatte core modifies Display Mode colour palette selection functionality for original Game Boy games.

* Super Game Boy palettes are added
* Display Mode selection modified and limited
 * Can no longer select between all available palettes
 * Internal software palettes: Super Game Boy, Game Boy Color
 * Hardware display palettes: DMG-01, MGB-001, MGB-101
 * Additional palettes: VirtualConsole-style greenscale, SGB grayscale
 * Optimal palette selection: colourful GBC > SGB game-specific > GBC game-specific > SGB grayscale
* Recommended custom palettes from Nintendo Power and other publications included

![alt text](https://www.mariowiki.com/images/d/d5/SML_SGB_1-F.PNG "Super Mario Land: sample of default palette application")
![alt text](https://www.mariowiki.com/images/2/25/GBC_SML_Title_Screen.png "Game Boy Color")
![alt text](https://www.mariowiki.com/images/9/96/SML_SGB_2-H.PNG "SGB monochrome palette")


### Cloning & Compiling

```
git clone --recursive https://github.com/TRIFORCE89/OpenEmu.git
cd OpenEmu
xcodebuild -workspace OpenEmu.xcworkspace -scheme "Build & Install Gambatte" -configuration Release
```
