Developing FreeFrame plugins

Included is the latest test host app - Plugin Tester.exe

The host requires plugins to be in a plugins directory off the 
one its in.  I set up my project to compile dlls to this directory for easy debugging.
There is also a version of Vjamm you can use to test.

When you run the host you need to browse to an avi.  

You can use the Plugin Tester in automatic mode, where it auto loads your AVI for you, and loads
the first plugin in the list, or if you are debugging a plugin, you can step through the FreeFrame
function calls using the buttons. The first stage is to 'open avi'
the down arrow cycles through the buttons in the host in the right order to correctly initialise the 
plugin.

The  API specification is in specification.txt

It's pretty easy to create a FreeFrame plugin.  If you re-implement FreeFrameSample.h and 
FreeFrameSample.cpp with new names for your plugin. You shouldn't need to alter FreeFrame.h or 
FreeFrame.cpp except to include your header.  You will also need to edit FreeFrame.def to expose the 
'plugMain' function correctly in the DLL. 


If you know about CVS everything's available at www.sourceforge.net/projects/freeframe.

All my plugins are being released under the LGPL open source license. The interface source files 
(FreeFrame.h and FreeFrame.c) are under the BSD License so you can use these in your plugins, free 
or not!  Obviously it's up to you what license you use.  

Good Luck

FreeFramers