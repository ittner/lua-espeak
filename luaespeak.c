/*
 * lua-espeak - A speech synthesis library for the Lua programming language
 * (c) 2007-08 Alexandre Erwin Ittner <aittner@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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


/*
 * WARNING: THE FOLLOWING CODE STILLS UNDER DEVELOPMENT AND MUST BE
 * CONSIDERED AS BETA QUALITY. If you find a bug, please report me. 
 *
 * TODO: Do a better handler for callbacks (realy needed?).
 * TODO: Are these globals secure?
 * TODO: Make a better "magic" documentation.
 *
 */


#include <lua.h>
#include <lauxlib.h>

/* For LUA_FILEHANDLE */
#include <lualib.h>

#include <stdlib.h>
#include <string.h>

#include <espeak/speak_lib.h>

#define LIB_NAME        "espeak"
#define LIB_VERSION     LIB_NAME " 1.36r1"

/* Secure initialization and termination */
typedef enum { NOT_INITIALIZED, INITIALIZED, TERMINATED } status_info_t;

/* Table assumed on top */
#define SET_TBL_INT(L, c, v)    \
    lua_pushliteral(L, c);      \
    lua_pushnumber(L, v);       \
    lua_settable(L, -3);

#define checkfile(L, i) (*(FILE **) luaL_checkudata(L, i, LUA_FILEHANDLE))


/* Internal library controls */
static status_info_t lib_status = NOT_INITIALIZED; /* Security */
static int ref_synth_callback = LUA_NOREF;  /* Reference to URI callback */
static int ref_uri_callback = LUA_NOREF;    /* Reference to Synth callback */
static lua_State *callback_state = NULL;    /* Pass states to callbacks */



/* Prototypes */

