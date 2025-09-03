# BF1942_patcher

For some reason, no one published any patches for Battlefield 1942's debug executable `BF1942_r.exe`, 
so I collected all the tweaks found in patched main `BF1942.exe` executables scattered across the web and 
adapted them for `BF1942_r.exe`. 

This program allows to apply and/or disable these (+some of my own) tweaks for both executables, 
and can also generate `BlackScreen_r.exe` which should reduce crashes when using `BF1942_r.exe`. 


# Tweaks

- `Handle4GBAddressSpace`: Allows the game to access more memory (4 GB)
- `DefaultWindowPosition`: Allows to change default window position when running in windowed mode
- `ForceMaximizedWindow`: Makes game window maximized by default
- `DefaultWindowResolution`: Sets default game window resolution to a custom value. Technically, it should prevent the game from messing up your monitor/desktop settings when running in fullscreen mode
- `DontSetDefaultVideoMode`: Prevents setting default video mode in favour of loading it from user profile config files
- `FixAspectRatio`: Fixes aspect ratio for widescreens
- `InhibitAddToBuddyListFailedMessage`: Removes console output for error 'failed to add player to buddylist. Couldn't locate player'
- `CustomMasterServer`: Sets custom master server
- `CustomWindowTitle`: Sets custom game window title, in case you want to embed some information about the patches
- `MaxBotCount`: Increases maximum bot count
- `InternetGamesSupport`:  Enables internet games support for **debug executable**, allowing manual IP:port connection when server browser fails to find a LAN game. Note that `BF1942_r.exe` is incompatible with regular executables/servers, so this patch simply allows to connect by IP without having to launch the game with cmd arguments like `+joinServer` and `+isInternet`
- `MakePortable`: Makes the game portable. Virtually every patched `BF1942.exe` on the Internet has this applied (Team SiMPLE, Henk and others)


# How to use

