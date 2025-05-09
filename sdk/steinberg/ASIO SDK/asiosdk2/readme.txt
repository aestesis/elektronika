ASIO 2.0 SDK Contents
---------------------

readme.txt                   - this file
ASIO SDK 2.pdf               - ASIO SDK 2.0 specification
ASIO Licensing Agreement.rtf - Licencing Agreement
mac.sea.hqx                  - Macintosh CodeWarrior Pro 5 Projects
                               After de-Binhexing and Unstuffing the
                               mac folder should be on this same directory level

common:
asio.h                 - ASIO C definition
iasiodrv.h             - interface definition for the ASIO driver class
asio.cpp               - asio host interface (not used on Mac)
asiodrvr.h
asiodrvr.cpp           - ASIO driver class base definition
combase.h
combase.cpp            - COM base definitions (PC only)
dllentry.cpp           - DLL functions (PC only)
register.cpp           - driver self registration functionality
wxdebug.h
debugmessage.cpp       - some debugging help

host:
asiodrivers.h
asiodrivers.cpp         - ASIO driver managment (enumeration and instantiation)
ASIOConvertSamples.h
ASIOConvertSamples.cpp  - sample data format conversion class
ginclude.h              - platform specific definitions

host/mac:
asioshlib.cpp          - asio.cpp for the Mac, resolves the symbols
codefragments.hpp
codefragments.cpp      - code fragment loader

host/pc:
asiolist.h
asiolist.cpp           - instantiates an ASIO driver via the COM model

host/sample:
hostsample.cpp         - a simple console app which shows ASIO hosting
hostsample.dsp         - MSVC++ 5.0 project

driver/asiosample:
asiosmpl.h
asiosmpl.cpp           - ASIO 2.0 sample driver
wintimer.cpp           - bufferSwitch() wakeup thread (Windows)
asiosample.def         - Windows DLL module export definition
mactimer.cpp           - bufferSwitch() wakeup thread (Macintosh)
macnanosecs.cpp        - Macintosh system reference time
makesamp.cpp           - Macintosh driver object instantiation

driver/asiosample/asiosample:
asiosample.dsp        - MSVC++ 5.0 project
