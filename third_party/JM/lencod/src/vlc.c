/*!
 ***************************************************************************
 * \file vlc.c
 *
 * \brief
 *    (CA)VLC coding functions
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Inge Lille-Langoy               <inge.lille-langoy@telenor.com>
 *    - Detlev Marpe
 *    - Stephan Wenger                  <stewe@cs.tu-berlin.de>
 ***************************************************************************
 */


#include <math.h>
#include "vlc.h"
#include "Entropy.h"
#include <stdlib.h>
#include <stdio.h>

#define CHAR_BIT 8

 /*!
  ************************************************************************
  * \brief
  *    mapping for se(v) syntax elements
  * \param se
  *    value to be mapped
  * \param dummy
  *    dummy parameter
  * \param len
  *    returns mapped value length
  * \param info
  *    returns mapped value
  ************************************************************************
  */
static inline int iabs(int x)
{
    static const int INT_BITS = (sizeof(int) * CHAR_BIT) - 1;
    int y = x >> INT_BITS;
    return (x ^ y) - y;
}

void se_linfo(int se, int dummy, int *len, int *info)
{
    int sign = (se <= 0) ? 1 : 0;
    int n = iabs(se) << 1;   //  n+1 is the number in the code table.  Based on this we find length and info
    int nn = (n >> 1);
    int i;
    for (i = 0; i < 33 && nn != 0; i++)
    {
        nn >>= 1;
    }
    *len = (i << 1) + 1;
    *info = n - (1 << i) + sign;
}

/*!
 ************************************************************************
 * \brief
 *    Makes code word and passes it back
 *    A code word has the following format: 0 0 0 ... 1 Xn ...X2 X1 X0.
 *
 * \par Input:
 *    Info   : Xn..X2 X1 X0                                             \n
 *    Length : Total number of bits in the codeword
 ************************************************************************
 */
 // NOTE this function is called with sym->inf > (1<<(sym->len >> 1)).  The upper bits of inf are junk
int symbol2uvlc(SyntaxElement *sym)
{
    int suffix_len = sym->len >> 1;
    suffix_len = (1 << suffix_len);
    sym->bitpattern = suffix_len | (sym->inf & (suffix_len - 1));
    return 0;
}

/*!
 *************************************************************************************
 * \brief
 *    write_se_v, writes an se(v) syntax element, returns the length in bits
 *
 * \param tracestring
 *    the string for the trace file
 * \param value
 *    the value to be coded
 *  \param bitstream
 *    the target bitstream the value should be coded into
 *
 * \return
 *    Number of bits used by the coded syntax element
 *
 * \ note
 *    This function writes always the bit buffer for the progressive scan flag, and
 *    should not be used (or should be modified appropriately) for the interlace crap
 *    When used in the context of the Parameter Sets, this is obviously not a
 *    problem.
 *
 *************************************************************************************
 */

int write_se_v (int value, Bitstream *bitstream)
{
  SyntaxElement symbol, *sym=&symbol;
  sym->value1 = value;
  sym->value2 = 0;

  //assert (bitstream->streamBuffer != NULL);

  se_linfo(sym->value1,sym->value2,&(sym->len),&(sym->inf));
  symbol2uvlc(sym);

  writeUVLC2buffer (sym, bitstream);

  return (sym->len);
}


/*!
 *************************************************************************************
 * \brief
 *    write_u_1, writes a flag (u(1) syntax element, returns the length in bits,
 *    always 1
 *
 * \param tracestring
 *    the string for the trace file
 * \param value
 *    the value to be coded
 *  \param bitstream
 *    the target bitstream the value should be coded into
 *
 * \return
 *    Number of bits used by the coded syntax element (always 1)
 *
 * \ note
 *    This function writes always the bit buffer for the progressive scan flag, and
 *    should not be used (or should be modified appropriately) for the interlace crap
 *    When used in the context of the Parameter Sets, this is obviously not a
 *    problem.
 *
 *************************************************************************************
 */

int write_u_v (int n, int value, Bitstream *bitstream)
{
  SyntaxElement symbol, *sym=&symbol;

  sym->bitpattern = value;
  sym->value1 = value;
  sym->len = n;  

  //assert (bitstream->streamBuffer != NULL);

  writeUVLC2buffer(sym, bitstream);

  return (sym->len);
}


/*!
 ************************************************************************
 * \brief
 *    writes UVLC code to the appropriate buffer
 ************************************************************************
 */
