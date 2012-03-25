==================
NekoAmp 1.2 readme
==================

NekoAmp is an MPEG-1 layer II/III decoder library.  It accepts a stream from
any source and decodes to 16-bit samples in memory.  The code is class-based,
and multiple decoders can be opened and operated without problems.  It is
intended primarily as a replacement in my VirtualDub program for the Amp-based
decoder that I had been using previously, and has the same interface.  For
those of you with older VirtualDub source, or even possibly my SpectrumAmp
front end, this decoder is drop-in compatible with Amplib.

NekoAmp doesn't decode Layer I, VBR, or MPEG-2 files.  Speed on a Celeron 300
seems to be a bit faster than Amplib (Amp 0.7.6) at layer II decoding, and a
bit slower at layer III.  SpectrumAmp's signal processing seems to indicate
that NekoAmp is comparable in quality to Amp and Nitrane.  I'm not an
audiophile, though, so maybe those of you with massive subwoofers and tweeters
can tell me what I'm missing.

The current layer III framework is a heavily modified version of the one from
version 4.1 of the public MPEG-1 decoder source.  IMDCT and polyphase routines
are optimized versions of FreeAmp routines.


Changes
=======
1.3	Fixed a minor layer III decoding bug that caused 'incomplete frame'
	errors on files with specific count1 area patterns.

1.2	Fixed a major buffer size bug that was screwing up layer II at 256kbps
	or higher and layer III as low as 160kbps, depending on the sampling
	rate.  Thanks to Stephen Botzko for catching this one!



legal schtuff
=============
NekoAmp is Copyright ©1999 Avery Lee, All Rights Reserved.  NekoAmp is
released with NO WARRANTY and freely usable/distributable under the terms of
the GNU General Public License (GPL).  It should have been included with
NekoAmp; if not, you may receive a copy by writing to the Free Software
Foundation, Inc., 675 Mass Ave., Cambridge, MA 02139, USA.

Any trademarks mentioned here are the property of their owners.  To the
author's knowledge no trademark or patent infringement exists in this
document or in the NekoAmp distribution; any such infringement is purely
unintentional.
