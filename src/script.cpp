
// ep128emu -- portable Enterprise 128 emulator
// Copyright (C) 2003-2007 Istvan Varga <istvanv@users.sourceforge.net>
// http://sourceforge.net/projects/ep128emu/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "ep128emu.hpp"
#include "vm.hpp"
#include "ep128vm.hpp"
#include "bplist.hpp"
#include "script.hpp"

#ifdef HAVE_LUA_H
extern "C" {
#  include "lua.h"
#  include "lauxlib.h"
#  include "lualib.h"
}
#endif  // HAVE_LUA_H

namespace Ep128Emu {

#ifdef HAVE_LUA_H

  void * LuaScript::allocFunc(void *userData,
                              void *ptr, size_t oldSize, size_t newSize)
  {
    (void) userData;
    (void) oldSize;
    if (newSize == 0) {
      if (ptr)
        std::free(ptr);
      return (void *) 0;
    }
    if (!ptr)
      return std::malloc(newSize);
    return std::realloc(ptr, newSize);
  }

  int LuaScript::luaFunc_AND(lua_State *lst)
  {
    lua_Integer result = (~(lua_Integer(0)));
    int     argCnt = lua_gettop(lst);
    for (int i = 1; i <= argCnt; i++) {
      if (!lua_isnumber(lst, i)) {
        LuaScript&  this_ = *(reinterpret_cast<LuaScript *>(
                                  lua_touserdata(lst, lua_upvalueindex(1))));
        this_.luaError("invalid argument type for AND()");
        return 0;
      }
      result = result & lua_Integer(lua_tointeger(lst, i));
    }
    lua_pushinteger(lst, result);
    return 1;
  }

  int LuaScript::luaFunc_OR(lua_State *lst)
  {
    lua_Integer result = 0;
    int     argCnt = lua_gettop(lst);
    for (int i = 1; i <= argCnt; i++) {
      if (!lua_isnumber(lst, i)) {
        LuaScript&  this_ = *(reinterpret_cast<LuaScript *>(
                                  lua_touserdata(lst, lua_upvalueindex(1))));
        this_.luaError("invalid argument type for OR()");
        return 0;
      }
      result = result | lua_Integer(lua_tointeger(lst, i));
    }
    lua_pushinteger(lst, result);
    return 1;
  }

  int LuaScript::luaFunc_XOR(lua_State *lst)
  {
    lua_Integer result = 0;
    int     argCnt = lua_gettop(lst);
    for (int i = 1; i <= argCnt; i++) {
      if (!lua_isnumber(lst, i)) {
        LuaScript&  this_ = *(reinterpret_cast<LuaScript *>(
                                  lua_touserdata(lst, lua_upvalueindex(1))));
        this_.luaError("invalid argument type for XOR()");
        return 0;
      }
      result = result ^ lua_Integer(lua_tointeger(lst, i));
    }
    lua_pushinteger(lst, result);
    return 1;
  }

  int LuaScript::luaFunc_SHL(lua_State *lst)
  {
    if (lua_gettop(lst) != 2) {
      LuaScript&  this_ = *(reinterpret_cast<LuaScript *>(
                                lua_touserdata(lst, lua_upvalueindex(1))));
      this_.luaError("invalid number of arguments for SHL()");
      return 0;
    }
    if (!(lua_isnumber(lst, 1) && lua_isnumber(lst, 2))) {
      LuaScript&  this_ = *(reinterpret_cast<LuaScript *>(
                                lua_touserdata(lst, lua_upvalueindex(1))));
      this_.luaError("invalid argument type for SHL()");
      return 0;
    }
    lua_Integer result = lua_tointeger(lst, 1);
    lua_Integer n = lua_tointeger(lst, 2);
    if (n > 0) {
      if (n >= lua_Integer(sizeof(lua_Integer) * 4))
        result = 0;
      else
        result = result << int(n);
    }
    else if (n < 0) {
      n = -n;
      if (n >= lua_Integer(sizeof(lua_Integer) * 4))
        result = 0;
      else
        result = result >> int(n);
    }
    lua_pushinteger(lst, result);
    return 1;
  }

