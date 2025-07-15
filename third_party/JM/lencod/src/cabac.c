/*!
 *************************************************************************************
 * \file cabac.c
 *
 * \brief
 *    CABAC entropy coding routines
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Detlev Marpe
 **************************************************************************************
 */

#include "cabac.h"
#include "biariencode.h"
#include "Entropy.h"
#include "assert.h"

/***********************************************************************
                   Constant declarations
***********************************************************************
*/

#define COM_MIN(a,b)  (((a) < (b)) ? (a) : (b))
#define CHAR_BIT      8         // number of bits in a char

enum CABACBlockTypes {
    LUMA_16DC = 0,
    LUMA_16AC = 1,
    LUMA_8x8 = 2,
    CHROMA_DC_4x4 = 3,
    CB_8x4 = 4,
    LUMA_4x4 = 5,
    CHROMA_DC = 6,
    CHROMA_AC = 7,
    CHROMA_DC_2x4 = 8,
    LUMA_8x4 = 9,
    CB_16DC = 10,
    CB_16AC = 11,
    CB_8x8 = 12,
    LUMA_4x8 = 13,
    CR_16DC = 14,
    CB_4x4 = 15,
    CR_16AC = 16,
    CB_4x8 = 17,
    CR_8x8 = 18,
    CR_4x8 = 19,
    CR_8x4 = 20,
    CR_4x4 = 21
} CABACBlockTypes;

//===== position -> ctx for MAP =====
//--- zig-zag scan ----
static const byte  pos2ctx_map8x8[] = {
    0,  1, 2,  1, 2,  3,  4,  3,  4,  5, 6,  5,  6,  5, 6,  7,
    8,  7,  8,  7,   8,  9, 10,  9,   10,  9, 10, 9, 10, 9, 10, 9,
    10,  9,  10,  11,   12,  11, 12,  11,   12,  11, 12,  11, 12, 11, 12,11,
    12,  13,  14,  13,   14,  13, 14,  13,   14,  13, 14,  13, 14, 13, 14,14
}; // 15 CTX
static const byte  pos2ctx_map8x4[] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8, 7, 8,  7,  8,  9,  10, 9,
    10, 9, 10,  11,  12,  11, 12,  11,  12, 13, 14,  13, 14, 13, 14, 14
}; // 15 CTX


static const byte  pos2ctx_map4x4 [] = {
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 14
}; // 15 CTX

static const byte  pos2ctx_map2x4c[] = {
  0,  0,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2
}; // 15 CTX

static const byte  pos2ctx_map4x4c[] = {
  0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2
}; // 15 CTX

//--- interlace scan ----
//Taken from ABT
static const byte  pos2ctx_map8x8i[] = {
    0,  1,  1,  2,  2,  3,  3,  4,  5,  6,  7,  7,  7,  8,  4,  5,
    6,  9, 10, 10,  8, 11, 12, 11,  9,  9, 10, 10,  8, 11, 12, 11,
    9,  9, 10, 10,  8, 11, 12, 11,  9,  9, 10, 10,  8, 13, 13,  9,
    9, 10, 10,  8, 13, 13,  9,  9, 10, 10, 14, 14, 14, 14, 14, 14
}; // 15 CTX

static const byte  pos2ctx_map8x4i[] = {
    0,  1,  2,  3,  4,  5,  6,  3,  4,  5,  6,  3,  4,  7,  6,  8,
    9,  7,  6,  8,  9, 10, 11, 12, 12, 10, 11, 13, 13, 14, 14, 14
}; // 15 CTX

static const byte  pos2ctx_map4x8i[] = {
    0,  1,  1,  1,  2,  3,  3,  4,  4,  4,  5,  6,  2,  7,  7,  8,
    8,  8,  5,  6,  9, 10, 10, 11, 11, 11, 12, 13, 13, 14, 14, 14
}; // 15 CTX


const unsigned char SCAN_POS_TU4x4_HOR_PRE[16][2] =
{
  {0,0},{0,1},{0,2},{0,3},
  {1,0},{1,1},{1,2},{2,0},
  {1,3},{2,1},{2,2},{3,0},
  {2,3},{3,1},{3,2},{3,3},
};

