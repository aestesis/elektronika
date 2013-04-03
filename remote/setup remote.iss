; -- Example1.iss --
; Demonstrates copying 3 files and creating an icon.

; SEE THE DOCUMENTATION FOR DETAILS ON CREATING .ISS SCRIPT FILES!

[Setup]
AppName=Aestesis ElekRemote
AppId=ElekRemote
AppVerName=Aestesis ElekRemote 2.12
DefaultDirName={pf}\aestesis\remote
DefaultGroupName=ElekRemote
UninstallDisplayIcon={app}\remote.exe
Compression=lzma
SolidCompression=yes

[Files]

; merge module
; Source: {#MERGEDIR}\.MSI; DestDir: {tmp}; Flags: deleteafterinstall

; microsoft
Source: "MFC80.dll"; DestDir: "{app}"
Source: "MFC80u.dll"; DestDir: "{app}"
Source: "msvcm80.dll"; DestDir: "{app}"
Source: "msvcp80.dll"; DestDir: "{app}"
Source: "msvcr80.dll"; DestDir: "{app}"


; doc
Source: "docs\remote documentation.rtf"; DestDir: "{app}\docs"

; images
Source: "images\1.png"; DestDir: "{app}\images"
Source: "images\2.png"; DestDir: "{app}\images"
Source: "images\3.png"; DestDir: "{app}\images"
Source: "images\4.png"; DestDir: "{app}\images"
Source: "images\5.png"; DestDir: "{app}\images"
Source: "images\6.png"; DestDir: "{app}\images"
Source: "images\7.png"; DestDir: "{app}\images"
Source: "images\8.png"; DestDir: "{app}\images"
Source: "images\9.png"; DestDir: "{app}\images"
Source: "images\clips.png"; DestDir: "{app}\images"
Source: "images\funk.png"; DestDir: "{app}\images"
Source: "images\mov.png"; DestDir: "{app}\images"
Source: "images\pop.png"; DestDir: "{app}\images"
Source: "images\rock.png"; DestDir: "{app}\images"
Source: "images\slow.png"; DestDir: "{app}\images"
Source: "images\tcm.png"; DestDir: "{app}\images"
Source: "images\tek.png"; DestDir: "{app}\images"


; set01/flash
Source: "set01\flash\dancers01.swf"; DestDir: "{app}\set01\flash"
Source: "set01\flash\dancers02.swf"; DestDir: "{app}\set01\flash"
Source: "set01\flash\dancers03.swf"; DestDir: "{app}\set01\flash"
Source: "set01\flash\dancers04.swf"; DestDir: "{app}\set01\flash"
Source: "set01\flash\dancers05.swf"; DestDir: "{app}\set01\flash"
Source: "set01\flash\dancers_generic_balls_collors.swf"; DestDir: "{app}\set01\flash"
Source: "set01\flash\dancers_generic_intro_black.swf"; DestDir: "{app}\set01\flash"
Source: "set01\flash\dancers_generic_new_circles.swf"; DestDir: "{app}\set01\flash"
Source: "set01\flash\dancers_intro_no_flash.swf"; DestDir: "{app}\set01\flash"
Source: "set01\flash\icons.swf"; DestDir: "{app}\set01\flash"
Source: "set01\flash\icons_full_screen.swf"; DestDir: "{app}\set01\flash"
Source: "set01\flash\rappers.swf"; DestDir: "{app}\set01\flash"
Source: "set01\flash\rappers_movies_fs.swf"; DestDir: "{app}\set01\flash"
Source: "set01\flash\symbol.swf"; DestDir: "{app}\set01\flash"
Source: "set01\flash\symbol_fs.swf"; DestDir: "{app}\set01\flash"
Source: "set01\flash\the_out_lines.swf"; DestDir: "{app}\set01\flash"


; set01/movies
Source: "set01\movies\sigraph-001-a.avi"; DestDir: "{app}\set01\movies"
Source: "set01\movies\sigraph-004.avi"; DestDir: "{app}\set01\movies"
Source: "set01\movies\vj-003.avi"; DestDir: "{app}\set01\movies"
Source: "set01\movies\vj-004.avi"; DestDir: "{app}\set01\movies"

; set01/images

Source: "set01\images\hello.png"; DestDir: "{app}\set01\images"

; set01

Source: "set01\set.eka"; DestDir: "{app}\set01"

; main

Source: "iconv.dll"; DestDir: "{app}"
Source: "alib100.dll"; DestDir: "{app}"
Source: "libxml2.dll"; DestDir: "{app}"
Source: "init.xml"; DestDir: "{app}"
Source: "remote.exe"; DestDir: "{app}"

[Icons]
Name: "{userdesktop}\ElekRemote"; Filename: "{app}\remote.exe"

Name: "{group}\ElekRemote"; Filename: "{app}\remote.exe"
Name: "{group}\documentation"; Filename: "{app}\docs\remote documentation.rtf"
Name: "{group}\uninstall"; Filename: "{uninstallexe}"





