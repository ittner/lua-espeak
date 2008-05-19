#!/usr/bin/env lua

-- A small program to play around eSpeak features.

require "espeak"

espeak.Initialize(espeak.AUDIO_OUTPUT_PLAYBACK, 500)

if espeak.SetVoiceByName("english") ~= espeak.EE_OK then
    print("Failed to set default voice.")
    return
end

local text = "There was Eru, the One, who in Arda is called Iluvatar; and "
.. "he made first the Ainur, the Holy Ones, that were the offspring of his "
.. "thought, and they were with him before aught else was made. And he "
.. "spoke to them, propounding to them themes of music; and they sang "
.. "before him, and he was glad."


local gender = { "male", "female" }

local voices = espeak.ListVoices()
local valid_voices = { }
for k, v in ipairs(voices) do
    valid_voices[v.name] = v
end

local opts = {
    r = "RATE",
    v = "VOLUME",
    p = "PITCH",
    a = "RANGE",
    u = "PUNCTUATION",
    c = "CAPITALS",
    w = "WORDGAP"
}

local opt, num, par, v

while true do
    io.write("\033c") -- Clears the screen.
    print("Lua eSpeak parameters")
    io.write("Test text: ", text or "None", "\n\n")

    v = espeak.GetCurrentVoice()
    io.write("Voice information:\n")
    io.write("  Name:       ", v.name, "\n")
    io.write("  Languages:  ")
    for _, lang in ipairs(v.languages) do
        io.write(lang[2] or "?", "(", lang[1] or "?", ") ")
    end
    io.write("\n")
    io.write("  Gender:     ", gender[v.gender] or "none", "\n")
    io.write("  Age:        ", v.age or "none", "\n")
    io.write("  Identifier: ", v.identifier or "none", "\n")
    io.write("  Variant:    ", v.variant or "none", "\n\n")

    print("Option Parameter      Default    Current")
    for k, v in pairs(opts) do
        print(string.format("   %s    %-13s %5d %10d", k, v,
            espeak.GetParameter(espeak[v], false),
            espeak.GetParameter(espeak[v], true)))
    end

    print("\n Other valid options are:")
    print(" - 's' to speaks some text and 'd' speaks the default test text")
    print(" - 'cv' to change the voice")
    print(" - 't' to changes the test text")
    print(" - Increment/decrement (+p, +1p, -20p, etc.)")

    io.write("\nOption: ")
    opt = io.read("*l")

    if opt == nil then
        print("Invalid option.")
    elseif opts[opt] then
        io.write("Enter new value for ", opts[opt], ": ")
        num = io.read("*n")
        if num then
            if espeak.SetParameter(espeak[opts[opt]], num, false) == espeak.EE_OK then
                io.write("\nNew value for ", opts[opt], " defined.\n")
            else
                print("An error has ocurred.")
            end
        else
            print("Bad value.")
        end
    elseif opt == "q" then
        break
    elseif opt == "cv" then
        io.write("New voice name: ")
        par = io.read("*l")
        if valid_voices[par] then
            espeak.SetVoiceByName(par)
        else
            print("Bad Voice")
        end
    elseif opt == "t" then
        io.write("Enter text: ")
        text = io.read("*l")
    elseif opt == "s" then
        io.write("Enter text: ")
        espeak.Synth(io.read("*l") or "You typed nothing.")
    elseif opt == "d" then
        espeak.Synth(text or "No default text selected.")
    else
        num, par = opt:match("([+-][0-9]*)([a-z])")
        if par and opts[par] then
            if num == "+" or num == "-" then
                num = tonumber(num .. "1")
            else
                num = tonumber(num)
            end
            if num then
                num = espeak.GetParameter(espeak[opts[par]], true) + num
                if espeak.SetParameter(espeak[opts[par]], num, false) == espeak.EE_OK then
                    io.write("New value for ", opts[par], " defined.\n")
                else
                    print("An error has ocurred.")
                end
            else
                print("Bad value.")
            end
        end
    end
end

espeak.Synchronize()
espeak.Terminate()

