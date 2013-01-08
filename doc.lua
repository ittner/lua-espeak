require "docparse"

print [[
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
 <head>
  <title>Lua-eSpeak Reference Manual</title>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
  <style type="text/css">
<!--
H1 {
    font-family: Verdana, Arial, helvetica, sans-serif;
    font-size: 18px;
    color: #000000;
    background-color: #FFFFFF;
    text-align: center;
}    

H2 {
    font-family: Verdana, Arial, helvetica, sans-serif;
    font-size: 16px;
    color: #000000;
    background-color: #EAEAEA;
}    

H3 {
    font-family: Verdana, Arial, helvetica, sans-serif;
    font-size: 14px;
    color: #000000;
    background-color: #EAEAEA;
}    

H4 {
    font-family: Verdana, Arial, helvetica, sans-serif;
    font-size: 12px;
    color: #000000;
}    

BODY  {
    font-family: Verdana, Arial, helvetica, sans-serif;
    font-size: 12px;
    color: #000000;
    background: #FFFFFF;
}

A {
    font-family: Verdana, Arial, helvetica, sans-serif;
    font-size: 12px;
    color: #000088;
    text-decoration: none;
}

A:HOVER {
    text-decoration: underline;
    color: #FF0000;
    background-color: #EAEAEA;
}

HR {
    border: 1px solid #EAEAEA;
}

TABLE {
    border: 0px;
}

TR, TD {
    font-family: Verdana, Arial, helvetica, sans-serif;
    font-size: 12px;
    border: 1px solid #EAEAEA;
    background-color: #FFFFFF;
    padding: 0px;
    cell-spacing: 1px;
}

TR.h, TD.h {
    text-align: center;
    font-weight: bold;
}

TR.i, TD.i {
    text-align: center;
}

TR.n, TD.n {
    font-size: 12px;
    font-family: "Courier New", courier, monospace;
}


pre
{
    font-family: "Courier New", courier, monospace;
    color: black;
}

pre.example
{
    border: 1px solid rgb(128, 128, 128);
    padding: 5pt;
    display: block;
    font-family: "Courier New", courier, monospace;
    background-color: #F0F0F0;
    margin-left: 2%;
    margin-right: 2%;
}

pre.example2
{
    border: 1px solid rgb(128, 128, 128);
    padding: 5pt;
    display: block;
    font-family: "Courier New", courier, monospace;
    background-color: #F0F0F0;
}

pre.wrong
{
    border: gray 1pt solid;
    padding: 2pt;
    display: block;
    font-family: "Courier New", courier, monospace;
    background-color: #FF0000;
    color: black;
}

-->
 </style>
</head>

<body>

<center>
 <a href="http://lua-espeak.luaforge.net"
  ><img src="lua-espeak.png" alt="Lua-eSpeak Logo" title="Lua-eSpeak logo" border="0"></a>
 <h1>Lua-eSpeak 1.36r1</h1>
 <h1>DRAFT</h1>
</center>

<h2> Contents </h2>
<ul>
 <li><a href="#intro">Introduction</a>
 <li><a href="#license">Licensing information</a></li>
 <li><a href="#download">Download and installation</a></li>
 <li><a href="#load">Library loading and initialization</a></li>
 <li><a href="#api">API</a>
   <ul>
     <li><a href="#api.constants">Constants</a></li>
     <li><a href="#api.initialization">Initialization</a></li>
     <li><a href="#api.synthesis">Synthesis</a></li>
     <li><a href="#api.speech parameters">Speech parameters</a></li>
     <li><a href="#api.voice selection">Voice selection</a></li>
     <li><a href="#api.flow control">Flow control</a></li>
   </ul>
 </li>
 <li><a href="#examples">Examples</a>
   <ul>
     <li><a href="#examples.thering">A simple speech</a></li>
     <li><a href="#examples.voices">Changing the voice</a></li>
     <li><a href="#examples.saytime">Telling the time</a></li>
     <li><a href="#examples.other">Other examples</a></li>
   </ul>
 </li>
 <li><a href="#contact">Contact information</a></li>
</ul>


<a name="intro"></a>
<h2>Introduction</h2>

<p><a href="http://espeak.sourceforge.net">eSpeak</a> is a compact open
source software speech synthesizer for English and other languages. It
produces good quality English speech using a different synthesis method
from other open source TTS engines. </p>

<p> Lua-eSpeak is a "binding": a library that exports functions from eSpeak
to the <a href="http://www.lua.org/">Lua Programming Language</a>, allowing
you to use eSpeak from Lua. The API was <b>NOT</b> literally exported, but
changed in a way that made it familiar to Lua users. </p>

<p> Lua-eSpeak is a programming library, not a synthesis program. If you are
looking for that or are not familiar to the Lua Programming Language, you
are in the wrong place. </p>

<p><b>A NOTE ON VERSION NUMBERS:</b> Lua-eSpeak version numbers are in the
format "X.YrZ", where X.Y indicates the eSpeak version and Z the version of
the binding. So, the version <b>1.36r1</b> is the first version of the
binding for eSpeak 1.36 and <b>1.36r2</b> has some improvements/bug fixes/etc.
but uses the same eSpeak version. </p>


<a name="license"></a>
<h2>Licensing information</h2>

<p> eSpeak and Lua-eSpeak are <em>copyrighted free software</em>, distributed
under the <a href="http://www.fsf.org/licensing/licenses/gpl.html">GNU General
Public License</a> version 3 or, at your option, any later version. It can be
used for both academic and commercial purpouses. <b>BUT</b>, if you
redistribute the library, you must make all source code linking against
this library and Lua interpreter available under the GPL too; This requirement
does not include your Lua code. More precisely: </p>

<blockquote>
<pre>
(c) 2007-08 Alexandre Erwin Ittner &lt;alexandre X ittner.com.br&gt;

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
</pre>
</blockquote>


<a name="download"></a>
<a name="installation"></a>
<h2>Download and installation</h2>

<p> Lua-eSpeak source code comes in a tar.gz package that can be downloaded
from <a href="http://luaforge.net/projects/lua-espeak/">Lua-eSpeak project
page</a> on <a href="http://www.luaforge.net">LuaForge</a>. There are also
some pre-compiled binary packages available for Linux and some other Unix
systems.</p>

<p> After downloading, you need to compile the library. This process requires
GNU Make, Lua, eSpeak and its dependencies (portaudio, etc.) installed on your
system. To compile on Unix, just unpack the distribution package, enter the
directory and type <code>make</code> in your favourite shell. </p>

<p> To install the library with <b>Lua 5.1</b> and above just type
<code>make install</code>, as root. </p>

<p> <b> WARNING:</b> This procedure will work only in systems with pkg-config
(which includes the major Linux distributions). In other systems, you will
need to edit the Makefile and manually install the library, by copying the
file 'espeak.so' to your Lua binary modules directory.</p>

<p> Compiling Lua-eSpeak on Windows systems is possible, but I have not
tried yet. You will need to edit the Makefile by yourself. </p>

<a name="load"></a>
<h2>Library loading and initialization</h2>

<p> Lua-eSpeak uses the Lua 5.1 package system that allows you to simply do a</p>

<pre class="example">require "espeak"</pre>

<p> call to load up the library. After that, you must call the
<code>espeak.Initialize</code> function before using the library.</p>

<a name="api"></a>
]]

