#!/usr/bin/env lua

-- Makes documentation for lua-eSpeak.

module("docparse", package.seeall)

local function escape(str)
    str = tostring(str)
    str = str:gsub("&", "&amp;")
    str = str:gsub("<", "&lt;")
    str = str:gsub(">", "&gt;")
    return str
end

function parse(ifp, ofp)
    local tag, mark, text, ending

    for line in ifp:lines() do
        if tag == nil then
            mark, text, ending = line:match("%s*/%*(!+)%s*(%S.*%S)%s*(%*/)")
            if not ending then
                mark, text = line:match("%s*/%*(!+)%s*(%S.*%S)")
            end
            if mark == "!!!" then
                tag = "h2"
            elseif mark == "!!" then
                tag = "h3"
            elseif mark == "!" then
                tag = "h4"
            end
            if tag and text then
                ofp:write('<a name="api.', text:lower(), '"></a>\n',
                    "<", tag, ">", text, "</", tag, ">\n")
                if ending then
                    tag = nil
                else
                    tag = "pre"
                    ofp:write("<", tag, ">")
                end
            end
        elseif line:match("%s*%*/") then
            ofp:write("</", tag, ">\n")
            tag = nil
        else
            mark, text = line:match("%s*(%*)%s(.*)")
            ofp:write(escape(text or ""), "\n")
        end
    end
end


function echo(fname, ofp)
    for line in io.lines(fname) do
        ofp:write(escape(line or ""), "\n")
    end
end

