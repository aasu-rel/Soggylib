# Soggylib

A shared library for **Plants vs. Zombies 2** modding. (64-bit ONLY!!!!!)
Yes, another one. No, this one's actually good.

---

# Features

- **Full lawn (wide view)** toggle in Settings - see more lawn. (add widescreen fix yourself tho)
- Native hook helpers for `libPVZ2.so`
- Custom tab in settings (Might be useful in future)
- Persistence helpers (your settings actually save now, wow)

---

# Build

Uses **CMake**. (Surprise.)

Just run `build.bat`. If it explodes, you're probably missing:

- Android NDK **r25+**
- CMake **3.18+**
- Ninja
- Patience

Output: `libSoggy.so`

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

# Credits

- **And64InlineHook** — Rprop
- **[Blazey's Example Mod](https://github.com/BlazeyLol/PVZ2ExpansionMod)** - Original repo for 9.6.1 libbing
- **[Plants vs Zombie discord (not offical one)](https://discord.gg/pvz)** - Awesome community
- **[Original Full lawn tab](https://github.com/CongJian833/PvZ2-LawnZoomTab)** - Original repo for full lawn from 9.8.1 [video](https://www.bilibili.com/video/BV1iNbX6DEy7/)

---

# License

MIT. Do whatever, just don't blame me.

PvZ2 belongs to PopCap / EA. Not affiliated, not endorsed, not sued (yet). No game assets included — go play the game.
