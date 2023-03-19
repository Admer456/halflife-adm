
# C# Scripting

## Introduction
This is one of those crazy and probably overkill ideas. Practically speaking, there is no need for C# in Half-Life modding, however, it would be fun to think about.

It would allow for code changes without needing new binaries. Depending on the scope, custom maps could have their own scripts too. 

Here's an imaginary entity in this system:
```cs
namespace GameEntities
{
    class FuncBreakable : public BaseEntity
    {
        [EntityLink("func_breakable", EntityType.BrushEntity)]
        public void Spawn()
        {
            SetMovetype( Movetype.None );
            SetSolid( Solid.BSP );
            SetModel( ModelName );
        }

        public void Use( ref BaseEntity activator, ref BaseEntity caller, UseType useType, float value )
        {
            Gib();
        }
    }
}
```

## Scope
- Addon scripts (a.k.a. server plugins)
- Clientside HUD scripts
- Serverside gamemode scripts
- Map entity scripts
- Map scripts
- Monster scripts
- Weapon scripts

## Implementation

### Phase 1
*TODO: write about this in detail*

We gotta define some terms:
- Host = the app/library that acts as a host to the C# scripts
- Managed game = the library that manages the C# scripts
- Scripts = C# code files in the mod's `scripts` folder

The first approach I thought of was sending data between the host and the managed game. Actual `CBaseEntity` pointers and `pev` and stuff.  
But then I thought of something else. There can be a "bridge" between the host and the managed game.

```cs
public class Bridge
{
    // On the host, this will be like a function pointer
    public delegate void DSetModel( int entityNumber, string modelName );
    // The structure we import from the game, it gives us the host's callbacks
    public struct GameImports
    {
        public DSetModel SetModel;
    }

    // There are the functions we'll call
    private GameImports game;

    // The host will call this method to export its functions to us
    public delegate void DImportFunctions( GameImports gi );
    public static void ImportFunctions( GameImports gi )
    {
        game.SetModel = gi.SetModel;
    }
}
```

Then, in some entity class, we could do this:
```cs
class BaseEntity
{
    public void SetModel( string modelName )
    {
        game.SetModel( EntityNumber, modelName );
    }
}
```

EntityNumber would be determined when the entity spawns, but you get the idea.   
On the host, the `SetModel` callback would look like this:
```cpp
class Bridge
{
public:
    static void SetModel( int entityNumber, const char* modelName )
    {
        CBaseEntity* ent = UTIL_GetEntityByIndex( entityNumber );

        if ( nullptr != ent )
        {
            ent->SetModel( modelName );
        }
    }
};
```

There are ways to speed this up, but this is the general idea behind it.

## Gains
We'd probably get a few more Unity folks and S&box folks, and people who don't like C++ but like C#. It'd be more beneficial if all Half-Life entities were ported to C# and everything was managed there.