const unsigned char SCAN_POS_TU4x4_VER_PRE[16][2] =
{
    {0,0},{1,0},{2,0},{3,0},
    {0,1},{1,1},{2,1},{0,2},
    {3,1},{1,2},{2,2},{0,3},
    {3,2},{1,3},{2,3},{3,3}
};

const unsigned char SCAN_POS_TU4x4_OTHER_MODE[16][2] =
{
  {0,0},{1,0},{0,1},{1,1},
  {0,2},{2,0},{3,0},{2,1},
  {1,2},{0,3},{1,3},{2,2},
  {3,1},{3,2},{2,3},{3,3}
};

const unsigned char SCAN_POS_TU8x8_HOR_PRE[64][2] = {
    {0,0},{0,1},{0,2},{0,3},{1,0},{1,1},{1,2},{2,0},
    {1,3},{2,1},{2,2},{3,0},{2,3},{3,1},{3,2},{3,3},
    {0,4},{0,5},{0,6},{0,7},{1,4},{1,5},{1,6},{2,4},
    {1,7},{2,5},{2,6},{3,4},{2,7},{3,5},{3,6},{3,7},
    {4,0},{4,1},{4,2},{4,3},{5,0},{5,1},{5,2},{6,0},
    {5,3},{6,1},{6,2},{7,0},{6,3},{7,1},{7,2},{7,3},
    {4,4},{4,5},{4,6},{4,7},{5,4},{5,5},{5,6},{6,4},
    {5,7},{6,5},{6,6},{7,4},{6,7},{7,5},{7,6},{7,7},
};
const unsigned char SCAN_POS_TU8x8_VER_PRE[64][2] = {
    {0,0},{1,0},{2,0},{3,0},{0,1},{1,1},{2,1},{0,2},
    {3,1},{1,2},{2,2},{0,3},{3,2},{1,3},{2,3},{3,3},
    {0,4},{1,4},{2,4},{3,4},{0,5},{1,5},{2,5},{0,6},
    {3,5},{1,6},{2,6},{0,7},{3,6},{1,7},{2,7},{3,7},
    {4,0},{5,0},{6,0},{7,0},{4,1},{5,1},{6,1},{4,2},
    {7,1},{5,2},{6,2},{4,3},{7,2},{5,3},{6,3},{7,3},
    {4,4},{5,4},{6,4},{7,4},{4,5},{5,5},{6,5},{4,6},
    {7,5},{5,6},{6,6},{4,7},{7,6},{5,7},{6,7},{7,7},
};
const unsigned char SCAN_POS_TU8x8_OTHER_MODE[64][2] = {
    {0,0},{1,0},{0,1},{1,1},{0,2},{2,0},{3,0},{2,1},
    {1,2},{0,3},{1,3},{2,2},{3,1},{3,2},{2,3},{3,3},
    {0,4},{1,4},{2,4},{3,4},{0,5},{1,5},{2,5},{0,6},
    {3,5},{1,6},{2,6},{0,7},{3,6},{1,7},{2,7},{3,7},
    {4,0},{4,1},{4,2},{4,3},{5,0},{5,1},{5,2},{6,0},
    {5,3},{6,1},{6,2},{7,0},{6,3},{7,1},{7,2},{7,3},
    {4,4},{5,4},{4,5},{5,5},{4,6},{6,4},{7,4},{6,5},
    {5,6},{4,7},{5,7},{6,6},{7,5},{7,6},{6,7},{7,7}
};

const unsigned char SCAN_POS_TU4x8_HOR_PRE[32][2] = {
{0,0},{0,1},{0,2},{0,3},
{1,0},{1,1},{1,2},{2,0},
{1,3},{2,1},{2,2},{3,0},
{2,3},{3,1},{3,2},{3,3},
{0,4},{0,5},{0,6},{0,7},
{1,4},{1,5},{1,6},{2,4},
{1,7},{2,5},{2,6},{3,4},
{2,7},{3,5},{3,6},{3,7},
};

const unsigned char SCAN_POS_TU4x8_VER_PRE[32][2] = {
{0,0},{1,0},{2,0},{3,0},
{0,1},{1,1},{2,1},{0,2},
{3,1},{1,2},{2,2},{0,3},
{3,2},{1,3},{2,3},{3,3},
{0,4},{1,4},{2,4},{3,4},
{0,5},{1,5},{2,5},{0,6},
{3,5},{1,6},{2,6},{0,7},
{3,6},{1,7},{2,7},{3,7}

};

