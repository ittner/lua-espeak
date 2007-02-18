# lua-espeak - A speech synthesis library for the Lua programming language
# (c) 2007 Alexandre Erwin Ittner <aittner@netuno.com.br>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
#
#

#CC = gcc
#RM = rm

# Name of .pc file. "lua5.1" on Debian/Ubuntu
LUAPKG = lua5.1
CFLAGS = `pkg-config $(LUAPKG) --cflags` -O3 -Wall
INSTALL_PATH = `pkg-config $(LUAPKG) --variable=INSTALL_CMOD`
LIBS = `pkg-config $(LUAPKG) --libs` -lespeak

## If your system doesn't have pkg-config, comment out the previous lines and
## uncomment and change the following ones according to your building
## enviroment.

#CFLAGS = -I/usr/include/lua5.1/ -O3 -Wall
#LIBS = -llua5.1 -lespeak
#INSTALL_PATH = /usr/lib/lua/5.1


espeak.so: luaespeak.c
	$(CC) -o espeak.so -shared $(LIBS) $(CFLAGS) luaespeak.c

install: espeak.so
	make test
	install -s espeak.so $(INSTALL_PATH)

clean:
	$(RM) espeak.so

test: espeak.so test_espeak.lua
	lua test_espeak.lua

all: espeak.so
