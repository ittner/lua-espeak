#!/usr/bin/env lua
-- -*- coding: utf-8 -*-

-- Writes speech to a audio file.
-- (c) 2007 Alexandre Erwin Ittner <aittner@netuno.com.br>
-- Distributed under the GPL v2 or later.

require "espeak"
require "waveout"

local hz = espeak.Initialize(espeak.AUDIO_OUTPUT_RETRIEVAL, 10000)

if not arg[1] then
    print("Syntax: lua test_file.lua <filename>")
    espeak.Terminate()
    return 1
end

local afp = waveout.open(arg[1], false, 1, 1, hz, 16)
if not afp then
    print("ERROR: Can't open file for writing.")
    espeak.Terminate()
    return 1
end

espeak.SetSynthCallback(function(wave, event)
    if event.type == espeak.EVENT_WORD
    or event.type == espeak.EVENT_SENTENCE then
        afp:feed(wave)
    end
    return false    -- Do not stops the synthesis.
end)

espeak.Synth("This is just a little test.")
espeak.Synchronize()

afp:close()
espeak.Terminate()