  int LuaScript::luaFunc_SHR(lua_State *lst)
  {
    if (lua_gettop(lst) != 2) {
      LuaScript&  this_ = *(reinterpret_cast<LuaScript *>(
                                lua_touserdata(lst, lua_upvalueindex(1))));
      this_.luaError("invalid number of arguments for SHR()");
      return 0;
    }
    if (!(lua_isnumber(lst, 1) && lua_isnumber(lst, 2))) {
      LuaScript&  this_ = *(reinterpret_cast<LuaScript *>(
                                lua_touserdata(lst, lua_upvalueindex(1))));
      this_.luaError("invalid argument type for SHR()");
      return 0;
    }
    lua_Integer result = lua_tointeger(lst, 1);
    lua_Integer n = lua_tointeger(lst, 2);
    if (n > 0) {
      if (n >= lua_Integer(sizeof(lua_Integer) * 4))
        result = 0;
      else
        result = result >> int(n);
    }
    else if (n < 0) {
      n = -n;
      if (n >= lua_Integer(sizeof(lua_Integer) * 4))
        result = 0;
      else
        result = result << int(n);
    }
    lua_pushinteger(lst, result);
    return 1;
  }

  int LuaScript::luaFunc_setBreakPoint(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 3) {
      this_.luaError("invalid number of arguments for setBreakPoint()");
      return 0;
    }
    if (!(lua_isnumber(lst, 1) &&
          lua_isnumber(lst, 2) &&
          lua_isnumber(lst, 3))) {
      this_.luaError("invalid argument type for setBreakPoint()");
      return 0;
    }
    try {
      BreakPointList  bpList;
      int       bpType = int(lua_tointeger(lst, 1));
      uint32_t  bpAddr = uint32_t(lua_tointeger(lst, 2));
      int       bpPriority = int(lua_tointeger(lst, 3));
      if ((bpType & (~(int(27)))) == 0) {
        bool    readFlag = ((bpType & 3) != 2);
        bool    writeFlag = ((bpType & 3) != 1);
        bool    addr22BitFlag = bool(bpType & 8) || (bpAddr >= 0x00010000U);
        bool    ignoreFlag = bool(bpType & 16);
        if (!addr22BitFlag) {
          bpList.addMemoryBreakPoint(uint16_t(bpAddr & 0xFFFFU),
                                     readFlag, writeFlag, ignoreFlag,
                                     bpPriority);
        }
        else {
          bpList.addMemoryBreakPoint(uint8_t((bpAddr >> 14) & 0xFFU),
                                     uint16_t(bpAddr & 0x3FFFU),
                                     readFlag, writeFlag, ignoreFlag,
                                     bpPriority);
        }
      }
      else if (bpType >= 5 && bpType <= 7) {
        bpList.addIOBreakPoint(uint16_t(bpAddr & 0xFFU),
                               bool(bpType & 1), bool(bpType & 2), bpPriority);
      }
      else {
        throw Exception("invalid breakpoint type");
      }
      this_.vm.setBreakPoints(bpList);
    }
    catch (std::exception& e) {
      this_.luaError(e.what());
      return 0;
    }
    return 0;
  }

  int LuaScript::luaFunc_clearBreakPoints(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 0) {
      this_.luaError("invalid number of arguments for clearBreakPoints()");
      return 0;
    }
    this_.vm.clearBreakPoints();
    return 0;
  }

  int LuaScript::luaFunc_getMemoryPage(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for getMemoryPage()");
      return 0;
    }
    if (!lua_isnumber(lst, 1)) {
      this_.luaError("invalid argument type for getMemoryPage()");
      return 0;
    }
    int     n = this_.vm.getMemoryPage(int(lua_tointeger(lst, 1) & 3));
    lua_pushinteger(lst, lua_Integer(n));
    return 1;
  }

  int LuaScript::luaFunc_readMemory(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for readMemory()");
      return 0;
    }
    if (!lua_isnumber(lst, 1)) {
      this_.luaError("invalid argument type for readMemory()");
      return 0;
    }
    uint8_t n =
        this_.vm.readMemory(uint32_t(lua_tointeger(lst, 1) & 0xFFFF), true);
    lua_pushinteger(lst, lua_Integer(n));
    return 1;
  }

  int LuaScript::luaFunc_writeMemory(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 2) {
      this_.luaError("invalid number of arguments for writeMemory()");
      return 0;
    }
    if (!(lua_isnumber(lst, 1) && lua_isnumber(lst, 2))) {
      this_.luaError("invalid argument type for writeMemory()");
      return 0;
    }
    this_.vm.writeMemory(uint32_t(lua_tointeger(lst, 1) & 0xFFFF),
                         uint8_t(lua_tointeger(lst, 2) & 0xFF), true);
    return 0;
  }

  int LuaScript::luaFunc_readMemoryRaw(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for readMemoryRaw()");
      return 0;
    }
    if (!lua_isnumber(lst, 1)) {
      this_.luaError("invalid argument type for readMemoryRaw()");
      return 0;
    }
    uint8_t n =
        this_.vm.readMemory(uint32_t(lua_tointeger(lst, 1) & 0x3FFFFF), false);
    lua_pushinteger(lst, lua_Integer(n));
    return 1;
  }

  int LuaScript::luaFunc_writeMemoryRaw(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 2) {
      this_.luaError("invalid number of arguments for writeMemoryRaw()");
      return 0;
    }
    if (!(lua_isnumber(lst, 1) && lua_isnumber(lst, 2))) {
      this_.luaError("invalid argument type for writeMemoryRaw()");
      return 0;
    }
    this_.vm.writeMemory(uint32_t(lua_tointeger(lst, 1) & 0x3FFFFF),
                         uint8_t(lua_tointeger(lst, 2) & 0xFF), false);
    return 0;
  }

  // TODO: implement these
