-- -*- coding: utf-8 -*-
--
-- Functions to "say" the time of the day.
-- (c) 2007-08 Alexandre Erwin Ittner <aittner@gmail.com>
--
-- This file is part of Lua-eSpeak and is distributed under the GNU GPL v2
-- or, at your option, any later version.
--
--

module("saytime", package.seeall)

-- Spoken Portuguese

local pt_hs = { "uma", "duas", "três", "quatro", "cinco", "seis", "sete",
    "oito", "nove", "dez", "onze" }

function pt_spoken(h, m)
    local mt
    if m > 30 then
        h = h + 1
        if h == 24 then
            return (60 - m) .. " para a meia-noite"
        elseif h == 1 then
            return (60 - m) .. " para a uma da manhã"
        elseif h > 1 and h < 12 then
            return (60 - m) ..  " para as " .. pt_hs[h] .. " da manhã"
        elseif h == 12 then
            return (60 - m) .. " para o meio-dia"
        elseif h == 13 then
            return (60 - m) .. " para a uma da tarde"
        elseif h > 13 and h < 19 then
            return (60 - m) ..  " para as " .. pt_hs[h - 12] .. " da tarde"
        else
            return (60 - m) ..  " para as " .. pt_hs[h - 12] .. " da noite"
        end
    else
        if m == 30 then
            if h == 0 then
                return "meia-noite e meia"
            elseif h == 12 then
                return "meio-dia e meio"
            elseif h > 12 then
                mt = pt_hs[h - 12] .. " e meia"
            else
                mt = pt_hs[h] .. " e meia"
            end
        else
            if h == 0 then
                mt = "meia-noite"
            elseif h == 12 then
                mt = "meio-dia"
            elseif h > 12 then
                mt = pt_hs[h - 12]
            else
                mt = pt_hs[h]
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
        elseif h > 12 and h < 19 then
            return mt .. " da tarde"
        else
            return mt .. " da noite"
        end
    end
end


-- Formal portuguese.

function pt_formal(h, m)
    local hs = ""
    if h == 0 then
        hs = "zero hora"
    elseif h == 1 then
        hs = "uma hora"
    else
        hs = h .. " horas"
    end
    if m == 1 then
        hs = hs .. " e um minuto"
    elseif m > 1 then
        hs = hs .. " e " .. m .. "minutos"
    end
    return hs
end
