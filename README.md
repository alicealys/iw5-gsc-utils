# iw5-script

Lua scripting support for Plutonium IW5

Works the same as it does in [IW6x](https://github.com/XLabsProject/iw6x-client/wiki/Scripting)

# How

* Download the latest version from the Releases tab
* Copy it to `%localappdata%/Plutonium/storage/iw5/plugins/`
* Create a `__init__.lua` file in a folder with a name of your choice in `%localappdata%/Plutonium/storage/iw5/scripts/`
* Example `%localappdata%/Plutonium/storage/iw5/scripts/myscript/__init__.lua`
* Run the server (preferably with the `-no-scripting` flag to disable ChaiScript)

Below are some features that are not available or documented in IW6x

# Chat notifies
```lua
level:onnotify("say", function(player, message)
    print(player.name .. " said: " .. message)
end)
```
or
```lua
level:onnotify("connected", function(player)
    player:onnotify("say", function(message)
        print(player.name .. " said: " .. message)
    end)
end)
```

# Player damage/killed callbacks

Callbacks can be added using the `game:onplayerkilled` or `game:onplayerdamage` functions:

Damage can be changed by returning it

Returning anything other than a number will not do anything (must be an integer)

```lua
game:onplayerdamage(function(_self, inflictor, attacker, damage, dflags, mod, weapon, point, dir, hitloc)
    damage = 0

    return damage
end)
```
```lua
game:onplayerkilled(function(_self, inflictor, attacker, damage, mod, weapon, dir, hitloc, timeoffset, deathanimduration)
    print(attacker.name .. " killed " .. _self.name)
end)
```

# Arrays
GSC arrays are supported and can be accessed similarly to gsc:
```lua
local ents = game:getentarray()

for i = 1, #ents do
    print(ents[i])
end

```

# Structs
GSC structs are also supported similarly as the arrays.

To get an entity's struct use the `getstruct` method:
```lua
local levelstruct = level:getstruct()

levelstruct.inGracePeriod = 10000
```
Structs in other variables like arrays are automatically converted:
```lua
level:onnotify("connected", function(player)
    player:onnotify("spawned_player", function()
        player.pers.killstreaks[1].streakName = "ac130"
        player.pers.killstreaks[1].available = 1
    end)
end)
```

Note: you cannot create new struct fields but only modify or read existing ones, same thing for arrays

# Functions

You can call (will not work for every function) functions and methods within the game's gsc scripts using the 

`scriptcall(filename, function, ...)` method:
```lua
level:onnotify("connected", function(player)
    player:onnotify("spawned_player", function()
        local hudelem = player:scriptcall("maps/mp/gametypes/_hud_utils", "createFontString", 1)
        
        hudelem:scriptcall("maps/mp/gametypes/_hud_util", "setPoint", "CENTER", nil, 100, 100)
        hudelem.label = "&Hello world"
    end)
end)
```
Functions in variables such as structs or arrays will be automatically converted to a lua function.

The first argument must always be the entity to call the function on (level, player...)
```lua
local levelstruct = level:getstruct()

level:onnotify("connected", function(player)
    player:onnotify("spawned_player", function()
        levelstruct.killstreakFuncs["ac130"](player)
    end)
end)
```
