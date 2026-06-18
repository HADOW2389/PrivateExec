# PrivateExec — Build Guide

Target: Roblox version-8884371d30284041  
Stack: MSVC v143 / C++20 | .NET 8 WPF | x64

---

## Prerequisites

- Visual Studio 2022 (v17+) with:
  - Desktop development with C++ workload
  - MASM (Microsoft Macro Assembler) — included in the C++ workload
  - .NET desktop development workload (.NET 8)
- Windows SDK 10.0+

---

## Build order

### 1. Open the solution

```
PrivateExec.sln
```

Set configuration: **Release | x64**

### 2. Build Injector (C++ EXE)

Project: `Injector`

Output: `bin\x64\Release\Injector.exe`

This builds `Syscalls.asm` + `Syscalls.hpp` + `ManualMapper.hpp` + `main.cpp`
into a console EXE that takes `<pid> <dll_path>` as arguments.

### 3. Build Payload (C++ DLL)

Project: `Payload`

Output: `bin\x64\Release\Payload.dll`

This is the DLL that gets mapped into RobloxPlayerBeta.exe.
It starts the pipe server, resolves Lua function addresses, and
exposes the full UNC environment.

### 4. Build UI (C# WPF EXE)

Project: `UI`

Output: `bin\x64\Release\PrivateExec.exe`

### 5. Copy binaries to UI output folder

The UI looks for `Injector.exe` and `Payload.dll` next to `PrivateExec.exe`.
Run the post-build copy:

```bat
copy bin\x64\Release\Injector.exe bin\x64\Release\
copy bin\x64\Release\Payload.dll  bin\x64\Release\
```

Or add a post-build event to the UI project (already handled if you used the
.vcxproj → OutDir pointing to the same folder).

---

## Directory layout (after build)

```
bin\x64\Release\
    PrivateExec.exe    ← WPF UI
    Injector.exe       ← Manual-map injector
    Payload.dll        ← In-process DLL
    workspace\         ← Script workspace (auto-created)
```

---

## AoB pattern calibration

`Payload/Offsets.hpp` → `LuaOffsets::Init()` scans for:
- `luaL_loadbuffer`
- `luaD_call`
- `luaD_pcall`
- `lua_newthread`

If patterns change in a future binary update:
1. Open `RobloxPlayerBeta.exe` in IDA / Ghidra / x64dbg
2. Find each function (see existing pattern comments for reference)
3. Update `PAT_*` strings in `LuaOffsets` namespace
4. Rebuild Payload

All struct offsets (TaskScheduler, DataModel, Humanoid, etc.) are hardcoded
in `offsets::` namespace — verified for version-8884371d30284041.

---

## Identity level

`LuaEngine::SetIdentity(L, 8)` writes `8` to `*(int*)(L - 0x10)`.
If Roblox changes the ExtraSpace layout, adjust the `-0x10` offset:
1. Find a `LocalScript` with identity 2 running
2. In x64dbg: follow lua_State pointer, scan backward for value 2
3. The offset from L to that `int` is the new identity offset

---

## Notes

- The injector uses indirect syscalls + manual map — no LoadLibrary/WriteProcessMemory
  from userland hooks. Byfron cannot see the DLL in the PEB module list.
- The pipe name `PrivateExec_{pid}` is unique per process.
- UNC workspace folder defaults to `.\workspace\` next to the EXE.