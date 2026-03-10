#include <lua.hpp>
#include <string>

namespace LuaUtils {
    std::string getRelativeToBasePath(lua_State* L, const std::string& path) {
        lua_getfield(L, LUA_REGISTRYINDEX, "basePath");
        std::string basePath;
        if (lua_isstring(L, -1)) {
            basePath = lua_tostring(L, -1);
        } else {
            basePath = "/";
        }
        lua_pop(L, 1);
        
        std::string relPath = path;
        if (!basePath.empty() && basePath.back() != '/') {
            basePath += "/";
        }
        if (!relPath.empty() && relPath.front() != '/') {
            relPath = basePath + relPath;
        }
        return relPath;
    }
}