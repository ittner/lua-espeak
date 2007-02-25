
require "espeak"

local text = "One Ring to rule them all."

espeak.Initialize(espeak.AUDIO_OUTPUT_PLAYBACK, 500)

if espeak.SetVoiceByName("english") ~= espeak.EE_OK then
    print("Failed to set default voice.")
    return
end

espeak.Synth(text, 0, espeak.POS_WORD, 0, nil)

espeak.Synchronize()
espeak.Terminate()

