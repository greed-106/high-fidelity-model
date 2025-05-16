
/*!
 *************************************************************************************
 * \file vlc.h
 *
 * \brief
 *    Prototypes for VLC coding funtions
 * \author
 *     Karsten Suehring
 *************************************************************************************
 */

#ifndef _VLC_H_
#define _VLC_H_

#include "Entropy.h"

typedef unsigned char byte;
typedef struct bit_stream_enc Bitstream;
struct bit_stream_enc
{
    int     buffer_size;        //!< Buffer size      
    int     byte_pos;           //!< current position in bitstream;
    int     bits_to_go;         //!< current bitcounter

    //int     stored_byte_pos;    //!< storage for position in bitstream;
    //int     stored_bits_to_go;  //!< storage for bitcounter
    //int     byte_pos_skip;      //!< storage for position in bitstream;
    //int     bits_to_go_skip;    //!< storage for bitcounter
    //int     write_flag;         //!< Bitstream contains data and needs to be written

    byte    byte_buf;           //!< current buffer for last written byte
    //byte    stored_byte_buf;    //!< storage for buffer of last written byte
    //byte    byte_buf_skip;      //!< current buffer for last written byte
    byte    *streamBuffer;      //!< actual buffer for written bytes
};

typedef struct syntaxelement_enc
{
    //int                 type;           //!< type of syntax element for data part.
    int                 value1;         //!< numerical value of syntax element
    int                 value2;         //!< for blocked symbols, e.g. run/level
    int                 len;            //!< length of code
    int                 inf;            //!< info part of UVLC code
    unsigned int        bitpattern;     //!< UVLC bitpattern
    //int                 context;        //!< CABAC context
    //!< for mapping of syntaxElement to UVLC
    //void(*mapping)(int value1, int value2, int* len_ptr, int* info_ptr);
} SyntaxElement;

extern int write_se_v (int value, Bitstream *bitstream);
extern int write_u_v  (int n, int value, Bitstream *bitstream);
void writeUVLC2buffer(SyntaxElement *se, Bitstream *currStream);
void BitstreamInit(Bitstream *bitstream, int logMemSize);
void BitstreamRelease(Bitstream *bitstream);
void BitstreamWrite(int i, Bitstream *bitstream, char* bitstreamFile);
int WriteSeqPicHeader(int frameIdx, int intraPeriod, SeqPicHeaderInfo *seqPicHeaderInfo, Bitstream *bitstream);
void WriteStuffingBits(Bitstream *bitstream);

#endif



