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
#include <string.h>

#include "speak_lib.h"

#define LIB_NAME        "espeak"
#define LIB_VERSION     LIB_NAME " r1"

/* Table assumed on top */
#define SET_TBL_INT(L, c, v)    \
    lua_pushliteral(L, c);      \
    lua_pushnumber(L, v);       \
    lua_settable(L, -3);



static void push_event(lua_State *L, espeak_EVENT *ev);
static espeak_EVENT *get_event(lua_State *L, int i);
static void free_event(espeak_EVENT *ev);
static void push_language_list(lua_State *L, char *langlist);
static char *get_language_list(lua_State *L, int i);
static void push_voice(lua_State *L, espeak_VOICE *v);
static espeak_VOICE *get_voice(lua_State *L, int i);
static void free_voice(espeak_VOICE *v);
static void constants(lua_State *L);



static void push_event(lua_State *L, espeak_EVENT *ev) {
    lua_newtable(L);

    SET_TBL_INT(L, "type", ev->type);

    /* message identifier (or 0 for key or character) */
    SET_TBL_INT(L, "unique_identifier", ev->unique_identifier);

    /* the number of characters from the start of the text */
    SET_TBL_INT(L, "text_position", ev->text_position);

    /* word length, in characters (for espeakEVENT_WORD) */
    SET_TBL_INT(L, "length", ev->length);

    /* the time in mS within the generated speech output data */
    SET_TBL_INT(L, "audio_position", ev->audio_position);

    /* sample id (internal use) */
    SET_TBL_INT(L, "sample", ev->sample);

    if (ev->type == espeakEVENT_WORD || ev->type == espeakEVENT_SENTENCE) {
        /* used for WORD and SENTENCE events */
        SET_TBL_INT(L, "id", (ev->id.number));
    } else if (ev->type == espeakEVENT_MARK || ev->type == espeakEVENT_PLAY) {
        /* used for MARK and PLAY events.  UTF8 string */
        lua_pushliteral(L, "id");
        lua_pushstring(L, ev->id.name ? ev->id.name : "");
        lua_settable(L, -3);
    }
    return;
}


static espeak_EVENT *get_event(lua_State *L, int i) {
    espeak_EVENT *ev; 

    luaL_checktype(L, i, LUA_TTABLE);

    if ((ev = (espeak_EVENT*) malloc(sizeof(espeak_EVENT))) == NULL) {
        luaL_error(L, "Memory allocation failure");
        return NULL; /* Never reached. Just avoids warnings. */ 
    }

    ev->user_data = NULL;
    ev->type = espeakEVENT_LIST_TERMINATED;
    ev->unique_identifier = 0;
    ev->text_position = 0;
    ev->length = 0;
    ev->audio_position = 0;
    ev->sample = 0;
    ev->id.number = 0;

    lua_getfield(L, i, "type");
    if (!lua_isnil(L, -1)) {
        if (lua_type(L, -1) == LUA_TNUMBER) {
            ev->type = lua_tonumber(L, -1);
        } else {
            free(ev);
            luaL_error(L, "Bad event type");
            return NULL; /* Never reached. Just avoids warnings. */ 
        }
    }
    lua_pop(L, 1);


    lua_getfield(L, i, "unique_identifier");
    if (!lua_isnil(L, -1)) {
        if (lua_type(L, -1) == LUA_TNUMBER) {
            ev->unique_identifier = lua_tonumber(L, -1);
        } else {
            free(ev);
            luaL_error(L, "Bad event 'unique_identifier'");
            return NULL; /* Never reached. Just avoids warnings. */ 
        }
    }
    lua_pop(L, 1);


    lua_getfield(L, i, "text_position");
    if (!lua_isnil(L, -1)) {
        if (lua_type(L, -1) == LUA_TNUMBER) {
            ev->text_position = lua_tonumber(L, -1);
        } else {
            free(ev);
            luaL_error(L, "Bad event 'text_position'");
            return NULL; /* Never reached. Just avoids warnings. */ 
        }
    }
    lua_pop(L, 1);


    lua_getfield(L, i, "length");
    if (!lua_isnil(L, -1)) {
        if (lua_type(L, -1) == LUA_TNUMBER) {
            ev->length = lua_tonumber(L, -1);
        } else {
            free(ev);
            luaL_error(L, "Bad event 'lenght'");
            return NULL; /* Never reached. Just avoids warnings. */ 
        }
    }
    lua_pop(L, 1);


    lua_getfield(L, i, "audio_position");
    if (!lua_isnil(L, -1)) {
        if (lua_type(L, -1) == LUA_TNUMBER) {
            ev->audio_position = lua_tonumber(L, -1);
        } else {
            free(ev);
            luaL_error(L, "Bad event 'audio_position'");
            return NULL; /* Never reached. Just avoids warnings. */ 
        }
    }
    lua_pop(L, 1);


    lua_getfield(L, i, "sample");
    if (!lua_isnil(L, -1)) {
        if (lua_type(L, -1) == LUA_TNUMBER) {
            ev->sample = lua_tonumber(L, -1);
        } else {
            free(ev);
            luaL_error(L, "Bad event 'sample'");
            return NULL; /* Never reached. Just avoids warnings. */ 
        }
    }
    lua_pop(L, 1);


    lua_getfield(L, i, "id");
    if (ev->type == espeakEVENT_WORD || ev->type == espeakEVENT_SENTENCE) {
        if (!lua_isnil(L, -1)) {
            if (lua_type(L, -1) == LUA_TNUMBER) {
                ev->id.number = lua_tonumber(L, -1);
            } else {
                free(ev);
                luaL_error(L, "Bad event 'id'");
                return NULL; /* Never reached. Just avoids warnings. */ 
            }
        }
    } else if (ev->type == espeakEVENT_MARK || ev->type == espeakEVENT_PLAY) {
        if (!lua_isnil(L, -1)) {
            if (lua_type(L, -1) == LUA_TSTRING) {
                ev->id.name = strdup(lua_tostring(L, -1));
            } else {
                free(ev);
                luaL_error(L, "Bad event 'id'");
                return NULL; /* Never reached. Just avoids warnings. */ 
            }
        }
    }
    lua_pop(L, 1);

    return ev;
}