const unsigned char SCAN_POS_TU4x8_OTHER_MODE[32][2] = {
{0,0},{1,0},{0,1},{1,1},
{0,2},{2,0},{3,0},{2,1},
{1,2},{0,3},{1,3},{2,2},
{3,1},{3,2},{2,3},{3,3},
{0,4},{1,4},{0,5},{1,5},
{0,6},{2,4},{3,4},{2,5},
{1,6},{0,7},{1,7},{2,6},
{3,5},{3,6},{2,7},{3,7}

};

const byte* pos2ctx_map    [] = {
  pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8, pos2ctx_map8x4,
  pos2ctx_map8x4, pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map4x4,
  pos2ctx_map2x4c, pos2ctx_map8x4,
  pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8,pos2ctx_map8x4,
  pos2ctx_map8x4, pos2ctx_map4x4,  //Cb component
  pos2ctx_map4x4, pos2ctx_map8x4, pos2ctx_map8x8,pos2ctx_map8x4,
  pos2ctx_map8x4,pos2ctx_map4x4  //Cr component
};


const byte* pos2ctx_map_int[] = {
  pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8i,pos2ctx_map8x4i,
  pos2ctx_map4x8i,pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map4x4,
  pos2ctx_map2x4c, pos2ctx_map4x4c,
  //444_TEMP_NOTE: the followings are addded for the 4:4:4 common mode};
  pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8i,pos2ctx_map8x4i,
  pos2ctx_map8x4i,pos2ctx_map4x4, //Cb component
  pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8i,pos2ctx_map8x4i,
  pos2ctx_map8x4i,pos2ctx_map4x4  //Cr component}
};  


//===== position -> ctx for LAST =====
static const byte  pos2ctx_last8x8 [] = {
  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
  3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,
  5,  5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8
}; //  9 CTX

static const byte  pos2ctx_last8x4 [] = {
  0,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,
  3,  3,  3,  3,  4,  4,  4,  4,  5,  5,  6,  6,  7,  7,  8,  8
}; //  9 CTX

static const byte  pos2ctx_last4x4 [] = {
  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
}; // 15 CTX

static const byte  pos2ctx_last2x4c[] = {
  0,  0,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2
}; // 15 CTX

static const byte  pos2ctx_last4x4c[] = { 
  0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2
}; // 15 CTX

const byte* pos2ctx_last    [] = {
  pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last8x8, pos2ctx_last8x4,
  pos2ctx_last8x4, pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last4x4,
  pos2ctx_last2x4c, pos2ctx_last8x4,
  pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last8x8,pos2ctx_last8x4,
  pos2ctx_last8x4, pos2ctx_last4x4,  //Cb component
  pos2ctx_last4x4, pos2ctx_last8x4, pos2ctx_last8x8,pos2ctx_last8x4,
  pos2ctx_last8x4, pos2ctx_last4x4 //Cr component
};

/***********************************************************************
 * L O C A L L Y   D E F I N E D   F U N C T I O N   P R O T O T Y P E S
 ***********************************************************************
 */

static inline int iabs(int x)
{
    static const int INT_BITS = (sizeof(int) * CHAR_BIT) - 1;
    int y = x >> INT_BITS;
    return (x ^ y) - y;
}

/*!
 ************************************************************************
 * \brief
 *    Exp Golomb binarization and encoding
 ************************************************************************
 */
static int exp_golomb_encode_eq_prob(Bitstream *bitstream, EncodingEnvironmentPtr eep_dp,
    unsigned int symbol,
    int k)
{
    int num_vlc_bit = 0;
    for (;;)
    {
        if (symbol >= (unsigned int)(1 << k))
        {
            if (eep_dp->cabac_encoding == 1)
                write_u_v(1, 1, bitstream);
            else
                num_vlc_bit++;

            symbol = symbol - (1 << k);
            k++;
        } else
        {
            if (eep_dp->cabac_encoding == 1)
                write_u_v(1, 0, bitstream);
            else
                num_vlc_bit++;
            while (k--)                               //next binary part
            {
                if (eep_dp->cabac_encoding == 1)
                    write_u_v(1, ((symbol >> k) & 1), bitstream);
                else
                    num_vlc_bit++;
            }

            break;

        }
    }
    return num_vlc_bit;
}


