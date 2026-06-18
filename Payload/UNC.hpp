#pragma once
// Unified Naming Convention (UNC) function implementations.
// These are registered as Lua globals when the DLL initialises.
// Full UNC spec: https://github.com/unified-naming-convention/NamingStandard

// Lua C API shims (RBX uses a modified Lua 5.1 / Luau)
extern "C" {
    using lua_State = void;
    using lua_CFunction = int(*)(lua_State*);

    // These are resolved from Offsets at init time
    int  rbx_lua_type(lua_State*, int);
    const char* rbx_lua_typename(lua_State*, int);
    void rbx_lua_pushstring(lua_State*, const char*);
    void rbx_lua_pushnumber(lua_State*, double);
    void rbx_lua_pushboolean(lua_State*, int);
    void rbx_lua_pushnil(lua_State*);
    void rbx_lua_pushlightuserdata(lua_State*, void*);
    const char* rbx_lua_tostring(lua_State*, int);
    double rbx_lua_tonumber(lua_State*, int);
    int  rbx_lua_toboolean(lua_State*, int);
    int  rbx_lua_gettop(lua_State*);
    void rbx_lua_settop(lua_State*, int);
    void rbx_lua_getglobal(lua_State*, const char*);
    void rbx_lua_setglobal(lua_State*, const char*);
    void rbx_lua_createtable(lua_State*, int, int);
    void rbx_lua_settable(lua_State*, int);
    void rbx_lua_gettable(lua_State*, int);
    void rbx_lua_rawset(lua_State*, int);
    void rbx_lua_rawget(lua_State*, int);
    void rbx_lua_pushvalue(lua_State*, int);
    void rbx_lua_pop(lua_State*, int);
    void rbx_lua_register(lua_State*, const char*, lua_CFunction);
    int  rbx_lua_pcall(lua_State*, int, int, int);
    void rbx_lua_error(lua_State*);
    int  rbx_luaL_error(lua_State*, const char*, ...);
    int  rbx_luaL_checkstring(lua_State*, int);
    // ... (all bound at runtime via pointer table)
}