fp = io.open("luaespeak.c", "r")
assert(fp, "Can't open luaespeak.c")
docparse.parse(fp, io.stdout)
fp:close()


print [[
<a name="examples"></a>
<h2>Examples</h2>


<a name="examples.thering"></a>
<h3>A simple speech</h3>
<p> This program utters a phrase and quits. You can listen to the resulting
audio <a href="ring.mp3">here</a>. </p>


<pre class="example2">]]

docparse.echo("demos/ring.lua", io.stdout)

print [[</pre>


<a name="examples.voices"></a>
<h3>Changing the voice</h3>
<p> This program speaks with some voices available in eSpeak. You can listen
to the resulting audio <a href="voices.mp3">here</a>. </p>

<pre class="example2">]]

docparse.echo("demos/set_voice_by_properties.lua", io.stdout)

print [[</pre>

<a name="examples.saytime"></a>
<h3>Telling the time</h3>
<p> This program speaks the current time in the "spoken" (informal) Brazilian
Portuguese. You can listen to the resulting audio
<a href="saytime.mp3">here</a>. </p>

<pre class="example2">]]

docparse.echo("demos/saytime.lua", io.stdout)

print [[</pre>

<pre class="example2">]]

docparse.echo("demos/clock_pt.lua", io.stdout)

print [[</pre>

<a name="examples.other"></a>
<h3>Other examples</h3>
<p> There are some useful examples in the <code>demos</code> directory
within the distribution package. </p>

<a name="contact"></a>
<h2>Contact information</h2>

<p>
Author: <b>Alexandre Erwin Ittner</b>
<br> E-mail: <a href="mailto:alexandre#ittner.com.br">alexandre<b>#</b>ittner.com.br</a>
(e-mail obfuscated to avoid spam-bots. Please replace the "#" with an "@").

<br> GnuPG/PGP Key: <a href="http://www.ittner.com.br/AlexandreErwinIttner.pub.asc">0x0041A1FB</a>
 (key fingerprint: <code>9B49 FCE2 E6B9 D1AD 6101  29AD 4F6D F114 0041 A1FB</code>).
<br> Homepage: <a href="http://www.ittner.com.br/">http://www.ittner.com.br/</a>.
<br> Location: Jaraguá do Sul, Santa Catarina, Brazil.
</p>

</body>
</html>
]]

