## Features

Fake rcon adds the fake_rcon_password and fake_rcon command because valve has not integrated the orignel command into the game (or it is broken).

## How to configure it ?
To configure the rcon, you must create an rcon.txt file in the **/game/bin/{OS}** folder, where is your CS game binary.
If you want you can use **-fakercon** as commandline, it will take priority over the rcon.txt file

**The password must be at least 4 characters long. We use a file because convar management is not yet complete on the cs2 SDK**

## How to use it ?
fake_rcon_password YOURPWD
fake_rcon say hello