0. [Verify](https://github.com/Casqade/BF1942_patcher/blob/main/README.md#supported-executables) that your game executables are supported by the patcher
1. Download latest [BF1942_patcher.exe](https://github.com/Casqade/BF1942_patcher/releases/latest)
2. Place `BF1942_patcher.exe` in the same directory 
as the executables you want to patch (`BF1942.exe`, `BF1942_r.exe` or `BlackScreen.exe`). 
The convenient way is just to drop it into the game root directory, 
though the patcher works in any folder as long as the patched executables exist in the same directory.
3. **MAKE A BACKUP of every executable you're going to patch!!!** 
Patcher doesn't do it automatically, and you'll probably be confused by autobackup files anyway. 
4. Launch the patcher and follow the on-screen prompts. 
If you've never worked with command-line interface, please refer to [the next section](https://github.com/Casqade/BF1942_patcher/blob/main/README.md#using-terminal)

For every tweak, this utility offers 3 modes of operation:
1. `Skip`: The bytes related to a particular tweak won't be touched (e.g. tweak won't be applied or removed)
2. `Restore`: The tweak will be removed/disabled (will restore bytes found in vanilla v1.61b or debug executable)
3. `Modify`: The tweak will be applied/enabled

Thus, to restore vanilla executable you should simply disable all the tweaks (`Restore`).

If you want to get an executable identical to Henk's *23.02.22* version, 
enable the following tweaks (and **disable** the others): 
- `Handle4GBAddressSpace`
- `DontSetDefaultVideoMode`
- `FixAspectRatio`
- `InhibitAddToBuddyListFailedMessage`
- `CustomMasterServer`: set to `master.bf1942.org`
- `CustomWindowTitle`: set to `BF1942 (Ver: Henk, 23 Feb. 2022)`
- `MakePortable`

Henk is the only one who wrote `0x56DE3A` to `CheckSum` field in `Optional Header` (offset `0x190`). 
I've no idea for what reason (except identification), because it isn't related to any CRC checks, 
and vanilla & other patched executables all have it set to all-zeroes. 


# Using terminal

A CLI program responds to user commands, so if you see a question or request 
followed by a blinking cursor, just type a command and press ENTER. 

When the patcher asks for one of 3 options, 
you can use `1` / `s` / `skip`, `2` / `n` / `no` or `3` / `y` / `yes` 
commands to skip, disable or enable tweak respectively. 
When the patcher asks for numbers or text, just type them. 
Pressing ENTER without typing anything will "select" a default value. 


# Supported executables

Unfortunately, there's no automatic way to reliably check whether the executable is supported or not, 
because there are A LOT of `BF1942.exe` flavours on the web, 
and I don't want to restrict the patcher to a small subset of executable versions. 
Moreover, some players patch the game themselves (e.g. update master server), 
so a simple hashum check won't be sufficient. Right now, only executable size is verified automatically. 

This patcher works best with an original/vanilla executables found in retail editions or published by EA, 
however it's possible to use it on other executable versions as well:

```
BF1942.exe variants:
  Size: 5648384 bytes (5516 KiB)
  TimeDateStamp: 417564C4 (2004-10-19T19:02:28.000Z)

  v1.61b (vanilla, retail with all DLCs and 2 official patches applied):
    Ver: Tue, 19 Oct 2004 14:58:45/scottab@BF_SERVER
    Modified: 2004-10-19
    CRC32: 9A8EBAA5
    MD5: A56D63E83EB5E02B43E1928CB22CD15A

  v1.61b (Origin)
    Sorry, I ain't going to install Origin and recover my abandoned account
    to download an abandoned game just to check if it works with my patcher :)
    Even though I had BF1942 in my library 10+ years ago, I'm not sure Origin
    still allows downloading it, as EA delisted the game from everywhere.
    If someone sends me the exe, I will verify it.

  SiMPLE (WWII Anthology)
    Ver: Tue, 09 Apr 2013 11:49:36/tuia@SiMPLE
    Modified: 2013-04-09
    CRC32: 652E7C0E
    MD5: 7541059ED468A2E34AAAAFF2BB4F83FC

  SiMPLE (for retail)
    Ver: Tue, 19 Oct 2004 14:58:45/scottab@BF_SERVER
    Modified: 2014-05-20
    CRC32: E82CC612
    MD5: 15CB9657CAB34CAD6B4E5B23E11E4B97

  SiMPLE (for Origin)
    Ver: Tue, 19 Oct 2004 14:58:45/scottab@BF_SERVER
    Modified: 2014-05-21
    CRC32: FA224DF0
    MD5: 228E8D361B8C200CF85C653F19FF3C64

// That's the weirdest one.
// Almost identical to SiMPLE variants,
// but has a slightly different CRC check bypass
// and also reports a different (v1.612) version to console/log
// (probably reports it to the server as well, but I didn't dig into this)
  v1.612
    Ver: Tue, 19 Oct 2004 14:58:45/scottab@BF_SERVER
    Modified: 2014-05-21
    CRC32: 4A4AB657
    MD5: F0DCC0872B7D0FE644D5F365011B43B9

  Henk:
    Ver: Henk, 23 Feb. 2022
    Modified: 2022-02-23
    CRC32: 2E7A6CB7
    MD5: B22B72718C79441FB12455951D398161

// Debug executable published by EA.
// Since I've never found any patches for debug executable,
// you probably have the right version
BF1942_r.exe:
  Size: 8454144 bytes (8256 KiB)
  TimeDateStamp: 4018E5BC (2004-01-29T10:51:40.000Z)
  Ver: Thu, 29 Jan 2004 11:34:56/afr@AFR-3
  Modified: 2004-01-29
  CRC32: BE4859F8
  MD5: 6A030767499776269E4922DD0F8164A4

// Vanilla executable.
// I doubt anyone ever patched it
BlackScreen.exe:
  Size: 6144 bytes (6 KiB)
  TimeDateStamp: 3F742DAD (2003-09-26T12:14:37.000Z)
  Modified: 2003-09-26
  CRC32: ECD5A069
  MD5: 7B62C94CC1C8571C8890B90BCB928E4D
```

If you know the exact executable from the list you're using, but you 
patched some strings (like master server or window title) or 
bytes (bot count, 4gb patch) yourself, the patcher should still work OK. 

In any case, the worst thing that can happen if you patch the wrong executable 
is probably a game crash upon launching or connecting to server. 


# Verifying executable

The most reliable way to verify the executable is by 
comparing either `CRC32` or `MD5` hashums with the ones from the list above. 
Or, if that's too difficult, you can compare `Modified` date, 
though it's very unreliable and non-representative.

There are many ways to calculate CRC32 and MD5, so here's how I do it:

Use [7-Zip](https://www.7-zip.org) (for CRC32 and MD5):

<img width="741" height="406" alt="image" src="https://github.com/user-attachments/assets/58d4b8ee-d95a-47a2-b609-d075619a5abe" />

---

Or use [CFF Explorer Suite](https://ntcore.com/explorer-suite) (for MD5 only):

<img width="710" height="441" alt="image" src="https://github.com/user-attachments/assets/a393b5bf-845d-4a6d-88bb-1b198c8b9bda" />


# Credits

It's hard to find the original author of a given patch, 
because I haven't seen any public `BF1942.exe` analyses. 

(probably inaccurate) credits:
- `Handle4GBAddressSpace`: Henk
- `DontSetDefaultVideoMode`: Henk
- `FixAspectRatio`: Team SiMPLE
- `InhibitAddToBuddyListFailedMessage`: Henk
- `CustomMasterServer` and `CustomWindowTitle`: probably Team SiMPLE. Though not really a patch, anyone with a hex editor can overwrite a string
- `MaxBotCount`: Ages ago I found several 128/256-bot versions on ModDB from different authors, but I don't remember which one I eventually used
- `MakePortable`: Team SiMPLE

Others I figured out myself and also adapted/ported every vanilla executable patch for `BF1942_r.exe`


# Thoughts on implementation and future plans

I tried to comment every byte sequence modification in the code, 
but bear in mind that all referenced function & class names shouldn't 
be relied upon since these're just my guesses deduced by analysing debug executable. 

Unfortunately, patching the executable statically is not flexible enough to achieve most things. 
In comparison, DLL injection allows runtime patching/hooking and adding new features without 
any modifications to the original executable, while also being easier for the user to configure. 
Given its benefits, maybe I'll implement this technique in another project or just fork 
[bf42plus](https://github.com/uuuzbf/bf42plus) and migrate all my patches there + implement new ones. 
