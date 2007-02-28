require "espeak"

local event_types = {
    [espeak.EVENT_LIST_TERMINATED] = "LIST_TERMINATED",
    [espeak.EVENT_WORD] = "WORD",
    [espeak.EVENT_SENTENCE] = "SENTENCE",
    [espeak.EVENT_MARK] = "MARK",
    [espeak.EVENT_PLAY] = "PLAY",
    [espeak.EVENT_END] = "END",
    [espeak.EVENT_MSG_TERMINATED] = "MSG_TERMINATED"
}

io.write("espeak.VERSION = ", espeak.VERSION or "Ooops", "\n")
io.write("espeak.Info() = ", espeak.Info(), "\n")
espeak.Initialize(espeak.AUDIO_OUTPUT_RETRIEVAL, 10000)

espeak.SetSynthCallback(function(wave, event)
    io.write("test_espeak.lua: event received. Type = ",
        event_types[event.type] or event.type,
        ", id = ", event.unique_identifier or "none")
    if wave then
        io.write(", data length = ", wave:len(), "\n")
    else
        io.write(", no data\n")
    end
    return false    -- Do not stops the synthesis.
end)

espeak.Synth("Remember, remember, the fifth of November.", 0, espeak.POS_WORD, 0, nil)
espeak.Synchronize()
espeak.Terminate()

