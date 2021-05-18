#include <libgui.h>
#include <gccore.h>
#include <lua.hpp>
#include "main.h"
#include "luasupport.h"
#include "pms2.h"

static int lua_PMS2_getUpdateProgress(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, PMS2::getUpdateProgress());

    return 1;
}

static int lua_PMS2_isUpdating(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushboolean(L, PMS2::isUpdating());

    return 1;
}

static int lua_PMS2_hasUpdateSucceeded(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushboolean(L, PMS2::hasUpdateSucceeded());

    return 1;
}

static int lua_PMS2_startUpdate(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::startUpdate(luaL_checkstring(L, 1));

    return 0;
}


static int lua_PMS2_isConnected(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushboolean(L, PMS2::isConnected());

    return 1;
}

static int lua_PMS2_getVer(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushnumber(L, PMS2::getVer());

    return 1;
}

static int lua_PMS2_getConf0(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, PMS2::getConf0());

    return 1;
}

static int lua_PMS2_getChargeCurrent(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, PMS2::getChargeCurrent());

    return 1;
}

static int lua_PMS2_getTermCurrent(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, PMS2::getTermCurrent());

    return 1;
}

static int lua_PMS2_getPreChargeCurrent(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, PMS2::getPreChargeCurrent());

    return 1;
}

static int lua_PMS2_getChargeVoltage(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, PMS2::getChargeVoltage());

    return 1;
}

static int lua_PMS2_getTREG(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, PMS2::getTREG());

    return 1;
}

static int lua_PMS2_getChargeStatus(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, PMS2::getChargeStatus());

    return 1;
}

static int lua_PMS2_getBatDesignCapacity(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, PMS2::getBatDesignCapacity());

    return 1;
}

static int lua_PMS2_getSOC(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushnumber(L, PMS2::getSOC());

    return 1;
}

static int lua_PMS2_getSOCRaw(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, PMS2::getSOCRaw());

    return 1;
}

static int lua_PMS2_getVCell(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushnumber(L, PMS2::getVCell());

    return 1;
}

static int lua_PMS2_getVCellRaw(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, PMS2::getVCellRaw());

    return 1;
}

static int lua_PMS2_getCurrent(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushnumber(L, PMS2::getCurrent());

    return 1;
}

static int lua_PMS2_getCurrentRaw(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, PMS2::getCurrentRaw());

    return 1;
}

static int lua_PMS2_getTTE(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, PMS2::getTTE());

    return 1;
}

static int lua_PMS2_getTTF(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushinteger(L, PMS2::getTTF());

    return 1;
}

static int lua_PMS2_setConf0(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::setConf0(luaL_checkinteger(L, 1));

    return 0;
}

static int lua_PMS2_setChargeCurrent(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::setChargeCurrent(luaL_checkinteger(L, 1));

    return 0;
}

static int lua_PMS2_setTermCurrent(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::setTermCurrent(luaL_checkinteger(L, 1));

    return 0;
}

static int lua_PMS2_setPreChargeCurrent(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::setPreChargeCurrent(luaL_checkinteger(L, 1));

    return 0;
}

static int lua_PMS2_setChargeVoltage(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::setChargeVoltage(luaL_checkinteger(L, 1));

    return 0;
}

static int lua_PMS2_setTREG(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::setTREG(luaL_checkinteger(L, 1));

    return 0;
}

static int lua_PMS2_setBatDesignCapacity(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::setBatDesignCapacity(luaL_checkinteger(L, 1));

    return 0;
}

static int lua_PMS2_flashConfig(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::flashConfig();

    return 0;
}

static int lua_PMS2_reconfigureMAX(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::reconfigureMAX();

    return 0;
}

static int lua_PMS2_getNTC(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    lua_pushnumber(L, PMS2::NTCToCelsius(PMS2::getNTC()));

    return 1;
}

static int lua_PMS2_setFanSpeed(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::setFanSpeed(luaL_checkinteger(L, 1));

    return 0;
}

static int lua_PMS2_freeFan(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 0) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::freeFan();

    return 0;
}

static int lua_PMS2_setFanPIDkP(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::setFanPIDkP(luaL_checknumber(L, 1));

    return 0;
}

static int lua_PMS2_setFanPIDkI(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::setFanPIDkI(luaL_checknumber(L, 1));

    return 0;
}

static int lua_PMS2_setFanPIDkD(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::setFanPIDkD(luaL_checknumber(L, 1));

    return 0;
}

