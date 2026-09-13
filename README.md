# Soggylib

A shared library for **Plants vs. Zombies 2** modding.
Yes, another one. No, this one's actually good.

---

# Features

- **Full lawn (wide view)** toggle in Settings — see more lawn.
- Native hook helpers for `libPVZ2.so`
- Settings UI injection (EA won't let me add tab via softcode)
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

1. Yeet `libSoggylib.so` into your APK's `lib/arm64-v8a/` folder (make sure to delete armeabi-v7a)
2. `System.loadLibrary("Soggylib");`
3. Launch the game
4. Profit

---

# Target

PvZ2 **9.6.1** (ARM64).

Got a different version? Cool. Update `offsets.h` yourself. Good luck.

---

# Credits

- **And64InlineHook** — Rprop
- **[Blazey's Example Mod](https://github.com/BlazeyLol/PVZ2ExpansionMod)** — Original repo for 9.6.1 libbing
- **[Plants vs Zombie discord (not offical one)](https://discord.gg/pvz)** — Awesome community

---

# License

MIT. Do whatever, just don't blame me.

PvZ2 belongs to PopCap / EA. Not affiliated, not endorsed, not sued (yet). No game assets included — go play the game.