#if 0
  int LuaScript::luaFunc_getPC(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 0) {
      this_.luaError("invalid number of arguments for getPC()");
      return 0;
    }
    lua_pushinteger(lst, lua_Integer(this_.vm.getProgramCounter()));
    return 1;
  }

  int LuaScript::luaFunc_setPC(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for setPC()");
      return 0;
    }
    if (!lua_isnumber(lst, 1)) {
      this_.luaError("invalid argument type for setPC()");
      return 0;
    }
    Ep128::Ep128VM& p4vm = *(reinterpret_cast<Ep128::Ep128VM *>(&(this_.vm)));
    Ep128::M7501Registers r;
    p4vm.getCPURegisters(r);
    r.reg_PC = uint16_t(lua_tointeger(lst, 1) & 0xFFFF);
    p4vm.setCPURegisters(r);
    return 0;
  }

  int LuaScript::luaFunc_loadMemory(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for loadProgram()");
      return 0;
    }
    if (!lua_isstring(lst, 1)) {
      this_.luaError("invalid argument type for loadProgram()");
      return 0;
    }
    try {
      Ep128::Ep128VM& p4vm = *(reinterpret_cast<Ep128::Ep128VM *>(&(this_.vm)));
      p4vm.loadProgram(lua_tolstring(lst, 1, (size_t *) 0));
    }
    catch (std::exception& e) {
      this_.luaError(e.what());
      return 0;
    }
    return 0;
  }

  int LuaScript::luaFunc_saveMemory(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    if (lua_gettop(lst) != 1) {
      this_.luaError("invalid number of arguments for saveProgram()");
      return 0;
    }
    if (!lua_isstring(lst, 1)) {
      this_.luaError("invalid argument type for saveProgram()");
      return 0;
    }
    try {
      Ep128::Ep128VM& p4vm = *(reinterpret_cast<Ep128::Ep128VM *>(&(this_.vm)));
      p4vm.saveProgram(lua_tolstring(lst, 1, (size_t *) 0));
    }
    catch (std::exception& e) {
      this_.luaError(e.what());
      return 0;
    }
    return 0;
  }
#endif

  int LuaScript::luaFunc_mprint(lua_State *lst)
  {
    LuaScript&  this_ =
        *(reinterpret_cast<LuaScript *>(lua_touserdata(lst,
                                                       lua_upvalueindex(1))));
    try {
      std::string buf;
      int     argCnt = lua_gettop(lst);
      for (int i = 1; i <= argCnt; i++) {
        if (!lua_isstring(lst, i))
          throw Exception("invalid argument type for mprint()");
        const char  *s = lua_tolstring(lst, i, (size_t *) 0);
        if (s)
          buf += s;
      }
      this_.messageCallback(buf.c_str());
    }
    catch (std::exception& e) {
      this_.luaError(e.what());
      return 0;
    }
    return 0;
  }