/*
static void free_event(espeak_EVENT *ev) {
    if ((ev->type == espeakEVENT_MARK || ev->type == espeakEVENT_PLAY)
    && ev->id.name) {
        free(ev->id.name);
    }
    free(ev);
}
*/

/*
 * The 'languages' field in the voice structure is a bit tricky. A explanation
 * from speak_lib.h follows:
 *
 *      Note: The espeak_VOICE structure is used for two purposes:
 *
 *      1.  To return the details of the available voices.
 *      2.  As a parameter to espeak_SetVoiceByProperties() in order to
 *          specify selection criteria.
 *
 *      In (1), the "languages" field consists of a list of (UTF8) language
 *      names for which this voice may be used, each language name in the
 *      list is terminated by a zero byte and is also preceded by a single
 *      byte which gives a "priority" number.  The list of languages is
 *      terminated by an additional zero byte.
 *
 *      A language name consists of a language code, optionally followed by
 *      one or more qualifier (dialect) names separated by hyphens (eg.
 *      "en-uk"). A voice might, for example, have languages "en-uk" and
 *      "en".  Even without "en" listed, voice would still be selected for
 *      the "en" language (because "en-uk" is related) but at a lower
 *      priority.
 *
 *      The priority byte indicates how the voice is preferred for the
 *      language. A low number indicates a more preferred voice, a higher
 *      number indicates a less preferred voice.
 *
 *      In (2), the "languages" field consists simply of a single (UTF8)
 *      language name, with no preceding priority byte.
 *
 * In Lua, this is represented in the form of a matrix:
 *
 *      langlist = {
 *          { 1, "pt-br" }
 *          { 2, "pt-pt" }
 *          { 3, "pt" }
 *          { 4, "en" }
 *      }
 *
 */

static void push_language_list(lua_State *L, char *langlist) {
    int i = 1;
    int pos = 0;

    lua_newtable(L);
    
    while (langlist[pos]) {
        lua_newtable(L);

        lua_pushnumber(L, 1);   /* Priority number */
        lua_pushnumber(L, (int) langlist[pos]);
        lua_settable(L, -3);
        pos++;

        lua_pushnumber(L, 2);   /* Language name */
        lua_pushstring(L, &langlist[pos]);
        lua_settable(L, -3);

        while (langlist[++pos]);
       
        lua_pushnumber(L, i++);
        lua_settable(L, -3);
        pos++;
    }
}

