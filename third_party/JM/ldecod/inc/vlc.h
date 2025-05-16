
/*!
 ************************************************************************
 * \file vlc.h
 *
 * \brief
 *    header for (CA)VLC coding functions
 *
 * \author
 *    Karsten Suehring
 *
 ************************************************************************
 */

#ifndef _VLC_H_
#define _VLC_H_

#include "Entropy.h"

#define SE_HEADER         0
typedef unsigned char byte;
typedef struct bit_stream_dec Bitstream;
//! Bitstream
struct bit_stream_dec
{
    // CABAC Decoding
    int           read_len;           //!< actual position in the codebuffer, CABAC only
    int           code_len;           //!< overall codebuffer length, CABAC only
    // CAVLC Decoding
    int64_t       frame_bitoffset;    //!< actual position in the codebuffer, bit-oriented, CAVLC only
    int64_t       bitstream_length;   //!< over codebuffer lnegth, byte oriented, CAVLC only
    // ErrorConcealment
    byte          *streamBuffer;      //!< actual codebuffer for read bytes
    int           ei_flag;            //!< error indication, 0: no error, else unspecified error
};

typedef struct syntaxelement_dec
{
    int           type;                  //!< type of syntax element for data part.
    int           value1;                //!< numerical value of syntax element
    int           value2;                //!< for blocked symbols, e.g. run/level
    int           len;                   //!< length of code
    int           inf;                   //!< info part of CAVLC code
    unsigned int  bitpattern;            //!< CAVLC bitpattern
    int           context;               //!< CABAC context
    int           k;                     //!< CABAC context for coeff_count,uv

  //! for mapping of CAVLC to syntaxElement
    void(*mapping)(int len, int info, int *value1, int *value2);
} SyntaxElement;


extern int read_se_v (Bitstream *bitstream);
extern int read_u_v (int LenInBits, Bitstream *bitstream);

// CAVLC mapping
extern void linfo_ue(int len, int info, int *value1, int *dummy);
extern void linfo_se(int len, int info, int *value1, int *dummy);

extern int readSyntaxElement_FLC(SyntaxElement *sym, Bitstream *currStream);
extern int readSyntaxElement_VLC (SyntaxElement *sym, Bitstream *currStream);
extern int GetBits(byte buffer[], int64_t totbitoffset, int *info, int64_t bitcount, int numbits);

#endif