#endif  // HAVE_LUA_H

  // --------------------------------------------------------------------------

  LuaScript::LuaScript(VirtualMachine& vm_)
    : vm(vm_),
      luaState((lua_State *) 0),
      errorMessage((char *) 0),
      haveBreakPointCallback(false)
  {
  }

  LuaScript::~LuaScript()
  {
#ifdef HAVE_LUA_H
    if (luaState)
      lua_close(luaState);
#endif  // HAVE_LUA_H
  }

#ifdef HAVE_LUA_H

  void LuaScript::registerLuaFunction(lua_CFunction f, const char *name)
  {
    lua_pushlightuserdata(luaState, (void *) this);
    lua_pushcclosure(luaState, f, 1);
    lua_setfield(luaState, LUA_GLOBALSINDEX, name);
  }

  bool LuaScript::runBreakPointCallback_(int type, uint16_t addr, uint8_t value)
  {
    lua_pushvalue(luaState, -1);
    lua_pushinteger(luaState, lua_Integer(type));
    lua_pushinteger(luaState, lua_Integer(addr));
    lua_pushinteger(luaState, lua_Integer(value));
    int     err = lua_pcall(luaState, 3, 1, 0);
    if (err != 0) {
      closeScript();
      if (errorMessage) {
        const char  *msg = errorMessage;
        errorMessage = (char *) 0;
        errorCallback(msg);
      }
      else if (err == LUA_ERRRUN)
        errorCallback("runtime error while running Lua script");
      else if (err == LUA_ERRMEM)
        errorCallback("memory allocation failure while running Lua script");
      else if (err == LUA_ERRERR)
        errorCallback("error while running Lua error handler");
      else
        errorCallback("error running Lua script");
      return true;
    }
    if (!lua_isboolean(luaState, -1)) {
      closeScript();
      errorCallback("Lua breakpoint function should return a boolean");
      return true;
    }
    bool    retval = bool(lua_toboolean(luaState, -1));
    lua_pop(luaState, 1);
    return retval;
  }

  void LuaScript::luaError(const char *msg)
  {
    if (!msg)
      msg = "unknown error in Lua script";
    errorMessage = msg;
    lua_pushstring(luaState, msg);
    lua_error(luaState);
  }