static char *get_language_list(lua_State *L, int i) {
    int index = 1;
    int pos = 0;
    size_t bufsz = 16;
    char *str;
    const char *name;

    luaL_checktype(L, i, LUA_TTABLE);

    str = (char*) malloc(sizeof(char) * bufsz);
    if (str == NULL) {
        luaL_error(L, "Memory allocation failure");
        return NULL; /* Never reached. Just avoids warnings. */ 
    }

    while (1) {
        /* Stack: i: <table> ... */
        lua_pushnumber(L, index++); /* Stack: i: <table> ... <index> */
        lua_gettable(L, -2);        /* Stack: i: <table> ... <table?> */

        if (lua_isnil(L, -1)) { /* End of list */
            /* Stack: i: <table> ... nil */
            lua_pop(L, 1);
            /* Stack: i: <table> ... */
            break;
        } else if (!lua_type(L, -1) == LUA_TTABLE) {
            free(str);
            luaL_error(L, "Bad priority/language pair found.");
            return NULL; /* Never reached. Just avoids warnings. */ 
        }

        /* Stack: i: <table> ... <table> */
        lua_pushnumber(L, 1);
        /* Stack: i: <table> ... <table> 1 */
        lua_gettable(L, -2);
        /* Stack: i: <table> ... <table> <number?> */

        if (!lua_type(L, -1) == LUA_TNUMBER) {
            free(str);
            luaL_error(L, "Bad language priority found.");
            return NULL; /* Never reached. Just avoids warnings. */ 
        }

        /* Stack: i: <table> ... <table> <number> */
        str[pos++] = (char) lua_tonumber(L, -1);        
    
        /* Stack: i: <table> ... <table> */
        lua_pushnumber(L, 2);
        /* Stack: i: <table> ... <table> 2 */
        lua_gettable(L, -2);
        /* Stack: i: <table> ... <table> <string?> */

        if (!lua_type(L, -1) == LUA_TSTRING) {
            free(str);
            luaL_error(L, "Bad language name found.");
            return NULL; /* Never reached. Just avoids warnings. */ 
        }

        /* Stack: i: <table> ... <table> <string> */
        name = lua_tostring(L, -1);

        /* Stack: i: <table> ... <table> */
        if (bufsz < (pos + strlen(name))) {
            bufsz = pos + strlen(name) + 10;
            str = (char*) realloc(str, bufsz * sizeof(char));
            if (str == NULL) {
                luaL_error(L, "Memory allocation failure");
                return NULL; /* Never reached. Just avoids warnings. */ 
            }
        }
        strcpy(&str[pos], name);
        pos += strlen(name) + 1;
        /* Stack: i: <table> ... <table> */
        lua_pop(L, 1);
        /* Stack: i: <table> ... */
    }

    str[pos] = '\0';
    return str;
}


/* Note that, for this function, the 'languages' field is used as a
 * language list (not as a standard string!)
 */

static void push_voice(lua_State *L, espeak_VOICE *v) {
    lua_newtable(L);

    /* a given name for this voice. UTF8 string. */
    if (v->name) {
        lua_pushliteral(L, "name");
        lua_pushstring(L, v->name);
        lua_settable(L, -3);
    }

    /* list of pairs of (byte) priority + (string) language (and dialect
     * qualifier) */
    if (v->languages) {
        lua_pushliteral(L, "name");
        push_language_list(L, v->languages);
        lua_settable(L, -3);
    }

    /* the filename for this voice within espeak-data/voices */
    if (v->identifier) {
        lua_pushliteral(L, "identifier");
        lua_pushstring(L, v->identifier);
        lua_settable(L, -3);
    }

    /* 0=none 1=male, 2=female */
    SET_TBL_INT(L, "gender", v->gender);

    /* 0=not specified, or age in years */
    SET_TBL_INT(L, "age", v->age);

    /* only used when passed as a parameter to espeak_SetVoiceByProperties */
    SET_TBL_INT(L, "variant", v->variant);

}


/* Note that, for this function, the 'languages' field is used as a standard
 * string (not as a language list!)
 */

