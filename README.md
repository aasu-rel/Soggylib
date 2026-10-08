# Soggylib

A shared library for **Plants vs. Zombies 2** modding. (64-bit ONLY!!!!!)
Yes, another one. No, this one's actually good.

This library required SnowieLib (Use for spawning zombies for sandbox mode) some some offsets might changes, so make sure to install slib :)

---

# Features

- **Full lawn (wide view)** toggle in Settings - Change view angle. (add widescreen fix yourself tho)
- Hide worldmap path (this is hell for me) - overwrite `m_unlockedNarrationID` (because I can't register new field) to hide the path (m_parentEvent)
```json
"m_parentEvent": "egypt_10",
"m_unlockedNarrationID": "egypt_10",
```
- Build date in build version tab in setting

---

# Build

Uses **CMake**. (Surprise hah.)

Just run `build.bat`. Edit `NDK_PATH` at the top if your NDK isn't at `C:/Android/ndk`.

If it explodes, you're probably missing:

- Android NDK **r25+**
- CMake **3.18+**
- Ninja
- Patience

Output: `output/libSoggy.so`

---

# Logging with ADB

Requires [ADB](https://developer.android.com/tools/adb) installed and on your `PATH`.

## USB device

```bash
adb logcat -s Soggy:* crash_dump:*
```

## Network emulator (MuMu, BlueStacks, LDPlayer, physical over Wi-Fi)

```bash
adb connect <emulator-ip>:<port>
adb logcat -s Soggy:* crash_dump:*
```

Common emulator ports:
```
MuMu : 7555
BlueStacks 5 : 5555
LDPlayer : 5555
Android Studio AVD : 5554
```

## Clear log and follow (Recommended)

```bash
adb logcat -c && adb logcat -s Soggy:* crash_dump:*
```

`-c` clears the buffer, `-s` silences all other tags.

## Save to file

```bash
adb logcat -s Soggy:* crash_dump:* > soggy.log
```

---

# Usage

1. Yeet `libSoggy.so` into your APK's `lib/arm64-v8a/` folder (make sure to delete armeabi-v7a)
2. Paste this code in `smali_classes2\com\popcap\PvZ2\PvZ2GameActivity`
   ```
   .line 57
    invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V

    const-string v0, "Soggy"
   ```
4. Launch the game
5. Figure out the feature

---

# Target

PvZ2 **9.6.1** (ARM64).

Got a different version? Cool. Update `offsets.h` yourself. good luck

---

# To-do list

### Sandbox mode
- Drag and drop zombies via seed packets
- Pages plant seed packets (same for zombie)
- Sandbox panel (Infinite suns, PFs, kill all zombies, etc)

**Editor note:** this may end up as a separate repository containing the required resources (level JSONs, module registrations) and install instructions.

---

# Credits

- **[And64InlineHook](https://github.com/rprop/and64inlinehook)** - Rprop
- **[Blazey's Example Mod](https://github.com/BlazeyLol/PVZ2ExpansionMod)** - Original repo for 9.6.1 libbing
- **[Plants vs Zombie discord (not offical one)](https://discord.gg/pvz)** - Awesome community
- **[Original Full lawn tab](https://github.com/CongJian833/PvZ2-LawnZoomTab)** - Original repo for full lawn from 9.8.1 [video](https://www.bilibili.com/video/BV1iNbX6DEy7/)
- **[Renojackson's Awesome lib](https://github.com/RenoJson/ARM64_example_injection_for_PvZ2/)** - Bunches of cool stuff for ARM64 lib (9.6.1) 

---

# License

CC0. Do whatever, just don't blame me.

If you use this lib without crediting me, well I don't really care but credit me please. :c

PvZ2 belongs to PopCap / EA. Not affiliated, not endorsed, not sued (yet). No game assets included — go play the game.

---

# My mod
Hey glad you made it this far, I guess wanna say that I'm currently working on **PvZ2 Soggy** which include **everything** in this lib (obviously) and bunch of cool contents, If you interested on being tester, feel free to send me a DM (aasu_rel). farewell modders.
