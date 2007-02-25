
require "espeak"

local phr = {
    {
        voice = "brazil",
        lang = "Brazilian Portuguese",
        ann = "A sample in Brazilian Portuguese.",
        out = "Backing to the main menu.",
        poem = {
            "Três anéis para os Reis-Elfos sob este céu,",
            "Sete para os Senhores-Anões em seus rochosos corredores,",
            "Nove para Homens Mortais, fadados ao eterno sono,",
            "Um para o Senhor do Escuro em seu escuro trono",
            "Na Terra de Mordor onde as Sombras se deitam.",
            '<break strength="strong" />',
            "Um anel para a todos governar, Um anel para encontrá-los,",
            "Um anel para a todos trazer e na escuridão aprisioná-los",
            "Na Terra de Mordor onde as sombras se deitam."
        }
    },
    {
        voice = "english",
        lang = "English",
        ann = "A sample in English.",
        out = "Backing to the main menu.",
        poem = {
            "Three Rings for the Elven kings under the sky,",
            "Seven for the Dwarf-lords in their halls of stone,",
            "Nine for Mortal Men, doomed to die,",
            "One for the Dark Lord on his Dark Throne,",
            "In the land of Mordor where the shadows lie,",
            '<break strength="strong" />',
            "One Ring to rule them all,",
            "One Ring to find them,",
            "One ring to bring them all,",
            "and in the darkness bind them,",
            "In the Land of Mordor where the shadows lie.",
        }
    },
    {
        voice = "german",
        lang = "German",
        ann = "A sample in German.",
        out = "Backing to the main menu.",
        poem = {
            "Drei Ringe den Elbenkönigen hoch im Licht,",
            "Sieben den Zwergenherrschern in ihren Hallen aus Stein,",
            "Den Sterblichen, ewig dem Tode verfallen, neun,",
            "Einer dem Dunklen Herrn auf dunklem Thron",
            "Im Lande Mordor, wo die Schatten drohn.",
            '<break strength="strong" />',
            "Ein Ring, sie zu knechten, sie alle zu finden,",
            "Ins Dunkel zu treiben und ewig zu bindenv",
            "Im Lande Mordor, wo die Schatten drohn."
        }
    },
    {
        voice = "esperanto",
        lang = "Esperanto",
        ann = "A sample in Esperanto.",
        out = "Backing to the main menu.",
        poem = {
            "Tri ringoj por la elfoj sub la hela ĉiel',",
            "Sep por la gnomoj en salonoj el ŝton'.",
            "Naŭ por la homoj sub la morto-sigel',",
            "Unu por la Nigra Reĝo sur la nigra tron'",
            "Kie kuŝas Ombroj en Mordora Land'.",
            '<break strength="strong" />',
            "Unu Ringo ilin regas, Unu ilin prenas,",
            "Unu Ringo en mallumon ilin gvidas kaj katenas",
            "Kie kuŝas Ombroj en Mordora Land'."
        }
    },
    {
        voice = "italian",
        lang = "Italian",
        ann = "A sample in Italian.",
        out = "Backing to the main menu.",
        poem = {
            "Tre anelli per i re di Elven sotto il cielo",
            "sette per i signori nani nei loro corridoi della pietra",
            "nove per gli uomini mortali, condannata per morire",
            "uno per il signore scuro, sul suo anello scuro del throne",
            '<break strength="strong" />',
            "uno per regolarli tutti, un anello per trovarli",
            "un anello per portarli tutti e nella nerezza li legano,",
            "nella terra di Mordor, in cui le ombre si trovano."
        }
    }
}

-- Prints and synths a string
local function say(str)
    print(str)
    espeak.Synth(str, 0, espeak.POS_WORD, 0, espeak.SSML)
end



espeak.Initialize(espeak.AUDIO_OUTPUT_PLAYBACK, 500)

if espeak.SetVoiceByName("english") ~= espeak.EE_OK then
    print("Failed to set default voice.")
    return
end

local opt = -1
say("Welcome to Lua eSpeak voice menu demonstration.")
while true do
    for k, v in ipairs(phr) do
        if k == 1 then
            say("Type " .. k .. " to listen to the sample in " .. v.lang .. ",")
        else
            say(k .. " to " .. v.lang .. ",")
        end
    end
    say("or type 0 to exit.")
    opt = tonumber(io.read("*l"))
    espeak.Cancel()
    if opt == 0 then
        say("Good bye.") 
        break
    elseif phr[opt] then
        say(phr[opt].ann)
        -- espeak.Synchronize()
        if espeak.SetVoiceByName(phr[opt].voice) == espeak.EE_OK then
            for k, v in ipairs(phr[opt].poem) do
                say(v)
                -- add pause here.
            end
            say('<break strength="x-strong" />')
        else
            espeak.SetVoiceByName("english")
            print("Failed to set this voice.")
        end
        espeak.SetVoiceByName("english")
        say(phr[opt].out)
    else
        espeak.SetVoiceByName("english")
        say("An invalid option was typed.")
    end
end

espeak.Synchronize()
espeak.Terminate()

