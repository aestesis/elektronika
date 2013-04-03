OpenSSL 0.9.7a 
==============

This package contains the OpenSSL 0.9.7a binaries for Windows.  You will
find the following directories here:

  bin:      contains the openssl.exe utility.
  certs:    contains the authority certificates distributed with
            OpenSSL
  include:  contains the OpenSSL header files.  
  lib:      contains the OpenSSL libraries.   
  util:     contains the test utilities.

The files in the lib directory which answer to the name '*_a.lib' are static
libraries. The rest is for dynamic linking. 

The files openssl.cnf.txt and oid.txt are sample configuration files. 

Installation
============

If you plan to develop your own programme, in C, which uses OpenSSL, then
you should know what to do with the files in the binary package. If you
don't, know this, then please, please do some research on how to use a
third-party library in a C programme.  The topic belongs to the very basics
and you will not be able to do much without that knowledge. 

If you wish to use OpenSSL solely through the supplied utilities, such as
openssl.exe, then all you need to do is place the contents of the 'lib' and
'bin' directories from this package in a directory on your disc which is
mentioned in your PATH environment variable. You can use an existing
directory which is allready in the path, such as 'C:\WINDOWS', or
'C:\WINNT'. You can also create a new directory for OpenSSL and place the
files there, but be sure to modify the PATH environment variable and add
that new directory to its list. 

The OpenSSL configuration file is usually called openssl.cnf. Sample
configuration file is named openssl.cnf.txt in order to avoid its being
displayed as a speed-dial link. If you use it, then name it as you please
and place it where you please, but set the environment variable OPENSSL_CONF
and have its contents contain the full path to it.

If you use other software which needs OpenSSL, then please consult the
documentation of that software and see if it mentions something about how it
uses OpenSSL and how it expects it to be installed. If you find nothing,
then the default installation, as described in the previous paragraph,
should be suficient. 


Notes
=====

There is no documentation in the source which is directly usable on Windows,
so I haven't included any. if you need some, visit the OpenSSL site at
http://www.openssl.org/ and see if it contains anything that helps. You can
also visit The Linux Documentation Project at http://www.tldp.org/ and read
the SSL-Certificates-HOWTO.

If there is something you cannot keep for yourself, such as a problem, a
cheer of joy, a comment or a suggestion, feel free to contact me using the
address below. 

Igor Zlatkovic (igor@zlatkovic.com)


