
require "espeak"

espeak.Initialize(espeak.AUDIO_OUTPUT_PLAYBACK, 0)

local langs = {
    { "pt-br", "Português brasileiro" },
    { "pt", "Português Europeu" },
    { "en-gb", "British English" },
    { "en-us", "American English" },
    { "de", "Deutsch"  },
    { "eo", "Esperanto" },
    { "it", "Italiano" },
    { "es", "Español" },
    { "fi", "Suomi" }
}

for _, l in ipairs(langs) do
    if espeak.SetVoiceByProperties({ languages = l[1] }) == espeak.EE_OK then
        espeak.Synth(l[2])
    end
    espeak.Synchronize()
end

espeak.Terminate()
