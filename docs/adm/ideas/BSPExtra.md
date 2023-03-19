
# BSP extension

## Introduction
The GoldSRC BSP format is rather limited.

* Brush models: 512
* Clipnodes: 32767
* Worldfaces: 32768
* Worldleaves: 8192
* AllocBlock: 64

Mappers most frequently run into AllocBlock, clipnodes and worldleaves.

We're not lucky enough to have GoldSRC engine access, so we can't just directly change those. However, we can do some nasty workarounds.

## Prerequisites
A custom renderer is needed if we want to take advantage of BSP extensions in any meaningful way.

Some more extreme iterations of this idea would require custom physics as well, i.e. to be handled entirely in the mod.

## Scope
- External lightmaps
- External geometry
- Riddance of clipnodes
- Simplification of VIS

## Implementation

### Phase 1A
HLCSG and likely HLBSP would have to be modified. Ideally, a new tool would be introduced after HLBSP, let's call it HLBSPX. 

CSG and BSP would process the map as if every face had the `NULL` texture unless some other tool texture was used, such as `HINT`, `SKIP` or `CLIP`. Either way, this would **minimise AllocBlock** to 1/64.

All lightmaps would be stored in a .bspx file sitting next to the original .bsp. The same goes for all map polygons. This way, we could have **more wpolys** than 32k.

These lightmaps could be very high-res as a consequence of this, because we are suddenly no longer limited by the engine's renderer.

### Phase 1B
*TODO: write about this in more detail*  
Lightstyles pose a bit of a problem. It would be nice if we could have separate, additive lightmaps for each dynamic light, as opposed to the cryptic implementation of Quake lightstyles.

### Phase 2
Getting rid of clipnodes won't be an easy task by any means. UTIL_TraceLine would have to be completely rewritten, player movement code would also need to be aware of these new physics etc.

At this point, the .bsp has 0 clipnodes, the only thing used from it is the VIS data. The .bspx would contain the convex hulls (brushes) for our custom traceline code (most likely managed by a physics engine like Bullet or PhysX).

We almost wouldn't be limited by GoldSRC BSP any more.

### Phase 3
*TODO: write about this in more detail*  
**Option 1:** We might end up needing our own VIS to avoid worldleaves. However, this is very unlikely.

**Option 2:** Keep GoldSRC VIS for game purposes. func_detail could be completely ignored by HLCSG and HLBSP. HLBSPX would process it in its own BSPX format, which would be treated like a typical mesh, without any impact on worldleaves.

## Gains
Probably the biggest ones out of all.  
Large and detailed maps would be very feasible, still with the exception of 256 max visible entities. A lot of the annoyances like AllocBlock would simply not exist, and newcomers would find it much easier overall.

# Alternative
As a complete, 180° alternative, we could just have our own map format and let the BSP be one box room, and that's all the engine sees. 