/*!
 ************************************************************************
 * \brief
 *    Exp-Golomb for MV Encoding
*
************************************************************************/

static int unary_exp_golomb_mv_encode(Bitstream *bitstream, EncodingEnvironmentPtr eep_dp,
    unsigned int symbol,
    BiContextTypePtr ctx,
    unsigned int max_bin)
{
    int num_vlc_bits = 0;
    if (symbol == 0)
    {
        biari_encode_symbol(eep_dp, 0, ctx);
        return 0;
    } else
    {
        unsigned int bin = 1;
        unsigned int l = symbol, k = 1;

        biari_encode_symbol(eep_dp, 1, ctx);
        num_vlc_bits += exp_golomb_encode_eq_prob(bitstream, eep_dp, symbol - 1, 3);
    }
    return num_vlc_bits;
}


int writeMVD_CABAC(Bitstream *bitstream, EncodingEnvironmentPtr eep_dp, MotionInfoContexts* mot_ctx, int MVD, int MVP, int is_MVDy, int cond)
{
    int      no_bits = 0;
    int curr_len = arienco_bits_written(eep_dp);
    int num_vlc_bit = 0;
    int a;
    int act_ctx;
    int act_sym;
    int mv_pred_res;
    int mv_sign;

    int k = is_MVDy; // MVD component

    a = iabs(MVP);

    //if ((mv_local_err = a) < 3)
    if (1)
        act_ctx = 5 * k;
    else
    {
        act_ctx = 5 * k + 2;
    }

    mv_pred_res = MVD;
    act_sym = iabs(mv_pred_res);
    //
    if (!(is_MVDy && !cond))
    {
        biari_encode_symbol(eep_dp, act_sym ? 1 : 0, &mot_ctx->mv_res_contexts[0][act_ctx]);
    }

    if (act_sym)
    {
        act_sym--;
        act_ctx = 5 * k;
        num_vlc_bit += unary_exp_golomb_mv_encode(bitstream, eep_dp, act_sym, mot_ctx->mv_res_contexts[1] + act_ctx, 3);
        mv_sign = (mv_pred_res < 0) ? 1 : 0;
        if (eep_dp->cabac_encoding == 1)
            write_u_v(1, mv_sign, bitstream);
        else
            num_vlc_bit++;
    }

    no_bits = arienco_bits_written(eep_dp) - curr_len;
    return no_bits + num_vlc_bit;
}

int write_MB_mode_CABAC(EncodingEnvironmentPtr eep_dp, MotionInfoContexts* mot_ctx, int mb_mode)
{
    int      no_bits = 0;
    int curr_len = arienco_bits_written(eep_dp);

    biari_encode_symbol(eep_dp, mb_mode, &mot_ctx->mb_pred_context[0]);

    no_bits = arienco_bits_written(eep_dp) - curr_len;
    return no_bits;
}

int write_inter_mode_CABAC(EncodingEnvironmentPtr eep_dp, MotionInfoContexts* mot_ctx, int inter_no_residual_flag)
{
    int      no_bits = 0;
    int curr_len = arienco_bits_written(eep_dp);

    biari_encode_symbol(eep_dp, inter_no_residual_flag, &mot_ctx->mb_pred_context[4]);

    no_bits = arienco_bits_written(eep_dp) - curr_len;
    return no_bits;
}

int write_inter_mvd_CABAC(EncodingEnvironmentPtr eep_dp, MotionInfoContexts* mot_ctx, int mvd_flag)
{
    int      no_bits = 0;
    int curr_len = arienco_bits_written(eep_dp);

    biari_encode_symbol(eep_dp, mvd_flag, &mot_ctx->mb_pred_context[2]);

    no_bits = arienco_bits_written(eep_dp) - curr_len;
    return no_bits;
}


