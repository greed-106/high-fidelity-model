/*!
 ************************************************************************
 * \file vlc.c
 *
 * \brief
 *    VLC support functions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Inge Lille-Langoy               <inge.lille-langoy@telenor.com>
 *    - Detlev Marpe
 *    - Gabi Blaettermann
 ************************************************************************
 */

#include "vlc.h"


/*!
 *************************************************************************************
 * \brief
 *    read_ue_v, reads an se(v) syntax element, the length in bits is stored in
 *    the global p_Dec->UsedBits variable
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
int read_se_v (Bitstream *bitstream)
{
  SyntaxElement symbol;

  //assert (bitstream->streamBuffer != NULL);
  symbol.type = SE_HEADER;
  symbol.mapping = linfo_se;   // Mapping rule: signed integer
  readSyntaxElement_VLC (&symbol, bitstream);
  return symbol.value1;
}


/*!
 *************************************************************************************
 * \brief
 *    read_ue_v, reads an u(v) syntax element, the length in bits is stored in
 *    the global p_Dec->UsedBits variable
 *
 * \param LenInBits
 *    length of the syntax element
 *
 * \param tracestring
 *    the string for the trace file
 *
 * \param bitstream
 *    the stream to be read from
 *
 * \return
 *    the value of the coded syntax element
 *
 *************************************************************************************
 */
int read_u_v (int LenInBits, Bitstream *bitstream)
{
  SyntaxElement symbol;
  symbol.inf = 0;

  symbol.type = SE_HEADER;
  symbol.mapping = linfo_ue;   // Mapping rule
  symbol.len = LenInBits;

  readSyntaxElement_FLC (&symbol, bitstream);

  return symbol.inf;
}

/*!
 ************************************************************************
 * \brief
 *    mapping rule for ue(v) syntax elements
 * \par Input:
 *    lenght and info
 * \par Output:
 *    number in the code table
 ************************************************************************
 */
void linfo_ue(int len, int info, int *value1, int *dummy)
{
  *value1 = (int) (((unsigned int) 1 << (len >> 1)) + (unsigned int) (info) - 1);
}

/*!
 ************************************************************************
 * \brief
 *    mapping rule for se(v) syntax elements
 * \par Input:
 *    lenght and info
 * \par Output:
 *    signed mvd
 ************************************************************************
 */
void linfo_se(int len,  int info, int *value1, int *dummy)
{
  //assert ((len >> 1) < 32);
  unsigned int n = ((unsigned int) 1 << (len >> 1)) + (unsigned int) info - 1;
  *value1 = (n + 1) >> 1;
  if((n & 0x01) == 0)                           // lsb is signed bit
    *value1 = -*value1;
}


int GetVLCSymbol (byte buffer[],int totbitoffset,int *info, int bytecount)
{
  long byteoffset = (totbitoffset >> 3);         // byte from start of buffer
  int  bitoffset  = (7 - (totbitoffset & 0x07)); // bit from start of byte
  int  bitcounter = 1;
  int  len        = 0;
  byte *cur_byte  = &(buffer[byteoffset]);
  int  ctr_bit    = ((*cur_byte) >> (bitoffset)) & 0x01;  // control bit for current bit posision

  while (ctr_bit == 0)
  {                 // find leading 1 bit
    len++;
    bitcounter++;
    bitoffset--;
    bitoffset &= 0x07;
    cur_byte  += (bitoffset == 7);
    byteoffset+= (bitoffset == 7);      
    ctr_bit    = ((*cur_byte) >> (bitoffset)) & 0x01;
  }

  if (byteoffset + ((len + 7) >> 3) > bytecount)
    return -1;
  else
  {
    // make infoword
    int inf = 0;                          // shortest possible code is 1, then info is always 0    

    while (len--)
    {
      bitoffset --;    
      bitoffset &= 0x07;
      cur_byte  += (bitoffset == 7);
      bitcounter++;
      inf <<= 1;    
      inf |= ((*cur_byte) >> (bitoffset)) & 0x01;
    }

    *info = inf;
    return bitcounter;           // return absolute offset in bit from start of frame
  }
}


/*!
 ************************************************************************
 * \brief
 *    read FLC codeword from UVLC-partition
 ************************************************************************
 */
int readSyntaxElement_FLC(SyntaxElement *sym, Bitstream *currStream)
{
  int64_t BitstreamLengthInBits  = (currStream->bitstream_length << 3) + 7;
  
  if ((GetBits(currStream->streamBuffer, currStream->frame_bitoffset, &(sym->inf), BitstreamLengthInBits, sym->len)) < 0)
    return -1;

  sym->value1 = sym->inf;
  currStream->frame_bitoffset += sym->len; // move bitstream pointer

  return 1;
}

/*!
 ************************************************************************
 * \brief
 *    read next UVLC codeword from UVLC-partition and
 *    map it to the corresponding syntax element
 ************************************************************************
 */
int readSyntaxElement_VLC(SyntaxElement *sym, Bitstream *currStream)
{

    sym->len = GetVLCSymbol(currStream->streamBuffer, currStream->frame_bitoffset, &(sym->inf), currStream->bitstream_length);
    if (sym->len == -1)
        return -1;

    currStream->frame_bitoffset += sym->len;
    sym->mapping(sym->len, sym->inf, &(sym->value1), &(sym->value2));

    return 1;
}

/*!
 ************************************************************************
 * \brief
 *  Reads bits from the bitstream buffer
 *
 * \param buffer
 *    containing VLC-coded data bits
 * \param totbitoffset
 *    bit offset from start of partition
 * \param info
 *    returns value of the read bits
 * \param bitcount
 *    total bytes in bitstream
 * \param numbits
 *    number of bits to read
 *
 ************************************************************************
 */
int GetBits (byte buffer[], int64_t totbitoffset,int *info, int64_t bitcount,
             int numbits)
{
  if ((totbitoffset + numbits ) > bitcount) 
  {
    return -1;
  }
  else
  {
    int bitoffset  = 7 - (totbitoffset & 0x07); // bit from start of byte
    int64_t byteoffset = (totbitoffset >> 3); // byte from start of buffer
    int bitcounter = numbits;
    byte *curbyte  = &(buffer[byteoffset]);
    int inf = 0;

    while (numbits--)
    {
      inf <<=1;    
      inf |= ((*curbyte)>> (bitoffset--)) & 0x01;    
      if (bitoffset == -1 ) 
      { //Move onto next byte to get all of numbits
        curbyte++;
        bitoffset = 7;
      }
      // Above conditional could also be avoided using the following:
      // curbyte   -= (bitoffset >> 3);
      // bitoffset &= 0x07;
    }
    *info = inf;

    return bitcounter;           // return absolute offset in bit from start of frame
  }
}