#include <Windows.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace UNC {

    namespace fs = std::filesystem;

    // Workspace folder — scripts read/write here
    static std::string g_workspace = ".\\workspace";

    // ── Helpers ──────────────────────────────────────────────────────────────

    static std::string SafePath(const std::string& rel) {
        auto base = fs::absolute(g_workspace);
        auto full = fs::weakly_canonical(base / rel);
        // Prevent directory traversal
        auto baseStr = base.string();
        auto fullStr = full.string();
        if (fullStr.substr(0, baseStr.size()) != baseStr)
            return ""; // blocked
        return fullStr;
    }

    // ── File I/O ─────────────────────────────────────────────────────────────

    static int unc_readfile(lua_State* L) {
        auto path = SafePath(rbx_lua_tostring(L, 1));
        if (path.empty()) return rbx_luaL_error(L, "Invalid path");

        std::ifstream f(path, std::ios::binary);
        if (!f) return rbx_luaL_error(L, "File not found");

        std::ostringstream ss; ss << f.rdbuf();
        auto content = ss.str();
        rbx_lua_pushstring(L, content.c_str());
        return 1;
    }

    static int unc_writefile(lua_State* L) {
        auto path    = SafePath(rbx_lua_tostring(L, 1));
        auto content = std::string(rbx_lua_tostring(L, 2));
        if (path.empty()) return rbx_luaL_error(L, "Invalid path");

        fs::create_directories(fs::path(path).parent_path());
        std::ofstream f(path, std::ios::binary);
        f.write(content.data(), content.size());
        return 0;
    }

    static int unc_appendfile(lua_State* L) {
        auto path    = SafePath(rbx_lua_tostring(L, 1));
        auto content = std::string(rbx_lua_tostring(L, 2));
        if (path.empty()) return rbx_luaL_error(L, "Invalid path");

        std::ofstream f(path, std::ios::binary | std::ios::app);
        f.write(content.data(), content.size());
        return 0;
    }

    static int unc_loadfile(lua_State* L) {
        auto path = SafePath(rbx_lua_tostring(L, 1));
        if (path.empty()) return rbx_luaL_error(L, "Invalid path");

        std::ifstream f(path, std::ios::binary);
        if (!f) { rbx_lua_pushnil(L); rbx_lua_pushstring(L, "File not found"); return 2; }

        std::ostringstream ss; ss << f.rdbuf();
        rbx_lua_pushstring(L, ss.str().c_str());
        return 1;
    }

    static int unc_isfile(lua_State* L) {
        auto path = SafePath(rbx_lua_tostring(L, 1));
        rbx_lua_pushboolean(L, !path.empty() && fs::is_regular_file(path));
        return 1;
    }

    static int unc_isfolder(lua_State* L) {
        auto path = SafePath(rbx_lua_tostring(L, 1));
        rbx_lua_pushboolean(L, !path.empty() && fs::is_directory(path));
        return 1;
    }

    static int unc_makefolder(lua_State* L) {
        auto path = SafePath(rbx_lua_tostring(L, 1));
        if (!path.empty()) fs::create_directories(path);
        return 0;
    }

    static int unc_delfolder(lua_State* L) {
        auto path = SafePath(rbx_lua_tostring(L, 1));
        if (!path.empty() && fs::exists(path)) fs::remove_all(path);
        return 0;
    }

    static int unc_delfile(lua_State* L) {
        auto path = SafePath(rbx_lua_tostring(L, 1));
        if (!path.empty() && fs::exists(path)) fs::remove(path);
        return 0;
    }

    static int unc_listfiles(lua_State* L) {
        auto path = SafePath(rbx_lua_tostring(L, 1));
        rbx_lua_createtable(L, 0, 0);
        if (path.empty() || !fs::is_directory(path)) return 1;

        int idx = 1;
        for (auto& e : fs::directory_iterator(path)) {
            rbx_lua_pushnumber(L, idx++);
            rbx_lua_pushstring(L, e.path().filename().string().c_str());
            rbx_lua_settable(L, -3);
        }
        return 1;
    }

    // ── HTTP request ─────────────────────────────────────────────────────────
    // Uses WinHTTP for an undetectable request path (no Roblox HTTP service)

    static int unc_request(lua_State* L) {
        // Simplified — expects table { Url, Method, Headers, Body }
        // Full implementation uses WinHttpOpen / WinHttpConnect / WinHttpSendRequest
        rbx_lua_pushnil(L);
        rbx_lua_pushstring(L, "http.request: WinHTTP implementation required");
        return 2;
    }

    // ── Identity / environment ────────────────────────────────────────────────

    static int unc_getidentity(lua_State* L) {
        // Read from ExtraSpace
        auto es = reinterpret_cast<BYTE*>(L) - 0x48;
        int identity = *reinterpret_cast<int*>(es + 0x40);
        rbx_lua_pushnumber(L, static_cast<double>(identity));
        return 1;
    }

    static int unc_setidentity(lua_State* L) {
        int level = static_cast<int>(rbx_lua_tonumber(L, 1));
        if (level < 0 || level > 8) return rbx_luaL_error(L, "Identity out of range");
        auto es = reinterpret_cast<BYTE*>(L) - 0x48;
        *reinterpret_cast<int*>(es + 0x40) = level;
        return 0;
    }

    // ── Misc UNC ─────────────────────────────────────────────────────────────

    static int unc_identifyexecutor(lua_State* L) {
        rbx_lua_pushstring(L, "PrivateExec");
        rbx_lua_pushstring(L, "1.0.0");
        return 2;
    }

    static int unc_gethwid(lua_State* L) {
        // Volume serial number as a simple HWID
        DWORD serial = 0;
        GetVolumeInformationW(L"C:\\", nullptr, 0, &serial, nullptr, nullptr, nullptr, 0);
        char buf[32]; sprintf_s(buf, "%08X", serial);
        rbx_lua_pushstring(L, buf);
        return 1;
    }

    static int unc_getexecutorname(lua_State* L) {
        rbx_lua_pushstring(L, "PrivateExec");
        return 1;
    }

    // ── Register all UNC globals ──────────────────────────────────────────────

    inline void Register(lua_State* L) {
        struct { const char* name; lua_CFunction fn; } funcs[] = {
            // File I/O
            { "readfile",          unc_readfile         },
            { "writefile",         unc_writefile        },
            { "appendfile",        unc_appendfile       },
            { "loadfile",          unc_loadfile         },
            { "isfile",            unc_isfile           },
            { "isfolder",          unc_isfolder         },
            { "makefolder",        unc_makefolder       },
            { "delfolder",         unc_delfolder        },
            { "delfile",           unc_delfile          },
            { "listfiles",         unc_listfiles        },
            // HTTP
            { "request",           unc_request          },
            // Identity
            { "getidentity",       unc_getidentity      },
            { "setidentity",       unc_setidentity      },
            // Misc
            { "identifyexecutor",  unc_identifyexecutor },
            { "gethwid",           unc_gethwid          },
            { "getexecutorname",   unc_getexecutorname  },
        };

        for (auto& f : funcs)
            rbx_lua_register(L, f.name, f.fn);
    }

} // namespace UNC