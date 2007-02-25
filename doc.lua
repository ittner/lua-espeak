#!/usr/bin/env lua

--
--
--
--


local function parse(ifp, ofp)
    local tag
    local mark, spaces, text, ending

    for line in ifp:lines() do
        if tag == nil then
            mark, text, ending = line:match("%s*/%*(!+)%s*(%S.*)(%*/)")
            if not ending then
                mark, text = line:match("%s*/%*(!+)%s*(%S.*)")
            end
            if mark == "!!!" then
                tag = "h2"
            elseif mark == "!!" then
                tag = "h3"
            elseif mark == "!" then
                tag = "h4"
            end
            if tag and text then
                ofp:write("<", tag, ">", text, "</", tag, ">\n")
                if ending then
                    tag = nil
                else
                    tag = "p"
                    txout = nil
                    ofp:write("<p>\n")
                end
            end
        elseif line:match("%s*%*/") then
            ofp:write("</", tag, ">\n")
            tag = nil
        else
            mark, spaces, text = line:match("%s*(%*)(%s+)(.*)")
            if text then
                if spaces:len() > 1 and tag ~= "pre" then
                    ofp:write("</", tag, ">\n<pre>")
                    tag = "pre"
                elseif spaces:len() == 1 and tag == "pre" then
                    ofp:write("</pre>\n<p>")
                    tag = "p"
                end
                ofp:write(spaces, text, "\n")
            else
                mark = line:match("%s*(%*)%s*")
                ofp:write("</", tag, ">\n<p>")
                tag = "p"
            end
        end
    end

end



parse(io.stdin, io.stdout)