void  writeUVLC2buffer(SyntaxElement *se, Bitstream *currStream)
{
  unsigned int mask = 1 << (se->len - 1);
  byte *byte_buf  = &currStream->byte_buf;
  int *bits_to_go = &currStream->bits_to_go;
  int i;

  // Add the new bits to the bitstream.
  // Write out a byte if it is full
  if ( se->len < 33 )
  {
    for (i = 0; i < se->len; i++)
    {
      *byte_buf <<= 1;

      if (se->bitpattern & mask)
        *byte_buf |= 1;

      mask >>= 1;

      if ((--(*bits_to_go)) == 0)
      {
        *bits_to_go = 8;      
        currStream->streamBuffer[currStream->byte_pos++] = *byte_buf;
        *byte_buf = 0;      
      }
    }
  }
  else
  {
    printf("once vlc write bits shall less than 33\n");
    exit(-1);
    // zeros
    for (i = 0; i < (se->len - 32); i++)
    {
      *byte_buf <<= 1;

      if ((--(*bits_to_go)) == 0)
      {
        *bits_to_go = 8;      
        currStream->streamBuffer[currStream->byte_pos++] = *byte_buf;
        *byte_buf = 0;      
      }
    }
    // actual info
    mask = (unsigned int) 1 << 31;
    for (i = 0; i < 32; i++)
    {
      *byte_buf <<= 1;

      if (se->bitpattern & mask)
        *byte_buf |= 1;

      mask >>= 1;

      if ((--(*bits_to_go)) == 0)
      {
        *bits_to_go = 8;      
        currStream->streamBuffer[currStream->byte_pos++] = *byte_buf;
        *byte_buf = 0;      
      }
    }
  }
}



void BitstreamInit(Bitstream *bitstream, int logMemSize)
{
    int max_bs_size = 1 << logMemSize;
    if ((bitstream->streamBuffer = (byte*)malloc(max_bs_size)) == NULL) {
        printf("mem allocation error for bitstream\n");
        exit(-1);
    }
    bitstream->bits_to_go = 8;
    bitstream->byte_pos = 0;
    bitstream->byte_buf = 0;
    bitstream->buffer_size = max_bs_size;
}

void BitstreamRelease(Bitstream *bitstream)
{
    free(bitstream->streamBuffer);
}

void BitstreamWrite(int i, Bitstream *bitstream, char* bitstreamFile)
{
    FILE *f;
    if (i == 0) {
        if (NULL == (f = fopen(bitstreamFile, "wb"))) {
            printf("fail to open bitstream file\n");
            exit(-1);
        }
    }
    else {
        if (NULL == (f = fopen(bitstreamFile, "ab"))) {
            printf("fail to open bitstream file\n");
            exit(-1);
        }
    }
    fwrite(bitstream->streamBuffer, 1, bitstream->byte_pos, f);
    fclose(f);
    free(bitstream->streamBuffer);
}

void rendering_information(Bitstream *bitstream)
{
    int cicp_info_present_flag = 0;
    write_u_v(1, cicp_info_present_flag, bitstream);
    int mdcv_info_present_flag = 0;
    write_u_v(1, mdcv_info_present_flag, bitstream);
    int clli_info_present_flag = 0;
    write_u_v(1, clli_info_present_flag, bitstream);
    int dm_present_flag = 0;
    write_u_v(1, dm_present_flag, bitstream);
    write_u_v(4, 0, bitstream); //reserved bits

    if (cicp_info_present_flag) {
        int colour_primaries = 0;
        write_u_v(8, colour_primaries, bitstream);
        int transfer_characteristics = 0;
        write_u_v(8, transfer_characteristics, bitstream);
        int matrix_coefficients = 0;
        write_u_v(8, matrix_coefficients, bitstream);
        int image_full_range_flag = 0;
        write_u_v(1, image_full_range_flag, bitstream);
        int chroma420_sample_loc_type = 0;
        write_u_v(3, chroma420_sample_loc_type, bitstream);
        write_u_v(4, 0, bitstream); //reserved bits
    }
    if (mdcv_info_present_flag) {
        int mastering_display_colour_primaries_x[3] = { 0 };
        int mastering_display_colour_primaries_y[3] = { 0 };
        for (int i = 0; i < 3; i++) {
            write_u_v(16, mastering_display_colour_primaries_x[i], bitstream);
            write_u_v(16, mastering_display_colour_primaries_y[i], bitstream);
        }
        int mastering_display_white_point_chromaticity_x = 0;
        write_u_v(16, mastering_display_white_point_chromaticity_x, bitstream);
        int mastering_display_white_point_chromaticity_y = 0;
        write_u_v(16, mastering_display_white_point_chromaticity_y, bitstream);
        int mastering_display_maximum_luminance = 0;
        write_u_v(32, mastering_display_maximum_luminance, bitstream);
        int mastering_display_minimum_luminance = 0;
        write_u_v(32, mastering_display_minimum_luminance, bitstream);
    }
    if (clli_info_present_flag) {
        int maximum_content_light_level = 0;
        write_u_v(16, maximum_content_light_level, bitstream);
        int maximum_frame_average_light_level = 0;
        write_u_v(16, maximum_frame_average_light_level, bitstream);
    }
    if (dm_present_flag) {
        int dm_type = 0;
        write_u_v(8, dm_type, bitstream);
        int dm_size = 0;
        write_u_v(16, dm_size, bitstream);
        int dm_data_byte[65536] = { 0 };
        for (int i = 0; i < dm_size; i++) {
            write_u_v(8, dm_data_byte[i], bitstream);
        }
    }

}
//
//
void enc_pic_header(int frameType, SeqPicHeaderInfo *seqPicHeaderInfo, Bitstream *bitstream) {

    int pic_size = 0;
    write_u_v(32, pic_size, bitstream);
    write_u_v(1, frameType, bitstream);
    write_u_v(1, seqPicHeaderInfo->qpDeltaEnable, bitstream);
    write_u_v(1, seqPicHeaderInfo->hfTransformSkip, bitstream);
    write_u_v(21, 0, bitstream);   //reserved bits

    //SUB_PIC_LENGTH
    //ctx->p_subpic_info = ctx->bs.cur + 4 - (ctx->bs.leftbits >> 3);  //point to sub pic info array, each 17 bytes
    int subPicNums = (seqPicHeaderInfo->width + seqPicHeaderInfo->subPicWidth -1)/seqPicHeaderInfo->subPicWidth * \
        ((seqPicHeaderInfo->height + seqPicHeaderInfo->subPicHeight - 1) / seqPicHeaderInfo->subPicHeight);
    for (int subPic_idx = 0; subPic_idx < subPicNums; subPic_idx++) {
        write_u_v(31, 0, bitstream); //QP info
        write_u_v(9, 0, bitstream); //reserved bits
        write_u_v(24, 0, bitstream); //subpic len
        write_u_v(24, 0, bitstream); //subpic ll cabac len
        write_u_v(24, 0, bitstream); //subpic ll vlc len
        write_u_v(24, 0, bitstream); //subpic hf cabac len
    }

    //constraint on number of subpic number
    if (seqPicHeaderInfo->width > 4096) {
        if (subPicNums > 32) {
            printf("number of subpics shall be no more than 32 for image width larger than 4096\n");
            exit(-1);
        }
    }
    else if (seqPicHeaderInfo->width > 2048) {
        if (subPicNums > 16) {
            printf("number of subpics shall be no more than 16 for image width larger than 2048\n");
            exit(-1);
        }
    }
    else {
        if (subPicNums > 8) {
            printf("number of subpics shall be no more than 8 for others\n");
            exit(-1);
        }
    }
}

