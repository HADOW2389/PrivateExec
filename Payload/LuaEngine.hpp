#pragma once
#include "Offsets.hpp"
#include <string>
#include <vector>
#include <Windows.h>

// ── SEH wrappers — must be plain functions with NO C++ objects in scope ───────
// MSVC C2712: __try cannot appear in a function that requires object unwinding.
// Workaround: dedicated wrapper functions that only hold POD locals.

static int _seh_dcall(void* fn, void* L) {
    __try {
        typedef void(*FnT)(void*, void*, int);
        ((FnT)fn)(L, nullptr, 0);
        return 1;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return 0;
    }
}

static void* _seh_newthread(void* fn, void* L) {
    __try {
        typedef void*(*FnT)(void*);
        return ((FnT)fn)(L);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

// Lua execution engine.
// Gets lua_State from ScriptContext, sets identity=8, pushes scripts via task scheduler.

namespace LuaEngine {

    inline std::string g_workspaceFolder = ".\\workspace";

    // ── RBX ExtraSpace — sits directly before lua_State in memory ────────────
    // identity at +0x40 within ExtraSpace (size 0x48 → lua_State starts after)
    struct RBXExtraSpace {
        uintptr_t sharedState;    // +0x00
        BYTE      _pad[0x38];
        int       identity;       // +0x40 — set to 8 for full permissions
        int       _pad2;
        uintptr_t _reserved;      // +0x48
    };

    using lua_State = void;
    using FnLuaDCall       = void(*)(lua_State*, void*, int);
    using FnLuaLLoadBuffer = int (*)(lua_State*, const char*, size_t, const char*);
    using FnLuaENewThread  = lua_State*(*)(lua_State*);

    // ── Thread-safe script queue ─────────────────────────────────────────────
    struct ScriptEntry { std::string source; };
    static CRITICAL_SECTION g_cs{};
    static std::vector<ScriptEntry> g_queue{};
    static bool g_csInit = false;
    static lua_State* g_L = nullptr;

    inline void PushScript(std::string src) {
        if (!g_csInit) return;
        EnterCriticalSection(&g_cs);
        g_queue.push_back({ std::move(src) });
        LeaveCriticalSection(&g_cs);
    }

    // ── Set lua thread identity to level 8 ───────────────────────────────────
    inline void SetIdentity(lua_State* L, int level = 8) {
        // ExtraSpace is immediately before the lua_State pointer
        auto es = reinterpret_cast<RBXExtraSpace*>(
            reinterpret_cast<uintptr_t>(L) - sizeof(uintptr_t) - 0x48 + 8);
        // Simpler: identity is at lua_State - 0x10 in most RBX builds.
        // Exact layout: *((int*)(L - 0x10)) = identity
        *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(L) - 0x10) = level;
    }

    // ── Execute a script chunk on the given state ────────────────────────────
    inline bool Execute(lua_State* L, const std::string& source) {
        if (!LuaOffsets::rLuaL_loadbuffer || !LuaOffsets::rLuaD_call) return false;

        SetIdentity(L, 8);

        auto loadbuf = reinterpret_cast<FnLuaLLoadBuffer>(LuaOffsets::rLuaL_loadbuffer);
        auto dcall   = reinterpret_cast<FnLuaDCall>(LuaOffsets::rLuaD_call);

        int r = loadbuf(L, source.c_str(), source.size(), "@exec");
        if (r != 0) {
            OutputDebugStringA("[LuaEngine] luaL_loadbuffer failed");
            return false;
        }

        if (!_seh_dcall(reinterpret_cast<void*>(dcall), L)) {
            OutputDebugStringA("[LuaEngine] SEH caught during dcall");
            return false;
        }
        return true;
    }

    // ── Obtain lua_State from ScriptContext ───────────────────────────────────
    // ScriptContext stores the main Lua state at a known offset.
    // We read it after the game has loaded.
    inline lua_State* GetLuaState() {
        uintptr_t sc = Runtime::GetScriptContext();
        if (!sc) return nullptr;

        // ScriptContext → lua_State is typically at +0x110 in this version.
        // Walk: SC + 0x110 → pointer to lua_State
        // Adjust if incorrect — use x64dbg to verify:
        //   1. Find ScriptContext pointer via DataModel + 0x440
        //   2. At SC + 0x110, you should see a valid heap address
        //   3. That address is the global lua_State (L)
        constexpr uintptr_t SC_TO_LUASTATE = 0x110;
        uintptr_t L = *reinterpret_cast<uintptr_t*>(sc + SC_TO_LUASTATE);
        if (!L) return nullptr;
        return reinterpret_cast<lua_State*>(L);
    }

    // ── Ticker — runs on a background thread, drains the queue ───────────────
    // Calls into Roblox's task scheduler step callback.
    // We don't hook the scheduler; instead we just run scripts on our own
    // timer thread using the lua_State we captured at init.
    static DWORD WINAPI TickerThread(LPVOID) {
        // Wait for the game to be fully loaded
        uintptr_t base = Runtime::ModuleBase();
        while (true) {
            uintptr_t dm = Runtime::GetDataModel();
            if (dm) {
                // Check GameLoaded flag
                BYTE loaded = *reinterpret_cast<BYTE*>(dm + offsets::GameLoaded);
                if (loaded) break;
            }
            Sleep(500);
        }

        // Grab lua_State (retry until ScriptContext is ready)
        while (!g_L) {
            g_L = GetLuaState();
            Sleep(200);
        }
        OutputDebugStringA("[LuaEngine] Got lua_State, engine ready");

        while (true) {
            Sleep(16); // ~60 Hz drain

            EnterCriticalSection(&g_cs);
            auto snapshot = std::move(g_queue);
            LeaveCriticalSection(&g_cs);

            for (auto& entry : snapshot) {
                // Spawn a new coroutine per script for isolation
                lua_State* co = g_L;
                if (LuaOffsets::rLuaE_newthread) {
                    void* result = _seh_newthread(LuaOffsets::rLuaE_newthread, g_L);
                    if (result) co = result;
                }
                Execute(co ? co : g_L, entry.source);
            }
        }
        return 0;
    }

    // ── Init ─────────────────────────────────────────────────────────────────
    inline bool Init() {
        InitializeCriticalSection(&g_cs);
        g_csInit = true;

        // Resolve Lua function pointers via AoB scan
        if (!LuaOffsets::Init()) {
            OutputDebugStringA("[LuaEngine] WARNING: some Lua patterns not found");
            // Don't fail — partial function set may still work
        }

        // Start ticker thread
        HANDLE h = CreateThread(nullptr, 0, TickerThread, nullptr, 0, nullptr);
        if (h) CloseHandle(h);

        return true;
    }

} // namespace LuaEngine