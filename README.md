This is 3GPP2's reference QCELP encoder, fixed up to compile on modern (2015) compilers and enhanced to read and write [RFC 3625](https://tools.ietf.org/html/rfc3625) QCP files (the usual format for QCELP files). It should work on Linux with GCC (just `make`), Mac with the supplied LLVM and Windows with Visual Studio 2015 (see the 'windows' branch).

This original code is the C.R0020-A v1.0 software distribution from 3gpp2.org: I can't deep link, but search for category 'C' on [the specifications page](http://www.3gpp2.org/Public_html/specs/index.cfm) and then search for '0020' in the results. I imported the latest version there, dated April 2005, but the code itself has dates in March 2004. This is actually the master definition of QCELP: the standard, available as a PDF from the same location, defers to the C implementation where unclear or conflict.

As above I have fixed it up to compile and to be useful to me as a QCELP encoder:

* miscellaneous compile fixes: include headers, extra type size definitions, stdout-not-a-constant 
* windows branch: Visual Studio 2015 project and added [the PlexFX portable implementation of getopt](http://www.plexfx.org/downloads/downloads.html)
* added io_qcp.c, a simple implementation of QCP file input and output.

Note that the QCP input and output code assumes a little-endian host. It is enabled by default but can be turned off with option `-P`. I'd guess that this code will only work when compiled for 32-bits too but I have not tested this.

The encoder still reads and writes raw audio files: these must be 16-bit host-endian, mono and 8,000 samples/second. For example you can convert a file to this format with FFmpeg:

    ffmpeg -i your_audio_file.wav -f s16le -ar 8000 -ac 1 sample.raw

To convert this to QCELP, use `-E`

    celp13k -E -i sample.raw -o sample.qcp

and to decode `-D` 

    celp13k -D -i sample.qcp -o sample-decoded.raw

and to play this raw file

    ffplay -f s16le -ar 8000 -ac 1 sample-decoded.raw

Or e.g. to force the encoder into reduced rate mode 3 (which will generate quarter-rate packets; by default the encoder won't)

    celp13k -E -i sample.raw -m 3 -o sample.qcp

The normal operating mode of this encoder is to encode then decode the same audio file to determine the effect of a round-trip on audio quality, and there are plenty of encoder options to modify rates or introduce random errors into the QCELP data stream to see how the decoder copes and how the quality suffers.

It's still possible to read and write the original 'packet' format for QCELP files (which this implementation used before my changes) with a new command-line option `-P`. This format uses a fixed 36-bytes per frame, even though QCELP is a variable-rate codec, and consists of:

* a 16-bit word containing the encoding mode for this frame (0=blank, 1=eighth rate, 2=quarter, 3=half, 4=full, 0xE=generated error frame)
* 17 16-bit words containing the packet frame data; each pair of bytes is switched compared to the documented packed byte order in the spec and compared to the order used in QCP files.

The code itself isn't very tidy, and contains multiple implementations of various components in renamed files. I've made no attempt to identify these or choose versions other than those that are built by default, nor have I tidied up the mix of formatting styles etc. There were no tests or test vectors included in the original distribution.

See LICENSE.md for the license text from the original package: the general gist is that this implementation is for reference use only without further agreement from the copyright holders. My small contributions are MIT-licensed.   