static espeak_VOICE *get_voice(lua_State *L, int i) {
    espeak_VOICE *v; 

    luaL_checktype(L, i, LUA_TTABLE);

    if ((v = (espeak_VOICE*) malloc(sizeof(espeak_VOICE))) == NULL) {
        luaL_error(L, "Memory allocation failure");
        return NULL; /* Never reached. Just avoids warnings. */ 
    }

    v->name = NULL;
    v->languages = NULL;
    v->identifier = NULL;
    v->gender = 0;
    v->age = 0;
    v->variant = 0;

    lua_getfield(L, i, "name");
    if (!lua_isnil(L, -1)) {
        if (lua_type(L, -1) == LUA_TSTRING) {
            v->name = strdup(lua_tostring(L, -1));
        } else {
            free_voice(v);
            luaL_error(L, "Bad voice name.");
            return NULL; /* Never reached. Just avoids warnings. */ 
        }
    }
    lua_pop(L, 1);


    lua_getfield(L, i, "identifier");
    if (!lua_isnil(L, -1)) {
        if (lua_type(L, -1) == LUA_TSTRING) {
            v->identifier = strdup(lua_tostring(L, -1));
        } else {
            free_voice(v);
            luaL_error(L, "Bad voice 'identifier' field.");
            return NULL; /* Never reached. Just avoids warnings. */ 
        }
    }
    lua_pop(L, 1);


    lua_getfield(L, i, "languages");
    if (!lua_isnil(L, -1)) {
        if (lua_type(L, -1) == LUA_TSTRING) {
            v->languages = strdup(lua_tostring(L, -1));
        } else {
            free_voice(v);
            luaL_error(L, "Bad voice 'languages' field.");
            return NULL; /* Never reached. Just avoids warnings. */ 
        }
    }
    lua_pop(L, 1);


    lua_getfield(L, i, "gender");
    if (!lua_isnil(L, -1)) {
        if (lua_type(L, -1) == LUA_TNUMBER) {
            v->gender = lua_tonumber(L, -1);
        } else {
            free_voice(v);
            luaL_error(L, "Bad voice 'gender' field");
            return NULL; /* Never reached. Just avoids warnings. */ 
        }
    }
    lua_pop(L, 1);


    lua_getfield(L, i, "age");
    if (!lua_isnil(L, -1)) {
        if (lua_type(L, -1) == LUA_TNUMBER) {
            v->age = lua_tonumber(L, -1);
        } else {
            free_voice(v);
            luaL_error(L, "Bad voice 'age' field");
            return NULL; /* Never reached. Just avoids warnings. */ 
        }
    }
    lua_pop(L, 1);


    lua_getfield(L, i, "variant");
    if (!lua_isnil(L, -1)) {
        if (lua_type(L, -1) == LUA_TNUMBER) {
            v->variant = lua_tonumber(L, -1);
        } else {
            free_voice(v);
            luaL_error(L, "Bad voice 'variant' field");
            return NULL; /* Never reached. Just avoids warnings. */ 
        }
    }
    lua_pop(L, 1);

    return v;
}
            

static void free_voice(espeak_VOICE *v) {
    if (v->name)
        free(v->name);
    if (v->languages)
        free(v->languages);
    if (v->identifier)
        free(v->identifier);
    free(v);
}