static void push_event(lua_State *L, espeak_EVENT *ev);
static espeak_EVENT *get_event(lua_State *L, int i);
static void push_language_list(lua_State *L, const char *langlist);
static char *get_language_list(lua_State *L, int i);
static void push_voice(lua_State *L, const espeak_VOICE *v);
static espeak_VOICE *get_voice(lua_State *L, int i);
static void free_voice(espeak_VOICE *v);
static void constants(lua_State *L);
static int lInitialize(lua_State *L);
static int lInfo(lua_State *L);  
static int lSynth(lua_State *L);
static int lKey(lua_State *L);
static int lChar(lua_State *L);
static int lSetParameter(lua_State *L);
static int lGetParameter(lua_State *L);
static int lSetPunctuationList(lua_State *L);
static int lSetPhonemeTrace(lua_State *L);
static int lCompileDictionary(lua_State *L);
static int lListVoices(lua_State *L);
static int lCancel(lua_State *L);
static int lIsPlaying(lua_State *L);
static int lTerminate(lua_State *L);




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

    if (ev->type == espeakEVENT_WORD || ev->type == espeakEVENT_SENTENCE
    || ev->type == espeakEVENT_PHONEME) {
        /* used for WORD, SENTENCE and PHONEME events. For PHONEME events
           this is the phoneme mnemonic. */
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
 * The 'languages' field in the voice structure is a bit tricky. An
 * explanation from speak_lib.h follows:
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

static void push_language_list(lua_State *L, const char *langlist) {
    int i = 1;
    int pos = 0;

    lua_newtable(L);

    while (langlist[pos]) {
        lua_pushnumber(L, i++);
        lua_newtable(L);

        /* Priority number */
        lua_pushnumber(L, 1);
        lua_pushnumber(L, (int) langlist[pos]);
        lua_settable(L, -3);
        pos++;

        /* Language name */
        lua_pushnumber(L, 2);
        lua_pushstring(L, &langlist[pos]);
        lua_settable(L, -3);

        lua_settable(L, -3);

        while (langlist[++pos]);
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

static void push_voice(lua_State *L, const espeak_VOICE *v) {
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
        lua_pushliteral(L, "languages");
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
    
    /*! espeak.EVENT_PHONEME
     *  Phoneme, if enabled in espeak.Initialize()
     */
    SET_TBL_INT(L, "EVENT_PHONEME", espeakEVENT_PHONEME)

    /*! espeak.EVENT_MARK
     *  Mark.
     */
    SET_TBL_INT(L, "EVENT_MARK", espeakEVENT_MARK)

    /*! espeak.EVENT_PLAY
     *  Audio element.
     */
    SET_TBL_INT(L, "EVENT_PLAY", espeakEVENT_PLAY)

    /*! espeak.EVENT_END
     *  End of sentence or clause.
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

    /*! espeak.AUDIO_OUTPUT_SYNCH_PLAYBACK
     *  Synchronous playback mode: plays the audio data, supplies events to the
     *  calling program.
     */
    SET_TBL_INT(L, "AUDIO_OUTPUT_SYNCH_PLAYBACK", AUDIO_OUTPUT_SYNCH_PLAYBACK)


    /*!! Errors and status */

    /*! espeak.EE_OK */
    SET_TBL_INT(L, "EE_OK", EE_OK)

    /*! espeak.EE_INTERNAL_ERROR */
    SET_TBL_INT(L, "EE_INTERNAL_ERROR", EE_INTERNAL_ERROR)

    /*! espeak.EE_BUFFER_FULL */
    SET_TBL_INT(L, "EE_BUFFER_FULL", EE_BUFFER_FULL)

    /*! espeak.EE_NOT_FOUND */
    SET_TBL_INT(L, "EE_NOT_FOUND", EE_NOT_FOUND)

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
    
    /*! espeak.WORDGAP */
    SET_TBL_INT(L, "WORDGAP", espeakWORDGAP)


    /*!! Punctuation */

    /*! espeak.PUNCT_NONE */
    SET_TBL_INT(L, "PUNCT_NONE", espeakPUNCT_NONE)

    /*! espeak.PUNCT_ALL */
    SET_TBL_INT(L, "PUNCT_ALL", espeakPUNCT_ALL)

    /*! espeak.PUNCT_SOME */
    SET_TBL_INT(L, "PUNCT_SOME", espeakPUNCT_SOME)
}



/*!!! Functions */

/*!! Initialization */

/*! espeak.Initialize(audio_output, buflength, [ path, [ options ]])
 *
 * Must be called before any synthesis functions are called. This function
 * yields errors if called more then once.
 *
 * 'audio_output' is the audio data can either be played by eSpeak or passed
 *  back by the SynthCallback function.
 *
 * 'buflength' is the length (in miliseconds) of sound buffers passed to the
 * SynthCallback function.
 *
 * 'path' is the directory which contains the espeak-data directory, or nil
 * for the default location.
 *
 * 'options' is a integer bitvector. The following values are valid:
 *      Bit 0: Set to allow espeak.EVENT_PHONEME events.
 * for compatibility with previous versions of Lua-eSpeak, passing 'nil' or
 * not passing this parameter is interpreted as zero.
 *
 * This function returns the sample rate in Hz or 'nil' (internal error);
 *
 */

static int lInitialize(lua_State *L) { 
    const char *path = NULL;
    int options = 0;
    int ret;

    if (lib_status == INITIALIZED)
        luaL_error(L, "eSpeak was already initialized.");

    if (lib_status == TERMINATED)
        luaL_error(L, "eSpeak was already terminated.");
    
    if (!lua_isnil(L, 3))
        path = lua_tostring(L, 3);
        
    if (!lua_isnil(L, 4))
        options = lua_tointeger(L, 4);
    
    ret = espeak_Initialize(luaL_checknumber(L, 1),
        luaL_checknumber(L, 2), path, options);

    if (ret != -1) {    /* ret == -1 -> Internal error */
        lua_pushnumber(L, ret);
    } else {
        lua_pushnil(L);
        return 1;   /* Internal error */
    }

    if (ref_synth_callback != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, ref_synth_callback);
        ref_synth_callback = LUA_NOREF;
    }

    if (ref_uri_callback != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, ref_uri_callback);
        ref_uri_callback = LUA_NOREF;
    }

    lib_status = INITIALIZED;
    return 1;
}



/*! espeak.Info()
 *
 * Gives the version of the eSpeak library, as a string. The version of
 * the Lua binding is given in espeak.VERSION, instead.
 *
 */

static int lInfo(lua_State *L) { 
    lua_pushstring(L, espeak_Info(NULL));
    return 1;
}



/*! espeak.SetSynthCallback(callback_function)
 *
 * Must be called before any synthesis functions are called. This specifies
 * a function in the calling program which is called when a buffer of speech
 * sound data has been produced. 
 * 
 * The callback function is of the form:
 *  
 *      function callback(wave, events)
 *          ...
 *          ...
 *          return false     -- or true.
 *      end
 *
 *
 * Where 'wave' is a string with the speech sound data which has been
 * produced. 'nil' indicates that the synthesis has been completed. And empty
 * string  does NOT indicate end of synthesis. 'events' is a table with items
 * which indicate word and sentence events, and  also the occurance if <mark>
 * and <audio> elements within the text. Valid elements are:
 *
 *      type: The event type, that must be espeak.EVENT_LIST_TERMINATED,
 *            EVENT_WORD, EVENT_SENTENCE, EVENT_PHONEME (if enabled in 
 *            speak.Initialize()), EVENT_MARK, EVENT_PLAY, EVENT_END
 *            or EVENT_MSG_TERMINATED.
 *      
 *      unique_identifier: The integer id passed from Synth function.
 *
 *      text_position: The number of characters from the start of the text.
 *  
 *      length: For espeak.EVENT_WORD, the word length, in characters.
 *      
 *      audio_position: The time in ms within the generated output data.
 *
 *      id: a number for WORD, SENTENCE or PHONEME events or a UTF8 string
 *          for MARK and PLAY events.
 *
 * Callback functions must return 'false' to continue synthesis or 'true' to
 * abort.
 *
 */

static int synth_callback(short *wav, int numsamples, espeak_EVENT *events) {
    int rval = 0;

    if (ref_synth_callback == LUA_NOREF || callback_state == NULL)
        return 0;   /* No callback defined. Continue synthesis. */

    /* Pushes the function. */
    lua_rawgeti(callback_state, LUA_REGISTRYINDEX, ref_synth_callback);

    if (lua_isnil(callback_state, -1)
    || lua_type(callback_state, -1) != LUA_TFUNCTION) {
        lua_pop(callback_state, 1);
        return 1;  /* A bad value was found instead of a function. Stops. */
    }

    /* Push the arguments. */
    lua_pushlstring(callback_state, (void*) wav, numsamples * sizeof(short));
    push_event(callback_state, events);

    if (lua_pcall(callback_state, 2, 1, 0) != 0) {
        lua_pop(callback_state, 1); /* Ignore error message. */
        return 1;   /* Error. Stop synthesis. */
    }

    rval = lua_toboolean(callback_state, -1);
    lua_pop(callback_state, 1);
    return rval;
}


static int lSetSynthCallback(lua_State *L) {
    if (lua_isnil(L, 1)) {
        espeak_SetSynthCallback(NULL);
        if (ref_synth_callback != LUA_NOREF) {
            luaL_unref(L, LUA_REGISTRYINDEX, ref_synth_callback);
            ref_synth_callback = LUA_NOREF;
        }
        return 0;
    }

    luaL_checktype(L, 1, LUA_TFUNCTION);
    ref_synth_callback = luaL_ref(L, LUA_REGISTRYINDEX);
    callback_state = L;
    espeak_SetSynthCallback(synth_callback);
    return 0;
}




/*! espeak.SetUriCallback(callback_function)
 *
 * This function must be called before synthesis functions are used, in
 * order to deal with <audio> tags. It specifies a callback function which
 * is called when an <audio> element is encountered and allows the calling
 * program to indicate whether the sound file which is specified in the
 * <audio> element is available and is to be played.
 *
 * The callback function is of the form:
 *
 *      function callback(type, uri, base)
 *          ...
 *          ...
 *          return false     -- or true.
 *      end
 *
 * Where:
 *
 * 'type' is type of callback event. Currently only 1 = <audio> element.
 * 'uri' is the "src" attribute from the <audio> element, a string.
 * 'base' is the "xml:base" attribute (if any) from the <speak> element
 *
 * The callback function must return 'true' to don't play the sound, but
 * speak the text alternative or 'false' to place a PLAY event in the event
 * list at the point where the <audio> element occurs. The calling program
 * can then play the sound at that point.
 *
 */


static int uri_callback(int type, const char *uri, const char *base) {
    int rval = 0;

    if (ref_uri_callback == LUA_NOREF || callback_state == NULL)
        return 0;   /* No callback defined. Continue synthesis. */

    /* Pushes the function. */
    lua_rawgeti(callback_state, LUA_REGISTRYINDEX, ref_uri_callback);

    if (lua_isnil(callback_state, -1)
    || lua_type(callback_state, -1) != LUA_TFUNCTION) {
        lua_pop(callback_state, 1);
        return 1;  /* A bad value was found instead of a function. Stops. */
    }

    /* Push the arguments. */
    lua_pushinteger(callback_state, type);
    lua_pushstring(callback_state, uri);
    if (base)
        lua_pushstring(callback_state, base);
    else
        lua_pushnil(callback_state);

    if (lua_pcall(callback_state, 3, 1, 0) != 0) {
        lua_pop(callback_state, 1); /* Ignore error message. */
        return 1;   /* Error. Stop synthesis. */
    }

    rval = lua_toboolean(callback_state, -1);
    lua_pop(callback_state, 1);
    return rval;
}


static int lSetUriCallback(lua_State *L) {
    if (lua_isnil(L, 1)) {
        espeak_SetUriCallback(NULL);
        if (ref_uri_callback != LUA_NOREF) {
            luaL_unref(L, LUA_REGISTRYINDEX, ref_uri_callback);
            ref_uri_callback = LUA_NOREF;
        }
        return 0;
    }

    luaL_checktype(L, 1, LUA_TFUNCTION);
    ref_uri_callback = luaL_ref(L, LUA_REGISTRYINDEX);
    callback_state = L;
    espeak_SetUriCallback(uri_callback);
    return 0;
}





/*!! Synthesis */

/*! espeak.Synth(text, position, position_type, [ end_position, [ flags ]]) 
 *
 * Synthesize speech for the specified text.  The speech sound data is passed
 * to the calling program in buffers by means of the callback function
 * specified by espeak.SetSynthCallback(). The command is asynchronous: it
 * is internally buffered and returns as soon as possible. If
 * espeak.Initialize() was previously called with espeak.AUDIO_OUTPUT_PLAYBACK
 * as argument, the sound data are played by eSpeak.
 *
 * 'text' is a string with the text to be spoken.  It may be either 8-bit
 * characters,  wide characters, or UTF8 encoding. Which of these is
 * determined by the 'flags' parameter.
 *
 * 'position' is the position in the text where speaking starts. Zero or nil
 * indicates speak from the start of the text.
 *
 * 'position_type' determines whether "position" is a number of characters,
 * words, or sentences. Valied values are espeak.POS_CHARACTER, 
 * espeak.POS_WORD or espeak.POS_SENTENCE.
 *
 * 'end_position', if set, this gives a character position at which speaking
 * will stop.  A value of zero or nil indicates no end position.
 *
 * 'flags': These may be added together:
 *     Type of character codes, one of: espeak.CHARS_UTF8, espeak.CHARS_8BIT,
 *          espeak.CHARS_AUTO (default) or espeak.CHARS_WCHAR.
 *
 *     espeak.SSML   Elements within < > are treated as SSML elements, or if
 *          not recognised are ignored.
 *
 *     espeak.PHONEMES  Text within [[ ]] is treated as phonemes codes (in
 *          espeak's Hirshenbaum encoding).
 *
 *     espeak.ENDPAUSE  If set then a sentence pause is added at the end of
 *          the text.  If not set then this pause is suppressed.
 *
 *
 * This function returns two values: the status of the operation (espeak.EE_OK,
 * espeak.EE_BUFFER_FULL or espeak.EE_INTERNAL_ERROR) and an unique integer
 * that will also be passed to the callback function (if any).
 *
 */


static int lSynth(lua_State *L) {
    const char *text = NULL;
	size_t size = 0;
	unsigned int position = 0;
	espeak_POSITION_TYPE ptype = POS_CHARACTER;
	unsigned int end_position = 0;
	unsigned int flags = espeakCHARS_AUTO;
	unsigned int id = 0;

    text = luaL_checkstring(L, 1);  
    size = (strlen(text) + 1) * sizeof(char);
    
    if (!lua_isnil(L, 2))
        position = (int) lua_tonumber(L, 2);

    if (!lua_isnil(L, 3))
        ptype = (int) lua_tonumber(L, 3);

    if (!lua_isnil(L, 4))
        end_position = (int) lua_tonumber(L, 4);

    if (!lua_isnil(L, 5))
        flags = (int) lua_tonumber(L, 5);

    lua_pushnumber(L, espeak_Synth(text, size, position, ptype, end_position,
        flags, &id, NULL));
    lua_pushinteger(L, id);

    return 2;
}



/*! espeak.Synth_Mark(text, index_mark, [ end_position, [ flags ]])
 *
 * Synthesize speech for the specified text. Similar to espeak.Synth() but
 * the start position is specified by the name of a <mark> element in the
 * text.
 *
 * 'index_mark' is the "name" attribute of a <mark> element within the text
 *  which specified the point at which synthesis starts. it must be an UTF8
 *  string.
 *
 *  For the other parameters, see espeak.Synth()
 *
 * This function returns two values: the status of the operation (espeak.EE_OK,
 * espeak.EE_BUFFER_FULL or espeak.EE_INTERNAL_ERROR) and an unique integer
 * that will also be passed to the callback function (if any).
 */

static int lSynth_Mark(lua_State *L) {
    const char *text = NULL;
    const char *mark = NULL;
	size_t size = 0;
	unsigned int end_position = 0;
	unsigned int flags = espeakCHARS_AUTO;
	unsigned int id = 0;

    text = luaL_checkstring(L, 1);
    size = (strlen(text) + 1) * sizeof(char);
    
    mark = luaL_checkstring(L, 2);

    if (!lua_isnil(L, 3))
        end_position = (int) lua_tonumber(L, 3);

    if (!lua_isnil(L, 4))
        flags = (int) lua_tonumber(L, 4);

    lua_pushnumber(L, espeak_Synth_Mark(text, size, mark, end_position,
        flags, &id, NULL));
    lua_pushinteger(L, id);

    return 2;
}


/*! espeak.Key(key_name)
 *
 * Speak the name of a keyboard key. If key_name is a single character, it
 * speaks the name of the character. Otherwise, it speaks key_name as a text
 * string.
 *
 *  Return: espeak.EE_OK: operation achieved 
 *          espeak.EE_BUFFER_FULL: the command can not be buffered;  you may
 *              try to call the function again after a while.
 *	        espeak.EE_INTERNAL_ERROR.
 */

static int lKey(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);

    lua_pushnumber(L, espeak_Key(name));
    return 1;
}


