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
# IO
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
 