static void constants(lua_State *L) {
    /*!!! Constants */

    /*!! General information */

    /*! espeak.VERSION
     * Holds information about library name and version. 
     */
    lua_pushliteral(L, "VERSION");
    lua_pushstring(L, LIB_VERSION);
    lua_settable(L, -3);


    /*!! Events */

    /*! espeak.EVENT_LIST_TERMINATED
     *  Retrieval mode: terminates the event list.
     */
    SET_TBL_INT(L, "EVENT_LIST_TERMINATED", espeakEVENT_LIST_TERMINATED)

    /*! espeak.EVENT_WORD
     *  Start of word.
     */
    SET_TBL_INT(L, "EVENT_WORD", espeakEVENT_WORD)

    /*! espeak.EVENT_SENTENCE
     *  Start of sentence.
     */
    SET_TBL_INT(L, "EVENT_SENTENCE", espeakEVENT_SENTENCE)

    /*! espeak.EVENT_MARK
     *  Mark.
     */
    SET_TBL_INT(L, "EVENT_MARK", espeakEVENT_MARK)

    /*! espeak.EVENT_PLAY
     *  Audio element.
     */
    SET_TBL_INT(L, "EVENT_PLAY", espeakEVENT_PLAY)

    /*! espeak.EVENT_END
     *  End of sentence.
     */
    SET_TBL_INT(L, "EVENT_END", espeakEVENT_END)

    /*! espeak.EVENT_MSG_TERMINATED
     *  End of message.
     */
    SET_TBL_INT(L, "EVENT_MSG_TERMINATED", espeakEVENT_MSG_TERMINATED)


    /*!! Positions */

    /*! espeak.POS_CHARACTER */
    SET_TBL_INT(L, "POS_CHARACTER", POS_CHARACTER)

    /*! espeak.POS_WORD */
    SET_TBL_INT(L, "POS_WORD", POS_WORD)

    /*! espeak.POS_SENTENCE */
    SET_TBL_INT(L, "POS_SENTENCE", POS_SENTENCE)


    /*!! Audio output */

    /*! espeak.AUDIO_OUTPUT_PLAYBACK
     *  PLAYBACK mode: plays the audio data, supplies events to the
     *  calling program.
     */
    SET_TBL_INT(L, "AUDIO_OUTPUT_PLAYBACK", AUDIO_OUTPUT_PLAYBACK)

    /*! espeak.AUDIO_OUTPUT_RETRIEVAL
     *  RETRIEVAL mode: supplies audio data and events to the calling program.
     */
    SET_TBL_INT(L, "AUDIO_OUTPUT_RETRIEVAL", AUDIO_OUTPUT_RETRIEVAL)

    /*! espeak.AUDIO_OUTPUT_SYNCHRONOUS
     *  SYNCHRONOUS mode: as RETRIEVAL but doesn't return until synthesis is    
     *  completed.
     */
    SET_TBL_INT(L, "AUDIO_OUTPUT_SYNCHRONOUS", AUDIO_OUTPUT_SYNCHRONOUS)


    /*!! Errors and status */

    /*! espeak.EE_OK */
    SET_TBL_INT(L, "EE_OK", EE_OK)

    /*! espeak.EE_INTERNAL_ERROR */
    SET_TBL_INT(L, "EE_INTERNAL_ERROR", EE_INTERNAL_ERROR)

    /*! espeak.EE_BUFFER_FULL */
    SET_TBL_INT(L, "EE_BUFFER_FULL", EE_BUFFER_FULL)


    /*!! Synthesis */

    /*! espeak.CHARS_AUTO */
    SET_TBL_INT(L, "CHARS_AUTO", espeakCHARS_AUTO)

    /*! espeak.CHARS_UTF8 */
    SET_TBL_INT(L, "CHARS_UTF8", espeakCHARS_UTF8)

    /*! espeak.CHARS_8BIT */
    SET_TBL_INT(L, "CHARS_8BIT", espeakCHARS_8BIT)

    /*! espeak.CHARS_WCHAR */
    SET_TBL_INT(L, "CHARS_WCHAR", espeakCHARS_WCHAR)

    /*! espeak.SSML */
    SET_TBL_INT(L, "SSML", espeakSSML)

    /*! espeak.PHONEMES */
    SET_TBL_INT(L, "PHONEMES", espeakPHONEMES)

    /*! espeak.ENDPAUSE */
    SET_TBL_INT(L, "ENDPAUSE", espeakENDPAUSE)

    /*! espeak.KEEP_NAMEDATA */
    SET_TBL_INT(L, "KEEP_NAMEDATA", espeakKEEP_NAMEDATA)


    /*!! Parameters */

    /*! espeak.RATE */
    SET_TBL_INT(L, "RATE", espeakRATE)

    /*! espeak.VOLUME */
    SET_TBL_INT(L, "VOLUME", espeakVOLUME)

    /*! espeak.PITCH */
    SET_TBL_INT(L, "PITCH", espeakPITCH)

    /*! espeak.RANGE */
    SET_TBL_INT(L, "RANGE", espeakRANGE)

    /*! espeak.PUNCTUATION */
    SET_TBL_INT(L, "PUNCTUATION", espeakPUNCTUATION)

    /*! espeak.CAPITALS */
    SET_TBL_INT(L, "CAPITALS", espeakCAPITALS)


    /*!! Punctuation */

    /*! espeak.PUNCT_NONE */
    SET_TBL_INT(L, "PUNCT_NONE", espeakPUNCT_NONE)

    /*! espeak.PUNCT_ALL */
    SET_TBL_INT(L, "PUNCT_ALL", espeakPUNCT_ALL)

    /*! espeak.PUNCT_SOME */
    SET_TBL_INT(L, "PUNCT_SOME", espeakPUNCT_SOME)
}



static const luaL_reg funcs[] = {
    { NULL, NULL }
};


int luaopen_espeak(lua_State *L) {
    luaL_register(L, LIB_NAME, funcs);
    constants(L);





    return 1;
}