/*! espeak.Char(character_code)
 *
 * Speak the name of the character, given as a 16 bit integer.
 *
 *  Return: espeak.EE_OK: operation achieved 
 *          espeak.EE_BUFFER_FULL: the command can not be buffered;  you may
 *              try to call the function again after a while.
 *	        espeak.EE_INTERNAL_ERROR.
 */

static int lChar(lua_State *L) {
    wchar_t ch = (wchar_t) lua_tonumber(L, 1);
    lua_pushnumber(L, espeak_Char(ch));
    return 1;
}


/*!! Speech parameters */

/*! espeak.SetParameter(parameter, value, relative)
 *
 * Sets the value of the specified parameter. 'relative' is a boolean that
 * marks the value as relative to the current value.
 *
 * The following parameters are valid:
 *
 *      espeak.RATE:    speaking speed in word per minute.
 *      espeak.VOLUME:  volume in range 0-100, 0 = silence
 *      espeak.PITCH:   base pitch, range 0-100.  50=normal
 *      espeak.RANGE:   pitch range, range 0-100. 0-monotone, 50=normal
 *
 *      espeak.PUNCTUATION:  which punctuation characters to announce:
 *         value in espeak_PUNCT_TYPE (none, all, some), 
 *         see espeak_GetParameter() to specify which characters are announced.
 *
 *      espeak.CAPITALS: announce capital letters by:
 *         0=none,
 *         1=sound icon,
 *         2=spelling,
 *         3 or higher, by raising pitch.  This values gives the amount
 *              in Hz by which the pitch of a word raised to indicate it
 *              has a capital letter.
 *
 *      espeak.WORDGAP: pause between words, units of 10ms (at the default speed)
 *
 *  Return: espeak.EE_OK: operation achieved 
 *          espeak.EE_BUFFER_FULL: the command can not be buffered;  you may
 *              try to call the function again after a while.
 *	        espeak.EE_INTERNAL_ERROR.
 */