void write_significance_map(EncodingEnvironmentPtr eep_dp, int type, int coeff[], TextureInfoContexts*  tex_ctx)
{
    int   k1 = maxpos[type];
    int   fld = 0;
    BiContextTypePtr  last_ctx = tex_ctx->last_contexts[fld][type2ctx_last[type]];  //num of ctx: (7+3*6 +3*5)*2= 80
    const byte *pos2ctxlast = pos2ctx_last[type];

    int last_pos = 0;
    for (int idx = 0; idx < k1 + 1; ++idx)
    {
        if (coeff[idx] != 0)
            last_pos = idx;
    }
    if (last_pos == k1)
    {
        biari_encode_symbol(eep_dp, 0, last_ctx + 0);
    } else
    {
        biari_encode_symbol(eep_dp, 1, last_ctx + 0);
        int bit_length = k1 == 15 ? 4 : k1 == 31 ? 5 : 6;
        for (int i = 0; i < bit_length; i++)
        {
            if ((bit_length - 1 != i) || (last_pos != k1 - 1)) {
                int bit_value = (last_pos >> (bit_length - 1 - i)) & 1;
                biari_encode_symbol(eep_dp, bit_value, last_ctx + i + 1);
            }
        }
    }
}

int write_significant_coefficients(Bitstream *bitstream, EncodingEnvironmentPtr eep_dp, int type, int coeff[], TextureInfoContexts* tex_ctx)
{
    BiContextType *one_contexts = tex_ctx->one_contexts[type2ctx_one[type]];
    int   absLevel;
    int   ctx;
    short sign;
    int greater_one;
    int   c1 = 1;
    int   i;

    int num_vlc_bit = 0;
    int  rice_param = 0;

    //sig and gt1
    int last_pos = 0;
    int k1 = maxpos[type];
    BiContextTypePtr  map_ctx = tex_ctx->map_contexts[0][type2ctx_map[type]];
    const byte *pos2ctxmap = pos2ctx_map[type];
    for (int idx = 0; idx < k1 + 1; ++idx)
    {
        if (coeff[idx] != 0)
            last_pos = idx;
    }
    const int skip_table[18] = {14, 9, 6, 4, 3, 2,  20, 14, 9, 6, 4, 2,  45, 30, 20, 15, 10, 5};
    int skip_start_pos;
    int noskip_v = k1 - 5;
    if (last_pos < noskip_v)
        skip_start_pos = 1;
    else {
        int table_offset = k1 == 15 ? 0 : k1 == 31 ? 6 : 12;
        skip_start_pos = skip_table[table_offset + k1 - last_pos];
    }
    if (last_pos != 0) {
        if (coeff[last_pos] > 0)   //last pos
        {
            absLevel = coeff[last_pos];
        } else
        {
            absLevel = -coeff[last_pos];
        }
        greater_one = (absLevel > 1);
        biari_encode_symbol(eep_dp, greater_one, one_contexts + c1);

        if (greater_one)
        {
            c1 = 0;
        } else if (c1)
        {
            c1++;
        }
        for (int k = last_pos - 1; k >= skip_start_pos; --k)
        {
            int sig = (coeff[k] != 0);
            biari_encode_symbol(eep_dp, sig, map_ctx + pos2ctxmap[k]);

            if (sig == 1) {
                if (coeff[k] > 0)   //last pos
                {
                    absLevel = coeff[k];
                } else
                {
                    absLevel = -coeff[k];
                }
                greater_one = (absLevel > 1);
                ctx = imin(c1, 7);
                biari_encode_symbol(eep_dp, greater_one, one_contexts + ctx);

                if (greater_one)
                {
                    c1 = 0;
                } else if (c1)
                {
                    c1++;
                }
            }
        }

    }

    //vlc below

    int lastCg = (maxpos[type] >> LL_SUBBLOCK_LOG2);

    for (int subSet = lastCg; subSet >= 0; subSet--) {

        int numNonZero = 0;
        unsigned int coeffSigns = 0;

        for (i = COM_MIN(maxpos[type], ((subSet + 1) << LL_SUBBLOCK_LOG2) - 1); i >= (subSet << LL_SUBBLOCK_LOG2); i--)
        {
            if (i < skip_start_pos) { //last_pos == k1 && k1 == 15
                if (coeff[i] > 0)
                {
                    absLevel = coeff[i];
                    sign = 0;
                    coeffSigns = coeffSigns * 2 + (coeff[i] < 0);
                    numNonZero++;
                } else if (coeff[i] < 0)
                {
                    absLevel = -coeff[i];
                    sign = 1;
                    coeffSigns = coeffSigns * 2 + (coeff[i] < 0);
                    numNonZero++;
                } else
                    absLevel = 0;
                if (i == 0 && type < 14 && type != 12)   //luma
                    rice_param = rice_param + 1;
                if (i == last_pos)
                    absLevel--;

                if (absLevel < (REM_PREFIX_LENGTH << rice_param))
                {
                    int code_lenth = (absLevel) >> rice_param;
                    while (code_lenth--)
                    {
                        if (eep_dp->cabac_encoding == 1)
                            write_u_v(1, 1, bitstream);
                        else
                            num_vlc_bit++;
                    }
                    if (eep_dp->cabac_encoding == 1)
                        write_u_v(1, 0, bitstream);
                    else
                        num_vlc_bit++;
                    for (int index = rice_param; index > 0; index--)
                    {
                        int code_bin = ((absLevel) % (1 << rice_param)) & (1 << (index - 1));
                        if (eep_dp->cabac_encoding == 1)
                            write_u_v(1, code_bin != 0, bitstream);
                        else
                            num_vlc_bit++;
                    }
                } else
                {
                    int code_lenth = REM_PREFIX_LENGTH;
                    while (code_lenth--)
                    {
                        if (eep_dp->cabac_encoding == 1)
                            write_u_v(1, 1, bitstream);
                        else
                            num_vlc_bit++;
                    }
                    int code_val = absLevel - (REM_PREFIX_LENGTH << rice_param);
                    num_vlc_bit += exp_golomb_encode_eq_prob(bitstream, eep_dp, code_val, rice_param); //Fix bits count
                }

                if (absLevel > (REM_PREFIX_LENGTH << rice_param)) //last pos 
                {
                    rice_param = rice_param + 1;
                }

                if (rice_param > 4)
                    rice_param = 4;

            } else if (coeff[i] != 0)
            {

                coeffSigns = coeffSigns * 2 + (coeff[i] < 0);
                numNonZero++;

                if (coeff[i] > 0)
                {
                    absLevel = coeff[i];
                    sign = 0;
                } else
                {
                    absLevel = -coeff[i];
                    sign = 1;
                }

                greater_one = (absLevel > 1);

                //--- if coefficient is one ---
                if (greater_one)
                {
                    if (i == 0 && type < 14 && type != 12)   //luma
                        rice_param = rice_param + 1;

                    if (absLevel - 2 < (REM_PREFIX_LENGTH << rice_param))
                    {
                        int code_lenth = (absLevel - 2) >> rice_param;
                        while (code_lenth--)
                        {
                            if (eep_dp->cabac_encoding == 1)
                                write_u_v(1, 1, bitstream);
                            else
                                num_vlc_bit++;
                        }
                        if (eep_dp->cabac_encoding == 1)
                            write_u_v(1, 0, bitstream);
                        else
                            num_vlc_bit++;
                        for (int index = rice_param; index > 0; index--)
                        {
                            int code_bin = ((absLevel - 2) % (1 << rice_param)) & (1 << (index - 1));
                            if (eep_dp->cabac_encoding == 1)
                                write_u_v(1, code_bin != 0, bitstream);
                            else
                                num_vlc_bit++;
                        }
                    } else
                    {
                        int code_lenth = REM_PREFIX_LENGTH;
                        while (code_lenth--)
                        {
                            if (eep_dp->cabac_encoding == 1)
                                write_u_v(1, 1, bitstream);
                            else
                                num_vlc_bit++;
                        }
                        int code_val = absLevel - 2 - (REM_PREFIX_LENGTH << rice_param);
                        num_vlc_bit += exp_golomb_encode_eq_prob(bitstream, eep_dp, code_val, rice_param);
                    }

                    if (absLevel > (REM_PREFIX_LENGTH << rice_param))
                    {
                        rice_param = rice_param + 1;
                    }
                    if (rice_param > 4)
                        rice_param = 4;
                }
            }
        }

        if (numNonZero > 0) {
            for (i = numNonZero - 1; i >= 0; i--) {
                int signbit = ((coeffSigns) & (1 << i));
                if (eep_dp->cabac_encoding == 1)
                    write_u_v(1, signbit != 0, bitstream);
                else
                    num_vlc_bit++;
            }
        }
    }
    return num_vlc_bit;
}

