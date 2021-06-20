# iw5-gsc-utils

This plugin adds some useful functions/methods to IW5's GSC VM

# Misc

* `executeCommand(command)`: Executes a console command.
* `replaceFunc(what, with)`: Replaces a function with another:

  ```c
  init()
  {
      replaceFunc(maps\mp\gametypes\_damage::Callback_PlayerDamage, ::callbackPlayerDamage);
  }

  callbackPlayerDamage(eInflictor, eAttacker, iDamage, iDFlags, sMeansOfDeath, sWeapon, vPoint, vDir, sHitLoc, timeOffset)
  {

  }
  ```
* `addCommand(name, callback)`: Adds a console command (gets removed after a map restart):

  ```c
  init()
  {
      addCommand("kill", ::cmd_kill);
  }
  
  cmd_kill(args)
  {
      if (args.size < 2)
      {
          print("Usage: kill <num>");
          return;
      }

      num = int(args[2]);
      player = getEntByNum(num);

      if (isPlayer(player))
      {
          player suicide();
      }
  }
  ```
# Player
* `say(message)`: Prints a message to all players' chat.

* `self tell(message)`: Prints a message to the player's chat.
* `self setName(name)`: Sets a player's name.
* `self resetName(name)`: Resets a player's name to its original.
* `self setClantag(name)`: Sets a player's clantag.
* `self resetClantag(name)`: Resets a player's clantag to its original.
* `self removeClantag(name)`: Removes a player's clantag.
# IO

The basepath for all IO functions is `Plutonium/storage/iw5`

* `fopen(path, mode)`: Opens a file of given name with given mode, returns a file stream.
* `fwrite(stream, text)`: Writes a string to a stream.
* `fread(stream)`: Reads entire file.
* `fclose(stream)`: Closes a file stream.
* `fremove(path)`: Deletes a file.

  ```c
  init()
  {
      basePath = getDvar("fs_basegame") + "/";
      
      file = fopen(basePath + "test.txt", "w");
      fwrite(file, "test");
      fclose(file);

      file = fopen(basePath + "test.txt", "r");
      print(fread(file));
      fclose(file);
  }
  ```
 * `fileExists(path)`: Returns true if the file exists.
 * `writeFile(path, data[, append])`: Creates a file if it doesn't exist and writes/appends text to it.
 * `readFile(path)`: Reads a file.
 * `fileSize(path)`: Returns file size in bytes.
 * `createDirectory(path)`: Creates a directory.
 * `directoryExists(path)`: Returns true if the directory exists.
 * `directoryIsEmpty(path)`: Returns true if the directory is empty.
 * `listFiles(path)`: Returns the list of files in the directory as an array.
 * `copyFolder(source, target)`: Copies a folder.

# JSON

* `jsonSerialize(variable[, indent])`: Converts GSC variables (such as arrays) into JSON:

  ```c
  init()
  {
      array = [];
      array[0] = 1;
      array[1] = 2;
      json = jsonSerialize(array, 4);
      
      print(json);
      
      /*
        [script]: [
            2,
            1
         ]
      */
  }
  ```
  
  This function can also be useful to reveal contents of existing arrays such as `game`:
  ```c
  init()
  {
      print(jsonSerialize(game["allies_model"], 4));
      
      /*
      [script]: {
          "ASSAULT": "[function]",
          "GHILLIE": "[function]",
          "JUGGERNAUT": "[function]",
          "LMG": "[function]",
          "RIOT": "[function]",
          "SHOTGUN": "[function]",
          "SMG": "[function]",
          "SNIPER": "[function]"
      }
      */
      
      print(jsonSerialize(game["music"], 4));
      
      /*
      [script]: {
          "defeat_allies": "UK_defeat_music",
          "defeat_axis": "IC_defeat_music",
           "losing_allies": "UK_losing_music",
           "losing_axis": "IC_losing_music",
           "losing_time": "mp_time_running_out_losing",
           "nuke_music": "nuke_music",
           "spawn_allies": "UK_spawn_music",
           "spawn_axis": "IC_spawn_music",
           "suspense": [
               "mp_suspense_06",
               "mp_suspense_05",
               "mp_suspense_04",
               "mp_suspense_03",
               "mp_suspense_02",
               "mp_suspense_01"
           ],
           "victory_allies": "UK_victory_music",
           "victory_axis": "IC_victory_music",
           "winning_allies": "UK_winning_music",
           "winning_axis": "IC_winning_music"
       }
      */
  }
  ```
* `jsonParse(json)`: Converts JSON into a GSC variable:

  ```c
  init()
  {
      array = jsonParse("[1,2,3,4]");
      print(array[0] + " " + array[1] + " " + array[2] + " " + array[3]);
      
      /*
        [script]: 1 2 3 4
      */
  }
  ```
* `array(...)`: Creates an array from arguments, not very relevant to JSON but can be useful to make code look better.

  Instead of doing this:
  ```c
  init()
  {
      array = [];
      array[0] = 1;
      array[1] = 2;
      array[2] = 3;
      array[3] = 4;
      
      print(jsonSerialize(array, 4));
      /*
        [script]: [
            4,
            3,
            2,
            1
        ]
      */
  }
  ```
  
  You can do this:
  
  ```c
  init()
  {
      array = array(1, 2, 3, 4);
      print(jsonSerialize(array, 4));
      /*
        [script]: [
            4,
            3,
            2,
            1
        ]
      */
  }
  ```
  
* `map(...)`: Simlar to `array(...)` but creates a string-indexed array:
  
  ```c
  init()
  {
      array = map("first", 1, "second", 2);
      print(jsonSerialize(array, 4));
      /*
          [script]: {
              "first": 1,
              "second": 2
          }
      */
  }
  ```
 * `jsonPrint(...)`: Prints values as json.
 # Credits
 * [xensik](https://github.com/xensik)
