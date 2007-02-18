/*
 * lua-espeak - A speech synthesis library for the Lua programming language
 * (c) 2007 Alexandre Erwin Ittner <aittner@netuno.com.br>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 *
 */


#include <lua.h>
#include <lauxlib.h>
#include <stdlib.h>

#define LIB_NAME        "espeak"
#define LIB_VERSION     LIB_NAME " r1"








static const luaL_reg funcs[] = {
    { NULL, NULL }
};

int luaopen_espeak(lua_State *L) {
    luaL_register(L, LIB_NAME, funcs);

    lua_pushliteral(L, "VERSION");
    lua_pushstring(L, LIB_VERSION);
    lua_settable(L, -3);

    return 1;
}