#endif  // HAVE_LUA_H

  void LuaScript::loadScript(const char *s)
  {
    closeScript();
    if (s == (char *) 0 || s[0] == '\0')
      return;
#ifdef HAVE_LUA_H
    luaState = lua_newstate(&allocFunc, (void *) this);
    if (!luaState) {
      errorCallback("error allocating Lua state");
      return;
    }
    luaL_openlibs(luaState);
    int     err = luaL_loadstring(luaState, s);
    if (err != 0) {
      closeScript();
      if (err == LUA_ERRSYNTAX)
        errorCallback("syntax error in Lua script");
      else if (err == LUA_ERRMEM)
        errorCallback("memory allocation failure while loading Lua script");
      else
        errorCallback("error loading Lua script");
      return;
    }
    registerLuaFunction(&luaFunc_AND, "AND");
    registerLuaFunction(&luaFunc_OR, "OR");
    registerLuaFunction(&luaFunc_XOR, "XOR");
    registerLuaFunction(&luaFunc_SHL, "SHL");
    registerLuaFunction(&luaFunc_SHR, "SHR");
    registerLuaFunction(&luaFunc_setBreakPoint, "setBreakPoint");
    registerLuaFunction(&luaFunc_clearBreakPoints, "clearBreakPoints");
    registerLuaFunction(&luaFunc_getMemoryPage, "getMemoryPage");
    registerLuaFunction(&luaFunc_readMemory, "readMemory");
    registerLuaFunction(&luaFunc_writeMemory, "writeMemory");
    registerLuaFunction(&luaFunc_readMemoryRaw, "readMemoryRaw");
    registerLuaFunction(&luaFunc_writeMemoryRaw, "writeMemoryRaw");
    // TODO: implement these
#if 0
    registerLuaFunction(&luaFunc_getPC, "getPC");
    registerLuaFunction(&luaFunc_getA, "getA");
    registerLuaFunction(&luaFunc_getF, "getF");
    registerLuaFunction(&luaFunc_getAF, "getAF");
    registerLuaFunction(&luaFunc_getB, "getB");
    registerLuaFunction(&luaFunc_getC, "getC");
    registerLuaFunction(&luaFunc_getBC, "getBC");
    registerLuaFunction(&luaFunc_getD, "getD");
    registerLuaFunction(&luaFunc_getE, "getE");
    registerLuaFunction(&luaFunc_getDE, "getDE");
    registerLuaFunction(&luaFunc_getH, "getH");
    registerLuaFunction(&luaFunc_getL, "getL");
    registerLuaFunction(&luaFunc_getHL, "getHL");
    registerLuaFunction(&luaFunc_getAF_, "getAF_");
    registerLuaFunction(&luaFunc_getBC_, "getBC_");
    registerLuaFunction(&luaFunc_getDE_, "getDE_");
    registerLuaFunction(&luaFunc_getHL_, "getHL_");
    registerLuaFunction(&luaFunc_getSP, "getSP");
    registerLuaFunction(&luaFunc_getIX, "getIX");
    registerLuaFunction(&luaFunc_getIY, "getIY");
    registerLuaFunction(&luaFunc_getI, "getI");
    registerLuaFunction(&luaFunc_getR, "getR");
    registerLuaFunction(&luaFunc_setPC, "setPC");
    registerLuaFunction(&luaFunc_setA, "setA");
    registerLuaFunction(&luaFunc_setF, "setF");
    registerLuaFunction(&luaFunc_setAF, "setAF");
    registerLuaFunction(&luaFunc_setB, "setB");
    registerLuaFunction(&luaFunc_setC, "setC");
    registerLuaFunction(&luaFunc_setBC, "setBC");
    registerLuaFunction(&luaFunc_setD, "setD");
    registerLuaFunction(&luaFunc_setE, "setE");
    registerLuaFunction(&luaFunc_setDE, "setDE");
    registerLuaFunction(&luaFunc_setH, "setH");
    registerLuaFunction(&luaFunc_setL, "setL");
    registerLuaFunction(&luaFunc_setHL, "setHL");
    registerLuaFunction(&luaFunc_setAF_, "setAF_");
    registerLuaFunction(&luaFunc_setBC_, "setBC_");
    registerLuaFunction(&luaFunc_setDE_, "setDE_");
    registerLuaFunction(&luaFunc_setHL_, "setHL_");
    registerLuaFunction(&luaFunc_setSP, "setSP");
    registerLuaFunction(&luaFunc_setIX, "setIX");
    registerLuaFunction(&luaFunc_setIY, "setIY");
    registerLuaFunction(&luaFunc_setI, "setI");
    registerLuaFunction(&luaFunc_setR, "setR");
    registerLuaFunction(&luaFunc_loadProgram, "loadMemory");
    registerLuaFunction(&luaFunc_saveProgram, "saveMemory");
#endif
    registerLuaFunction(&luaFunc_mprint, "mprint");
    err = lua_pcall(luaState, 0, 0, 0);
    if (err != 0) {
      closeScript();
      if (errorMessage) {
        const char  *msg = errorMessage;
        errorMessage = (char *) 0;
        errorCallback(msg);
      }
      else if (err == LUA_ERRRUN)
        errorCallback("runtime error while running Lua script");
      else if (err == LUA_ERRMEM)
        errorCallback("memory allocation failure while running Lua script");
      else if (err == LUA_ERRERR)
        errorCallback("error while running Lua error handler");
      else
        errorCallback("error running Lua script");
      return;
    }
    lua_getfield(luaState, LUA_GLOBALSINDEX, "breakPointCallback");
    if (!lua_isfunction(luaState, -1))
      lua_pop(luaState, 1);
    else
      haveBreakPointCallback = true;
#else
    errorCallback("Lua scripting is not supported by this build of ep128emu");
#endif  // HAVE_LUA_H
  }

  void LuaScript::closeScript()
  {
    haveBreakPointCallback = false;
#ifdef HAVE_LUA_H
    if (luaState) {
      lua_close(luaState);
      luaState = (lua_State *) 0;
    }
#endif  // HAVE_LUA_H
  }

  void LuaScript::errorCallback(const char *msg)
  {
    if (msg == (char *) 0 || msg[0] == '\0')
      msg = "Lua script error";
    throw Exception(msg);
  }

  void LuaScript::messageCallback(const char *msg)
  {
    if (!msg)
      msg = "";
    std::printf("%s\n", msg);
  }

}       // namespace Ep128Emu