static int lSetParameter(lua_State *L) {
    espeak_PARAMETER p = 0;
    int v = 0;
    int rel = 0;

    p = (espeak_PARAMETER) luaL_checknumber(L, 1);
    v = (int) luaL_checknumber(L, 2);
    rel = lua_toboolean(L, 3);

    lua_pushnumber(L, espeak_SetParameter(p, v, rel));
    return 1;
}



/*! espeak.GetParameter(parameter, current)
 *
 * Returns synthesis parameters. 'current' is a boolean that tells the
 * function to return the current value, instead of the default one.
 *
 */

static int lGetParameter(lua_State *L) {
    espeak_PARAMETER p = 0;
    int curr = 0;

    p = (espeak_PARAMETER) luaL_checknumber(L, 1);
    curr = lua_toboolean(L, 2);
    lua_pushnumber(L, espeak_GetParameter(p, curr));
    return 1;
}



/*! espeak.SetPunctuationList(punctlist)
 *
 * Specified a list of punctuation characters whose names are to be spoken
 * when the value of the Punctuation parameter is set to "some". 'punctlist'
 * is a array of character codes (as integers). 
 *
 */

static int lSetPunctuationList(lua_State *L) {
    wchar_t *punctlist = NULL;
    size_t bufsz = 16;
    int i = 1;

    luaL_checktype(L, 1, LUA_TTABLE);

    punctlist = (wchar_t*) malloc(bufsz * sizeof(wchar_t));
    if (punctlist == NULL) {
        luaL_error(L, "Memory allocation failure");
        return 0;
    }

    while (1) {
        /* Stack: 1:<table> ... */
        lua_pushnumber(L, i);
        /* Stack: 1:<table> ... i */
        lua_gettable(L, 1);
        /* Stack: 1:<table> ... <somevalue> */
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            break;
        } else if (lua_type(L, -1) == LUA_TNUMBER) {
            punctlist[i-1] = (wchar_t) lua_tointeger(L, -1);
            lua_pop(L, 1);
            /* Stack: 1:<table> ... */
            i++;
        } else {
            free(punctlist);
            luaL_error(L, "Bad character code found.");
            return 0;
        }
        if (i >= bufsz) {
            bufsz *= 2;
            punctlist = (wchar_t*) realloc(punctlist, bufsz * sizeof(wchar_t));
            if (punctlist == NULL) {
                luaL_error(L, "Memory allocation failure");
                return 0;
            }
        }
    }

    punctlist[i-1] = (wchar_t) 0;
    lua_pushnumber(L, espeak_SetPunctuationList(punctlist));
    free(punctlist);
    return 1;
}