int enc_writePredMode_CABAC(EncodingEnvironmentPtr eep_dp, TextureInfoContexts* tex_ctx, int pred_mode, int component, uint32_t cclm_enable)
{
    int      no_bits = 0;
    int curr_len = arienco_bits_written(eep_dp);
    if (component == 0) {  //luma
        biari_encode_symbol(eep_dp, pred_mode >> 1, tex_ctx->ipr_contexts);
        if (pred_mode >> 1 == 0) {
            biari_encode_symbol(eep_dp, pred_mode % 2, tex_ctx->ipr_contexts + 1);
        }
    } else {
        biari_encode_symbol(eep_dp, pred_mode >> 1, tex_ctx->ipr_contexts + 3);
        if (pred_mode >> 1 == 0 || cclm_enable) {
            biari_encode_symbol(eep_dp, pred_mode % 2, tex_ctx->ipr_contexts + 4 + (pred_mode >> 1));
        }
    }
    no_bits = arienco_bits_written(eep_dp) - curr_len;
    return no_bits;
}

int enc_writePuSize_CABAC(EncodingEnvironmentPtr eep_dp, TextureInfoContexts* tex_ctx, int pu_size)
{
    int      no_bits = 0;
    int curr_len = arienco_bits_written(eep_dp);
    biari_encode_symbol(eep_dp, pu_size, tex_ctx->trans_contexts);
    no_bits = arienco_bits_written(eep_dp) - curr_len;
    return no_bits;
}


