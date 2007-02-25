#!/usr/bin/env lua
-- -*- coding: utf-8 -*-

-- Says the time of the day as in the spoken Portuguese.
-- (c) 2007 Alexandre Erwin Ittner <aittner@netuno.com.br>
-- Distributed under the GPL v2 or later.

require "espeak"

local function saytime(h, m)
    local hs = { "uma", "duas", "três", "quatro", "cinco", "seis", "sete",
        "oito", "nove", "dez", "onze" }

    local mt
    if m > 30 then
        h = h + 1
        if h == 24 then
            return (60 - m) .. " para a meia-noite"
        elseif h == 1 then
            return (60 - m) .. " para a uma da manhã"
        elseif h > 1 and h < 12 then
            return (60 - m) ..  " para as " .. hs[h] .. " da manhã"
        elseif h == 12 then
            return (60 - m) .. " para o meio-dia"
        elseif h == 13 then
            return (60 - m) .. " para a uma da tarde"
        elseif h > 13 and h < 19 then
            return (60 - m) ..  " para as " .. hs[h - 12] .. " da tarde"
        else
            return (60 - m) ..  " para as " .. hs[h - 12] .. " da noite"
        end
    else
        if m == 30 then
            if h == 0 then
                return "meia-noite e meia"
            elseif h == 12 then
                return "meio-dia e meio"
            elseif h > 12 then
                mt = hs[h - 12] .. " e meia"
            else
                mt = hs[h] .. " e meia"
            end
        else
            if h == 0 then
                mt = "meia-noite"
            elseif h == 12 then
                mt = "meio-dia"
            elseif h > 12 then
                mt = hs[h - 12]
            else
                mt = hs[h]
            end
            if m ~= 0 then
                mt = mt .. " e " .. m
            end
            if h == 0 or h == 12 then
                return mt
            end
        end
        if h < 12 then
            return mt .. " da manhã"
        elseif h > 13 and h < 19 then
            return mt .. " da tarde"
        else
            return mt .. " da noite"
        end
    end
end


espeak.Initialize(espeak.AUDIO_OUTPUT_PLAYBACK, 500)

if espeak.SetVoiceByName("brazil") ~= espeak.EE_OK then
    print("Impossível localizar a voz correta.")
    return
end

local dt = os.date("*t")
espeak.Synth(saytime(dt.hour, dt.min))

espeak.Synchronize()
espeak.Terminate()