/*! espeak.SetPhonemeTrace(value, filehandle)
 *
 * Controls the output of phoneme symbols for the text.
 *
 *  value=0  No phoneme output (default)
 *  value=1  Output the translated phoneme symbols for the text
 *  value=2  as (1), but also output a trace of how the translation was done
 *           (matching rules and list entries)
 *
 * 'filehandle' is the output stream for the phoneme symbols (and trace). If
 * nil then it uses io.stdout.
 * 
 * This function returns no values.
 *
 */

static int lSetPhonemeTrace(lua_State *L) {
    int v = lua_tointeger(L, 1);
    FILE *fp = NULL;

    if (!lua_isnil(L, 2))
        fp = checkfile(L, 2);
    
    espeak_SetPhonemeTrace(v, fp);
    return 0;
}

    

/*! espeak.CompileDictionary(path, filehandle, [ flags ])
 *
 * Compile pronunciation dictionary for a language which corresponds to the
 * currently selected voice. The required voice should be selected before
 * calling this function.
 *
 * 'path' is the directory which contains the language's "_rules" and
 *  "_list" files. 'path' should end with a path separator character ('/').
 *
 * 'filehandle' is the output stream for error reports and statistics
 * information. If nil, then io.stderr will be used.
 *
 * 'flags' is a integer bitvector that accepts the following values:
 *     Bit 0: include source line information for debug purposes (as is
 *            displayed with the -X command line option in 'speak' command).
 * for compatibility with previous versions of Lua-eSpeak, passing 'nil' or
 * not passing this parameter is interpreted as zero.
 *      
 *
 * This function returns no values.
 *
 */