void enc_seq_header(SeqPicHeaderInfo *seqPicHeaderInfo, Bitstream *bitstream) 
{
    //byte *seq_header_start = bitstream->byte_pos;

    write_u_v(8, seqPicHeaderInfo->profileIdc, bitstream);
    write_u_v(8, seqPicHeaderInfo->levelIdc, bitstream);
    write_u_v(2, seqPicHeaderInfo->frameCount - 1, bitstream);
    write_u_v(8, seqPicHeaderInfo->frameRate, bitstream);

    write_u_v(16, seqPicHeaderInfo->width, bitstream);
    write_u_v(16, seqPicHeaderInfo->height, bitstream);

    write_u_v(3, (seqPicHeaderInfo->subPicWidth >> 7) - 2, bitstream);
    write_u_v(6, (seqPicHeaderInfo->subPicHeight >> 7) - 2, bitstream);
    if (seqPicHeaderInfo->subPicWidth > 1024 || seqPicHeaderInfo->subPicWidth < 256 || seqPicHeaderInfo->subPicWidth % 128 != 0) {
        printf("sub pic width shall be in range of 256 to 1024 and multiple of 128\n");
        exit(-1);
    }
    if (seqPicHeaderInfo->subPicHeight > 4352 || seqPicHeaderInfo->subPicHeight < 256 || seqPicHeaderInfo->subPicHeight % 128 != 0) {
        printf("sub pic height shall be in range of 256 to 4352 and multiple of128\n");
        exit(-1);
    }
    write_u_v(4, seqPicHeaderInfo->bitDepth -8, bitstream);
    write_u_v(2, seqPicHeaderInfo->pixelFormat, bitstream);
    //rendering info
    rendering_information(bitstream);
    write_u_v(23, 0, bitstream); //reserved bits

}


int WriteSeqPicHeader(int frameIdx, int intraPeriod, SeqPicHeaderInfo *seqPicHeaderInfo, Bitstream *bitstream)
{
    int frameType = frameIdx % intraPeriod;
    if (frameType == 0) {  //I FRAME
        enc_seq_header(seqPicHeaderInfo, bitstream);
    }
    int seqHeaderByte = bitstream->byte_pos;
    enc_pic_header(frameType, seqPicHeaderInfo, bitstream);
    return seqHeaderByte;
}

void WriteStuffingBits(Bitstream *bitstream)
{
    if(bitstream->bits_to_go!=8)
        write_u_v(bitstream->bits_to_go, 0, bitstream);
}