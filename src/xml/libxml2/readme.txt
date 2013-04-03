  libxml2 2.5.7
  --------------

  This is libxml2, version 2.5.7, binary package for the native Win32/IA32
platform.

  The directory named 'include' contains the header files. Place its
contents somewhere where it can be found by the compiler.
  The directory which answers to the name 'lib' contains the static and
dynamic libraries. Place them somewhere where they can be found by the
linker. The files whose names end with '_a.lib' are aimed for static
linking, the other files are lib/dll pairs.
  The directory called 'util' contains various programs which count as a
part of libxml2.

  If you plan to develop your own programme, in C, which uses libxml2, then
you should know what to do with the files in the binary package. If you don't,
know this, then please, please do some research on how to use a
third-party library in a C programme. The topic belongs to the very basics
and you will not be able to do much without that knowledge.

  If you wish to use libxml2 solely through the supplied utilities, such as
xmllint or xsltproc, then all you need to do is place the
contents of the 'lib' and 'util' directories from the binary package in a
directory on your disc which is mentioned in your PATH environment
variable. You can use an existing directory which is allready in the
path, such as 'C:WINDOWS', or 'C:WINNT'. You can also create a new
directory for libxml2 and place the files there, but be sure to modify
the PATH environment variable and add that new directory to its list.

  If you use other software which needs libxml2, such as Apache
Web Server in certain configurations, then please consult the
documentation of that software and see if it mentions something about
how it uses libxml2 and how it expects it to be installed. If you find
nothing, then the default installation, as described in the previous
paragraph, should be suficient.

  If there is something you cannot keep for yourself, such as a problem,
a cheer of joy, a comment or a suggestion, feel free to contact me using
the address below.

                              Igor Zlatkovic (igor@zlatkovic.com)
