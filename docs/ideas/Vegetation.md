
# Vegetation system

## Introduction
The current way of placing vegetation into your map requires you to use cycler, cycler_sprite, env_sprite and similar entities. This is not ideal, because it consumes edicts (entity dictionary slots in the engine), uses up more bandwidth and requires transmitting to the client, which counts towards the limit of 256 max visible entities.

There are several workarounds that the artist can do, such as compile all the vegetation for the level into a single MDL, but that becomes harder to tweak later on, and there's the unfortunate chance of the entity disappearing under certain angles, due to the way GoldSRC does runtime visibility calculations.

## Prerequisites
We'd need an entity system localised entirely in the client DLL. Let's call this a clientside entity system.

This entity system would read entities from the BSP and instantiate needed ones on the clientside. Having them able to spawn, think and render themselves should be enough.

## Scope
- Simple vegetation rendering to replace serverside entities

## Implementation

### Phase 1
We gotta define at one key concept: a vegetation instance.  
I imagine vegetation instances to have at least these properties:
* Origin
* Rotation
* Scale
* Model (most likely a model index)
* Light (possibly a direct and ambient term)

Once all vegetation instances are loaded, they could then be submitted to the renderer in `HUD_CreateEntities`. 

```cpp
cl_entity_t renderEnt;
memset( &renderEnt, 0, sizeof( renderEnt ) );

renderEnt.origin = vegInstance.GetOrigin();
renderEnt.angles = vegInstance.GetAngles();
renderEnt.model = vegInstance.GetModel();

renderEnt.curstate.rendermode = kRenderNormal;
renderEnt.curstate.renderfx = kRenderFxNone;
renderEnt.curstate.renderamt = 255;
renderEnt.baseline.renderamt = 255;
renderEnt.curstate.framerate = 1.0f;
renderEnt.curstate.frame = 0;
renderEnt.curstate.scale = vegInstance.GetScale();

gEngfuncs.CL_CreateVisibleEntity( ET_NORMAL, &renderEnt );
```

### Phase 2A
A very important problem to solve would be collision detection, which also ties in with AI visibility. If we want the player to be able to hide behind bushes and trees, we'll need to expose the system to the serverside as well, form a "collision model" of some sorts.

This would involve modifying the serverside `UTIL_TraceLine` and a couple of other things. 

Next up, we'd need to make the player movement code aware of this as well. We could do something in `PM_Init` and `PM_Move`.

### Phase 2B
If a custom renderer gets implemented one day, this vegetation system would make perfect use of instanced rendering.

### Phase 2C
Expand the vegetation instances with these properties:
* render distance
* scale
* wind factor
* LODs
* sprites for furthest LOD

## Gains
We'd have a pretty nice system for vegetation, capable of rendering potentially hundreds or even a couple of thousand vegetation instances: trees, grass, bushes, rocks etc.