static int lua_PMS2_setFanPIDTarget(lua_State* L) {
    int argc = lua_gettop(L);
    if (argc != 1) {
        return luaL_error(L, "wrong number of arguments");
    }

    PMS2::setFanPIDTarget(luaL_checknumber(L, 1));

    return 0;
}

static const luaL_Reg PMS2_functions[] = {
    {"getUpdateProgress", lua_PMS2_getUpdateProgress},
    {"isUpdating", lua_PMS2_isUpdating},
    {"startUpdate", lua_PMS2_startUpdate},
    {"isConnected", lua_PMS2_isConnected},
    {"hasUpdateSucceeded", lua_PMS2_hasUpdateSucceeded},
    {"getVer", lua_PMS2_getVer},
    {"getConf0", lua_PMS2_getConf0},
    {"getChargeCurrent", lua_PMS2_getChargeCurrent},
    {"getTermCurrent", lua_PMS2_getTermCurrent},
    {"getPreChargeCurrent", lua_PMS2_getPreChargeCurrent},
    {"getChargeVoltage", lua_PMS2_getChargeVoltage},
    {"getTREG", lua_PMS2_getTREG},
    {"getChargeStatus", lua_PMS2_getChargeStatus},
    {"getBatDesignCapacity", lua_PMS2_getBatDesignCapacity},
    {"getSOC", lua_PMS2_getSOC},
    {"getSOCRaw", lua_PMS2_getSOCRaw},
    {"getVCell", lua_PMS2_getVCell},
    {"getVCellRaw", lua_PMS2_getVCellRaw},
    {"getCurrent", lua_PMS2_getCurrent},
    {"getCurrentRaw", lua_PMS2_getCurrentRaw},
    {"getTTE", lua_PMS2_getTTE},
    {"getTTF", lua_PMS2_getTTF},
    {"setConf0", lua_PMS2_setConf0},
    {"setChargeCurrent", lua_PMS2_setChargeCurrent},
    {"setTermCurrent", lua_PMS2_setTermCurrent},
    {"setPreChargeCurrent", lua_PMS2_setPreChargeCurrent},
    {"setChargeVoltage", lua_PMS2_setChargeVoltage},
    {"setTREG", lua_PMS2_setTREG},
    {"setBatDesignCapacity", lua_PMS2_setBatDesignCapacity},
    {"flashConfig", lua_PMS2_flashConfig},
    {"reconfigureMAX", lua_PMS2_reconfigureMAX},
    {"getNTC", lua_PMS2_getNTC},
    {"setFanSpeed", lua_PMS2_setFanSpeed},
    {"freeFan", lua_PMS2_freeFan},
    {"setFanPIDkP", lua_PMS2_setFanPIDkP},
    {"setFanPIDkI", lua_PMS2_setFanPIDkI},
    {"setFanPIDkD", lua_PMS2_setFanPIDkD},
    {"setFanPIDTarget", lua_PMS2_setFanPIDTarget},
    {NULL, NULL}
};

void luaRegisterPMS2Lib(lua_State* L) {
    lua_newtable(L);
    luaL_setfuncs(L, PMS2_functions, 0);
    luaSetTableIntField(L, "PWR_BTN_MOMEN", 0x00);
    luaSetTableIntField(L, "PWR_BTN_TOGGLE", 0x01);
    luaSetTableIntField(L, "PWR_BTN_TYPE", 0x01);
    luaSetTableIntField(L, "PWR_BTN_ACTLOW", 0x00);
    luaSetTableIntField(L, "PWR_BTN_ACTHI", 0x02);
    luaSetTableIntField(L, "PWR_BTN_POLARITY", 0x02);
    luaSetTableIntField(L, "STAT_LED_STD", 0x00);
    luaSetTableIntField(L, "STAT_LED_WS", 0x04);
    luaSetTableIntField(L, "STAT_LED_TYPE", 0x04);
    luaSetTableIntField(L, "CHG_STAT_NOT_CHG", CHG_STAT_NOT_CHG);
    luaSetTableIntField(L, "CHG_STAT_PRE_CHG", CHG_STAT_PRE_CHG);
    luaSetTableIntField(L, "CHG_STAT_FAST_CHG", CHG_STAT_FAST_CHG);
    luaSetTableIntField(L, "CHG_STAT_DONE", CHG_STAT_DONE);
    lua_setglobal(L, "PMS2");
}
