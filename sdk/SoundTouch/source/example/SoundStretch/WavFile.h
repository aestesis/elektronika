/******************************************************************************
 *
 * Classes for easy reading & writing of WAV sound files.
 *
 * For big-endian CPU, define BIG_ENDIAN during compile-time to correctly
 * parse the WAV files with such processors.
 * 
 * Admittingly, more complete WAV reader routines may exist in public domain, but 
 * the reason for 'yet another' one is that those generic WAV reader libraries are
 * exhaustingly large and cumbersome! Wanted to have something simpler here, i.e. 
 * something that's not already larger than rest of the SoundTouch/SoundStretch program...
 *
 * Author        : Copyright (c) Olli Parviainen
 * Author e-mail : oparviai @ iki.fi
 * File created  : 13-Jan-2002
 *
 * Last changed  : $Date: 2003/09/21 10:50:40 $
 * File revision : $Revision: 1.4 $
 *
 * $Id: WavFile.h,v 1.4 2003/09/21 10:50:40 Olli Exp $
 *
 * License :
 *
 *  SoundTouch sound processing library
 *  Copyright (c) Olli Parviainen
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *****************************************************************************/

#ifndef WAVFILE_H
#define WAVFILE_H

#include <stdio.h>

#ifndef uint
typedef unsigned int uint;
#endif           


/// WAV audio file 'riff' section header
typedef struct 
{
    char riff_char[4];
    int  package_len;
    char wave[4];
} WavRiff;

/// WAV audio file 'format' section header
typedef struct 
{
    char  fmt[4];
    int   format_len;
    short fixed;
    short channel_number;
    int   sample_rate;
    int   byte_rate;
    short byte_per_sample;
    short bits_per_sample;
} WavFormat;

/// WAV audio file 'data' section header
typedef struct 
{
    char  data_field[4];
    uint  data_len;
} WavData;


/// WAV audio file header
typedef struct 
{
    WavRiff   riff;
    WavFormat format;
    WavData   data;
} WavHeader;


/// Class for reading WAV audio files.
class WavInFile
{
private:
    /// File pointer.
    FILE *fptr;

    /// Counter of how many bytes of sample data have been read from the file.
    uint dataRead;

    /// WAV header information
    WavHeader header;

    /// Read WAV file headers.
    /// \return zero if all ok, nonzero if file format is invalid.
    int readWavHeaders();

    /// Checks WAV file header tags.
    /// \return zero if all ok, nonzero if file format is invalid.
    int checkCharTags();

    /// Reads a single WAV file header block.
    /// \return zero if all ok, nonzero if file format is invalid.
    int readHeaderBlock();

    /// Reads WAV file 'riff' block
    int readRIFFBlock();

public:
    /// Constructor: Opens the given WAV file. If the file can't be opened,
    /// throws 'runtime_error' exception.
    WavInFile(const char *filename);

    /// Destructor: Closes the file.
    ~WavInFile();

    /// Close the file. Notice that file is automatically closed also when the 
    /// class instance is deleted.
    void close();

    /// Rewind to beginning of the file
    void rewind();

    /// Get sample rate.
    uint getSampleRate() const;

    /// Get number of bits per sample, i.e. 8 or 16.
    uint getNumBits() const;

    /// Get sample data size in bytes. Ahem, this should return same information as 
    /// 'getBytesPerSample'...
    uint getDataSizeInBytes() const;

    /// Get total number of samples in file.
    uint getNumSamples() const;

    /// Get number of bytes per audio sample (e.g. 16bit stereo = 4 bytes/sample)
    uint getBytesPerSample() const;
    
    /// Get number of audio channels in the file (1=mono, 2=stereo)
    uint getNumChannels() const;

    /// Get the audio file length in milliseconds
    uint getLengthMS() const;

    /// Reads audio samples from the WAV file. Reads given number of bytes from the file.
    /// or if end-of-file reached, as many bytes as are left in the file.
    ///
    /// \return Number of _bytes_ read from the file.
    int read(void *buffer,                  ///< Pointer to buffer where to read data.
             const int bufferSizeInBytes    ///< Size of buffer. Reads at most these many bytes.
             );

    /// Check end-of-file.
    ///
    /// \return Nonzero if end-of-file reached.
    int eof() const;
};


/// Class for writing WAV audio files.
class WavOutFile
{
private:
    /// Pointer to the WAV file
    FILE *fptr;

    /// WAV file header data.
    WavHeader header;

    /// Counter of how many bytes have been written to the file so far.
    int bytesWritten;

    /// Fills in WAV file header information.
    void fillInHeader(const uint sampleRate, const uint bits, const uint channels);

    /// Finishes the WAV file header by supplementing information of amount of
    /// data written to file etc
    void finishHeader();

    /// Writes the WAV file header.
    void writeHeader();

public:
    /// Constructor: Creates a new WAV file. Throws a 'runtime_error' exception 
    /// if file creation fails.
    WavOutFile(const char *fileName,    ///< Filename
               const int sampleRate,    ///< Sample rate (e.g. 44100 etc)
               const int bits,          ///< Bits per sample (8 or 16 bits)
               const int channels       ///< Number of channels (1=mono, 2=stereo)
               );

    /// Destructor: Finalizes & closes the WAV file.
    ~WavOutFile();

    /// Write data to WAV file. Throws a 'runtime_error' exception if writing to
    /// file fails.
    void write(const void *buffer,      ///< Pointer to sample data buffer.
               const int numBytes       ///< How many _bytes_ of data are to be written to file.
               );

    /// Finalize & close the WAV file. Automatically supplements the WAV file header
    /// information according to written data etc.
    ///
    /// Notice that file is automatically closed also when the class instance is deleted.
    void close();
};

#endif
