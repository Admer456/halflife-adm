
# Physics system

## Introduction
As a kid, I always loved physics and stuff. Particularly physics and destruction. Fascinating stuff. So I want to have that in Half-Life. I believe simple boxy collision shapes will still comply with the GoldSRC feel.

Not ragdolls though, that's way too 2004.

## Scope
Only basic rigid bodies. No ragdolls nor soft bodies.

## Implementation

### Phase 1 - Clientside physics
- Likely JoltPhysics
- Simple debris
- Debug visualisation
- Debris can be scattered around by the player
- Convex BSP hulls derived from the `.map` file
- MDL collision hulls
- Convex model decomposition

### Phase 2 - Serverside physics
- Physics brush entities
- Physics point entities
- Joints

## Gains
We'd have a nice foundation for not only HL2-style physics puzzles, but also vehicles.
