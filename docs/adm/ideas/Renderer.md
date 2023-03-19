# Renderer rewrite

## Introduction

### The problem
It's simple. GoldSRC's renderer is inefficient. A quick look in Ghidra reveals that there are no glDrawArrays or glDrawElements calls, just a bunch of glBegin, glVertex3f/3i and glEnd calls.

As time moves on, our GPUs will become less and less (and less) supporting of this pipeline. It is already starting to show its limits in maps like dm_perthowned, or in Sven Co-op, where a simple flashlight can make an RX 560 drop on its knees, although that is mostly a CPU thing. The GPU happens to be waiting a lot.

All of this limits some of us modders, who want to have content-rich maps. I'm not even talking about HD assets (there's an audience for that too tho'), I'm talking about maps with lots of variety within them and probably lots of space.

### The workaround
It is a generally accepted assumption that GoldSRC cannot render large maps with a consistent, indoor level of detail. Imagine a 8000x8000 unit area with rocks and cacti every 128 units, some dunes here and there, then a grunt outpost. No, this is just plain impossible with vanilla GoldSRC.

The only way a map of such scale could be realised within GoldSRC's renderer and BSP limits is, having extremely low-poly terrain and very few rocks and cacti. I get that limitations breed creativity, but we've reached such a level of sophistication and quality standard that this is starting to become absurd. 

So, I propose a renderer rewrite on the clientside. It's an enormous task, but a worthwhile one.

[BSPExtra](BSPExtra.md) would also benefit from this a lot.

## Prerequisites
Some pretty good graphics programming knowledge.

[FoxGLBox](https://github.com/Admer456/FoxGLBox) could be used, or [Magnum](https://github.com/mosra/magnum), or [bgfx](https://github.com/bkaradzic/bgfx) or write yer own. Ideally, we'd want to be able to switch between OpenGL 2.1 and 3.3, so using an abstraction layer would be nice.

Potentially a custom audio system if we decide to turn on `r_norefresh`.

## Scope
A rewrite of GoldSRC's renderer with some extra features: a material system and shaders. No fancy shading, PBR or anything like that!

## Implementation

### Phase 1A - Texture replacements
We could use stb_image to load PNG and TGA files that replace WAD textures. Each map could have the equivalent of a `mapname_details.txt` file, where used textures are replaced with images, something like this:
```
OUT_RK03 textures/rocks03.png
PIPES01 textures/somepipes.tga
```

### Phase 1B - Initialisation
Because we wanna be able to switch OpenGL context versions, the main game window should be destroyed and `r_norefresh` should probably be 1, so the engine doesn't interfere with our rendering operations. That way, we can create a new game window and even support Vulkan, DX11 or DX12 if we wanted to.

Note that setting `r_norefresh` will cause the built-in sound system to stop updating the player's listener position. We'd need a custom audio system then.

### Phase 2 - UI rendering
*TODO: Write*
- the engine isn't rendering anything in its game window any more, use RmlUI
- try to build the WON HL menu with RmlUI

### Phase 3 - BSP rendering
*TODO: Write*
- load BSP
- load WADs
- render the whole BSP
- render lightmaps
- render brush entities
- support render modes and stuff
- materials

### Phase 4 - Animated model and sprite rendering
*TODO: Write*
- load MDL
- skinned animation
- load SPR
- render billboards
- detect if entities are visible

### Phase 5 - BSPX rendering
*TODO: Write*
- load BSPX
- don't render BSP
- render lightmaps in a different way to get rid of cryptic lightstyles
- occlusion culling via simplified portals

## Gains
Phase 1A would let us use high-resolution textures in the vanilla OpenGL renderer. This would be quite useful for larger terrain sections, for example, spreading a 2048x2048 texture across a large area, whereas in the map editor, that would be occupied by a 256x256 texture or a 512x512 texture.

By the end of phase 3, you'll be able to render basically entire BSPs with 10k wpolys, on Intel HD Graphics 4000 at least at 100fps. 

Having a material system and shaders will allow us to create special map surfaces with their own special effects. Using shaders, we could basically emulate the software water effect.

By the end of phase 5, we've reached everything there is to be done whilst retaining the GoldSRC look and aesthetic. We can make huge maps with over 100k wpolys, have high-res lightmaps and have as many switchable lights as we want. 

As a direct consequence of phase 1B, VGUI is no longer really used, and as such, we don't have to deal with it. Lightweight HTML-based UI for the win!