int enc_writeCoeff4x4_CABAC(Bitstream *bitstream, EncodingEnvironmentPtr eep_dp, TextureInfoContexts* tex_ctx, pel* coeff, int plane, int pred_mode) //first plane = 0
{
    int      no_bits = 0;
    int curr_len = arienco_bits_written(eep_dp);
    int k, level = 1, run = 0;
    int i, j;
    int max_coeff_num = 0;
    int scan_pos;
    int pLevel[17];
    int pRun[17];
    int pCoeff[17];

    max_coeff_num = 16;

    run = -1;
    scan_pos = 0;

    int num_vlc_bits = 0;

    for (k = 16 - max_coeff_num; k < 16; ++k)
    {
        if (pred_mode == INTRA_HOR)
        {
            i = SCAN_POS_TU4x4_HOR_PRE[k][0];
            j = SCAN_POS_TU4x4_HOR_PRE[k][1];
        } else if (pred_mode == INTRA_VER)
        {
            i = SCAN_POS_TU4x4_VER_PRE[k][0];
            j = SCAN_POS_TU4x4_VER_PRE[k][1];
        } else
        {
            i = SCAN_POS_TU4x4_OTHER_MODE[k][0];
            j = SCAN_POS_TU4x4_OTHER_MODE[k][1];
        }
        level = coeff[4 * j + i];
        pCoeff[k] = level;

        ++run;
        if (level != 0)
        {
            pLevel[scan_pos] = level;
            pRun[scan_pos++] = run;
            run = -1;
        }
    }
    pLevel[scan_pos] = 0;
    pCoeff[16] = 0;

    int type = ((plane == 0) ? (LUMA_4x4) : ((plane == 1) ? CB_4x4 : CR_4x4));
    biari_encode_symbol(eep_dp, scan_pos != 0, tex_ctx->bcbp_contexts[type2ctx_bcbp[type]]);  //write cbf

    if (scan_pos != 0)
    {
        //===== encode significance map =====
        write_significance_map(eep_dp, type, pCoeff, tex_ctx);
        //===== encode significant coefficients =====
        num_vlc_bits += write_significant_coefficients(bitstream, eep_dp, type, pCoeff, tex_ctx);
    }

    no_bits = arienco_bits_written(eep_dp) - curr_len;
    return no_bits + num_vlc_bits;
}