static int lCompileDictionary(lua_State *L) {
    const char *path;
    FILE *fp = NULL;
    int flags = 0;

    path = luaL_checkstring(L, 1);
    if (!lua_isnil(L, 2))
        fp = checkfile(L, 2);
        
    if (!lua_isnil(L, 3))
        flags = lua_tointeger(L, 3);

    espeak_CompileDictionary(path, fp, flags);
    return 0;
}


/*!! Voice Selection */


/*! espeak.ListVoices(voice_spec)
 *
 * Reads the voice files from espeak-data/voices and creates an array of
 * voice tables. If 'voice_spec' is given, then only the voices which are
 * compatible with the 'voice_spec' are listed, and they are listed in
 * preference order.
 * 
 */

static int lListVoices(lua_State *L) {
    espeak_VOICE *v = NULL; 
    const espeak_VOICE **vl;
    int i;

    if (lua_gettop(L) > 0)
        v = get_voice(L, 1);

    vl = espeak_ListVoices(v);

    if (v != NULL)
        free_voice(v);

    lua_newtable(L);

    for (i = 0; vl[i]; i++) {
        /* Stack: 1: ... <table> */
        lua_pushnumber(L, i+1); 
        push_voice(L, vl[i]);
        lua_settable(L, -3);
    }

    return 1;
}



