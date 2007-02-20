
require "espeak"

espeak.Initialize(espeak.AUDIO_OUTPUT_PLAYBACK, 500)

local vl = espeak.ListVoices()
local gender = { "male", "female" }


for i, v in ipairs(vl) do
    io.write("Voice name: ", v.name, "\n")

    io.write("Languages:  ")
    for _, lang in ipairs(v.languages) do
        io.write(lang[2] or "?", " ")
    end
    io.write("\n")

    io.write("Gender:     ", gender[v.gender] or "none", "\n")
    io.write("Age:        ", v.age or "none", "\n")
    io.write("Identifier: ", v.identifier or "none", "\n")
    io.write("Variant:    ", v.variant or "none", "\n")
    io.write("--\n")
end

espeak.Terminate()