int enc_writeCoeff8x8_CABAC(Bitstream *bitstream, EncodingEnvironmentPtr eep_dp, TextureInfoContexts* tex_ctx, pel* coeff, int plane, int pred_mode)
{
    int      no_bits = 0;
    int curr_len = arienco_bits_written(eep_dp);
    int k, level = 1, run = 0;
    int i, j;
    int max_coeff_num = 0;
    int scan_pos;
    int pLevel[65];
    int pRun[65];
    int pCoeff[65];

    max_coeff_num = 64;

    run = -1;
    scan_pos = 0;

    for (k = 64 - max_coeff_num; k < 64; ++k)
    {
        if (pred_mode == INTRA_HOR)
        {
            i = SCAN_POS_TU8x8_HOR_PRE[k][0];
            j = SCAN_POS_TU8x8_HOR_PRE[k][1];
        } else if (pred_mode == INTRA_VER)
        {
            i = SCAN_POS_TU8x8_VER_PRE[k][0];
            j = SCAN_POS_TU8x8_VER_PRE[k][1];
        } else
        {
            i = SCAN_POS_TU8x8_OTHER_MODE[k][0];
            j = SCAN_POS_TU8x8_OTHER_MODE[k][1];
        }
        level = coeff[8 * j + i];
        pCoeff[k] = level;

        ++run;
        if (level != 0)
        {
            pLevel[scan_pos] = level;
            pRun[scan_pos++] = run;
            run = -1;
        }
    }
    pLevel[scan_pos] = 0;
    pCoeff[64] = 0;
    enum CABACBlockTypes bType[3] = {LUMA_8x8,CB_8x8,CR_8x8};
    int type = bType[plane];

    int num_vlc_bits = 0;
    biari_encode_symbol(eep_dp, scan_pos != 0, tex_ctx->bcbp_contexts[type2ctx_bcbp[type]]);  //write cbf, range 2-3


    if (scan_pos != 0)
    {
        //===== encode significance map =====
        write_significance_map(eep_dp, type, pCoeff, tex_ctx);

        //===== encode significant coefficients =====
        num_vlc_bits += write_significant_coefficients(bitstream, eep_dp, type, pCoeff, tex_ctx);
    }

    no_bits = arienco_bits_written(eep_dp) - curr_len;
    return no_bits + num_vlc_bits;
}


int enc_writeCoeff4x8_CABAC(Bitstream *bitstream, EncodingEnvironmentPtr eep_dp, TextureInfoContexts* tex_ctx, pel* coeff, int plane, int pred_mode)
{
    int      no_bits = 0;
    int curr_len = arienco_bits_written(eep_dp);
    int k, level = 1, run = 0;
    int i, j;
    int max_coeff_num = 32;
    int scan_pos;
    int pLevel[33];
    int pRun[33];
    int pCoeff[33];

    run = -1;
    scan_pos = 0;

    for (k = 32 - max_coeff_num; k < 32; ++k)
    {
        if (pred_mode == INTRA_HOR)

        {
            i = SCAN_POS_TU4x8_HOR_PRE[k][0];
            j = SCAN_POS_TU4x8_HOR_PRE[k][1];
        } else if (pred_mode == INTRA_VER)

        {
            i = SCAN_POS_TU4x8_VER_PRE[k][0];
            j = SCAN_POS_TU4x8_VER_PRE[k][1];
        } else
        {
            i = SCAN_POS_TU4x8_OTHER_MODE[k][0];
            j = SCAN_POS_TU4x8_OTHER_MODE[k][1];
        }
        level = coeff[4 * j + i];
        pCoeff[k] = level;


        ++run;
        if (level != 0)
        {
            pLevel[scan_pos] = level;
            pRun[scan_pos++] = run;
            run = -1;
        }
    }
    pLevel[scan_pos] = 0;
    pCoeff[32] = 0;

    int num_vlc_bits = 0;

    int type = ((plane == 0) ? (LUMA_4x8) : ((plane == 1) ? CB_4x8 : CR_4x8));
    biari_encode_symbol(eep_dp, scan_pos != 0, tex_ctx->bcbp_contexts[type2ctx_bcbp[type]]);  //write cbf, range 2-3

    if (scan_pos != 0)
    {
        //===== encode significance map =====
        write_significance_map(eep_dp, type, pCoeff, tex_ctx);

        //===== encode significant coefficients =====
        num_vlc_bits += write_significant_coefficients(bitstream, eep_dp, type, pCoeff, tex_ctx);
    }
    no_bits = arienco_bits_written(eep_dp) - curr_len;
    return no_bits + num_vlc_bits;
}