/*! espeak.SetVoiceByName(name)
 *
 * Searches for a voice with a matching "name" field.  Language is not
 * considered. "name" is a UTF8 string.
 *
 *  Return: espeak.EE_OK: operation achieved 
 *          espeak.EE_BUFFER_FULL: the command can not be buffered;  you may
 *              try to call the function again after a while.
 *	        espeak.EE_INTERNAL_ERROR.
 * 
 */

static int lSetVoiceByName(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    lua_pushnumber(L, espeak_SetVoiceByName(name));
    return 1;
}


/*! espeak.SetVoiceByProperties(voice_spec)
 *
 * An voice table is used to pass criteria to select a voice. Any of the
 * following fields may be set:
 *
 *  name        nil or a voice name
 *
 *  languages   nil or a single language string (with optional dialect), eg.
 *              "en-uk", or "en"
 *
 *  gender      0 or nil = not specified, 1 = male, 2 = female
 *
 *  age         0 or nil = not specified, or an age in years
 *
 *  variant     After a list of candidates is produced, scored and sorted,
 *              "variant" is used to index that list and choose a voice.
 *              variant=0 takes the top voice (i.e. best match), variant=1
 *              takes the next voice, etc
 *
 *  Return: espeak.EE_OK: operation achieved 
 *          espeak.EE_BUFFER_FULL: the command can not be buffered;  you may
 *              try to call the function again after a while.
 *	        espeak.EE_INTERNAL_ERROR.
 *
 */

