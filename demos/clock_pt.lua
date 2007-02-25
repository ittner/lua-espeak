#!/usr/bin/env lua
-- -*- coding: utf-8 -*-

-- Says the time of the day as in the spoken Portuguese.
-- (c) 2007 Alexandre Erwin Ittner <aittner@netuno.com.br>
-- Distributed under the GPL v2 or later.

require "espeak"
require "saytime"

espeak.Initialize(espeak.AUDIO_OUTPUT_PLAYBACK, 500)

if espeak.SetVoiceByName("brazil") ~= espeak.EE_OK then
    print("Impossível localizar a voz correta.")
    return
end

local dt = os.date("*t")
espeak.Synth(saytime.pt_spoken(dt.hour, dt.min))

espeak.Synchronize()
espeak.Terminate()