static int lSetVoiceByProperties(lua_State *L) {
    espeak_VOICE *v = get_voice(L, 1);
    lua_pushnumber(L, espeak_SetVoiceByProperties(v));
    return 1;
}


/*! espeak.GetCurrentVoice()
 *
 * Returns a voice table data for the currently selected voice. This is not
 * affected by temporary voice changes caused by SSML elements such as
 * <voice> and <s>.
 *
 */

static int lGetCurrentVoice(lua_State *L) {
    espeak_VOICE *v = espeak_GetCurrentVoice();
    if (v != NULL)
        push_voice(L, v);
    else
        lua_pushnil(L);
    return 1;
}


/*!! Flow control */

/*! espeak.Cancel()
 * Stop immediately synthesis and audio output of the current text. When this
 * function returns, the audio output is fully stopped and the synthesizer is
 * ready to synthesize a new message. This function returns espeak.EE_OK if
 * the operation was achieved or espeak.EE_INTERNAL_ERROR.
 */

static int lCancel(lua_State *L) {
    lua_pushnumber(L, espeak_Cancel());
    return 1;
}


/*! espeak.IsPlaying()
 * Returns 'true' if audio is playing or 'false' otherwise.
 */

static int lIsPlaying(lua_State *L) {
    lua_pushboolean(L, espeak_IsPlaying());
    return 1;
}


/*! espeak.Synchronize()
 * This function returns when all data have been spoken. Returns
 * espeak.EE_OK if the operation was achieved or espeak.EE_INTERNAL_ERROR.
 */

static int lSynchronize(lua_State *L) {
    lua_pushboolean(L, espeak_Synchronize());
    return 1;
}


/*! espeak.Terminate()
 * Last function to be called. Returns espeak.EE_OK if the operation was
 * achieved, espeak.EE_INTERNAL_ERROR on eSpeak error. This function yells
 * errors if called before initialization or more then once.
 */

static int lTerminate(lua_State *L) {

    if (lib_status == NOT_INITIALIZED)
        luaL_error(L, "eSpeak was not initialized yet.");

    if (lib_status == TERMINATED)
        luaL_error(L, "eSpeak was already terminated.");

    if (ref_synth_callback != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, ref_synth_callback);
        ref_synth_callback = LUA_NOREF;
    }

    if (ref_uri_callback != LUA_NOREF) {
        luaL_unref(L, LUA_REGISTRYINDEX, ref_uri_callback);
        ref_uri_callback = LUA_NOREF;
    }

    lib_status = TERMINATED;
    lua_pushboolean(L, espeak_Terminate());
    return 1;
}


static const luaL_reg funcs[] = {
    { "Initialize",         lInitialize },
    { "Info",               lInfo },
    { "SetSynthCallback",   lSetSynthCallback },
    { "SetUriCallback",     lSetUriCallback },
    { "Synth",              lSynth },
    { "Synth_Mark",         lSynth_Mark },
    { "Key",                lKey },
    { "Char",               lChar },
    { "SetParameter",       lSetParameter },
    { "GetParameter",       lGetParameter },
    { "SetPunctuationList", lSetPunctuationList },
    { "SetPhonemeTrace",    lSetPhonemeTrace },
    { "CompileDictionary",  lCompileDictionary },
    { "ListVoices",         lListVoices },
    { "SetVoiceByName",     lSetVoiceByName },
    { "SetVoiceByProperties", lSetVoiceByProperties },
    { "GetCurrentVoice",    lGetCurrentVoice },
    { "Synchronize",        lSynchronize },
    { "Cancel",             lCancel },
    { "IsPlaying",          lIsPlaying },
    { "Terminate",          lTerminate },
    { NULL, NULL }
};


int luaopen_espeak(lua_State *L) {
    luaL_register(L, LIB_NAME, funcs);
    constants(L);
    return 1;
}

