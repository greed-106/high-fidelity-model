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
#include "biaridecod.h"
#define CHAR_BIT      8         // number of bits in a char
#define CABAC   0
enum {
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

static const short maxpos       [] = { 15, 14, 63, 31, 31, 15,  3, 14,  7, 31, 15, 14, 63, 31, 31, 15, 15, 31, 63, 31, 31, 15 };
static const short type2ctx_bcbp[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };

static const short type2ctx_map [] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 }; // 8

static const short type2ctx_last[] = { -1,  -1,  2,  -1,  -1,  5,  -1,  -1,  -1,  9, -1, -1, -1, 9, -1, 15, -1, 17, -1, 19, -1, 21 }; // 8

static const short type2ctx_one [] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 }; // 7

//===== position -> ctx for MAP =====
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

static const byte  pos2ctx_map4x4[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 14 }; // 15 CTX
static const byte  pos2ctx_map2x4c[] = { 0,  0,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2 }; // 15 CTX
static const byte  pos2ctx_map4x4c[] = { 0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2 }; // 15 CTX
//--- interlace scan ----
//taken from ABT
static const byte  pos2ctx_map8x8i[] = { 0,  1,  1,  2,  2,  3,  3,  4,  5,  6,  7,  7,  7,  8,  4,  5,
																				 6,  9, 10, 10,  8, 11, 12, 11,  9,  9, 10, 10,  8, 11, 12, 11,
																				 9,  9, 10, 10,  8, 11, 12, 11,  9,  9, 10, 10,  8, 13, 13,  9,
																				 9, 10, 10,  8, 13, 13,  9,  9, 10, 10, 14, 14, 14, 14, 14, 14 }; // 15 CTX
static const byte  pos2ctx_map8x4i[] = { 0,  1,  2,  3,  4,  5,  6,  3,  4,  5,  6,  3,  4,  7,  6,  8,
																				 9,  7,  6,  8,  9, 10, 11, 12, 12, 10, 11, 13, 13, 14, 14, 14 }; // 15 CTX
static const byte  pos2ctx_map4x8i[] = { 0,  1,  1,  1,  2,  3,  3,  4,  4,  4,  5,  6,  2,  7,  7,  8,
																				 8,  8,  5,  6,  9, 10, 10, 11, 11, 11, 12, 13, 13, 14, 14, 14 }; // 15 CTX

static const byte* pos2ctx_map[] = { pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8, pos2ctx_map8x4,
																				pos2ctx_map8x4, pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map4x4,
																				pos2ctx_map2x4c, pos2ctx_map8x4,
																				pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8,pos2ctx_map8x4,
																				pos2ctx_map8x4, pos2ctx_map4x4,
																				pos2ctx_map4x4, pos2ctx_map8x4, pos2ctx_map8x8,pos2ctx_map8x4,
																				pos2ctx_map8x4,pos2ctx_map4x4 };

static const byte* pos2ctx_map_int[] = { pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8i,pos2ctx_map8x4i,
																				pos2ctx_map4x8i,pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map4x4,
																				pos2ctx_map2x4c, pos2ctx_map8x4,
																				pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8i,pos2ctx_map8x4i,
																				pos2ctx_map8x4i,pos2ctx_map4x4,
																				pos2ctx_map4x4, pos2ctx_map4x4, pos2ctx_map8x8i,pos2ctx_map8x4i,
																				pos2ctx_map8x4i,pos2ctx_map4x4 };

//===== position -> ctx for LAST =====
static const byte  pos2ctx_last8x8[] = { 0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
																					2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
																					3,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,
																					5,  5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8 }; //  9 CTX
static const byte  pos2ctx_last8x4[] = { 0,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,
																					3,  3,  3,  3,  4,  4,  4,  4,  5,  5,  6,  6,  7,  7,  8,  8 }; //  9 CTX

static const byte  pos2ctx_last4x4[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15 }; // 15 CTX
static const byte  pos2ctx_last2x4c[] = { 0,  0,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2 }; // 15 CTX
static const byte  pos2ctx_last4x4c[] = { 0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2 }; // 15 CTX
static const byte* pos2ctx_last[] = { pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last8x8, pos2ctx_last8x4,
																				 pos2ctx_last8x4, pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last4x4,
																				 pos2ctx_last2x4c, pos2ctx_last8x4,
																				 pos2ctx_last4x4, pos2ctx_last4x4, pos2ctx_last8x8,pos2ctx_last8x4,
																				 pos2ctx_last8x4, pos2ctx_last4x4,
																				 pos2ctx_last4x4, pos2ctx_last8x4, pos2ctx_last8x8,pos2ctx_last8x4,
																				 pos2ctx_last8x4, pos2ctx_last4x4 };



/***********************************************************************
 * L O C A L L Y   D E F I N E D   F U N C T I O N   P R O T O T Y P E S
 ***********************************************************************
 */

//static unsigned int unary_exp_golomb_level_decode( DecodingEnvironmentPtr dep_dp, BiContextTypePtr ctx);
static unsigned int unary_exp_golomb_mv_decode   (Bitstream*bs_ll, DecodingEnvironmentPtr dep_dp, BiContextTypePtr ctx, unsigned int max_bin);
static unsigned int exp_golomb_decode_eq_prob    (Bitstream*bs_ll, int k);


int dec_readPreMode_CABAC(DecodingEnvironment *dep_dp, TextureInfoContexts *tex_ctx, int component)
{
	int pred_mode;
	if (component == 0)  //luma
	{
		pred_mode = biari_decode_symbol(dep_dp, tex_ctx->ipr_contexts) << 1;
		pred_mode += biari_decode_symbol(dep_dp, tex_ctx->ipr_contexts + 1 + (pred_mode>>1));
	}
	else {
		pred_mode = biari_decode_symbol(dep_dp, tex_ctx->ipr_contexts + 3) << 1;
		pred_mode += biari_decode_symbol(dep_dp, tex_ctx->ipr_contexts + 4 + (pred_mode >> 1));
	}

	return pred_mode;
}

int dec_read_MB_Mode_CABAC(DecodingEnvironment *dep_dp, MotionInfoContexts *mot_ctx)
{
	int MB_Mode = biari_decode_symbol(dep_dp, mot_ctx->mb_pred_context);
	return MB_Mode;
}

int dec_read_inter_Mode_CABAC(DecodingEnvironment *dep_dp, MotionInfoContexts *mot_ctx)
{
	int inter_Mode = biari_decode_symbol(dep_dp, mot_ctx->mb_pred_context+4);
	return inter_Mode;
}

int dec_read_inter_split_CABAC(DecodingEnvironment *dep_dp, MotionInfoContexts *mot_ctx)
{
	int inter_Mode = biari_decode_symbol(dep_dp, mot_ctx->mb_pred_context+1);
	return inter_Mode;
}

int dec_read_inter_mvd_CABAC(DecodingEnvironment *dep_dp, MotionInfoContexts *mot_ctx)
{
	int inter_Mode = biari_decode_symbol(dep_dp, mot_ctx->mb_pred_context+2);
	return inter_Mode;
}

int dec_readTuSize_CABAC(DecodingEnvironment *dep_dp, TextureInfoContexts *tex_ctx)
{
	int TuSize = biari_decode_symbol(dep_dp, tex_ctx->trans_contexts);
	return TuSize;
}


int read_significant_coefficients(Bitstream* bs_ll, TextureInfoContexts *tex_ctx, DecodingEnvironmentPtr dep_dp,
    int                     type,
    int                    *coeff,
    int plane)
{
    const byte *pos2ctx_Map = pos2ctx_map[type];
    const byte *pos2ctx_Last = pos2ctx_last[type];

    BiContextTypePtr  map_ctx = tex_ctx->map_contexts[0][type2ctx_map[type]];
    BiContextTypePtr  last_ctx = tex_ctx->last_contexts[0][type2ctx_last[type]]; //2x(1x7+2x6+1x5)->24
    BiContextType *one_contexts = tex_ctx->one_contexts[type2ctx_one[type]];

    int   i;
    int   coeff_ctr = 0;
    int   i0 = 0;
    int   i1 = maxpos[type];
    int   c1 = 1;

    memset(coeff, 0, (i1 + 1) * sizeof(int)); //ini 0 value

    int last_pos = 0;
    int bit_length = i1 == 15 ? 4 : i1 == 31 ? 5 : 6;
#if CABAC
    int is_last_pos = biari_decode_symbol(dep_dp, last_ctx + 0);
#else
    int is_last_pos = read_u_v(1, bs_ll);
#endif
    if (is_last_pos)
    {
        for (int i = 0; i < bit_length; i++)
        {
#if CABAC
            last_pos += biari_decode_symbol(dep_dp, last_ctx + i + 1) << (bit_length - 1 - i);
#else
            last_pos += read_u_v(1, bs_ll) << (bit_length - 1 - i);
#endif
            if (last_pos == i1 - 1)
                break;
        }
    }
    else
    {
        last_pos = i1;
    }
    const int regular_stop_pos_table[18] = { 14, 9, 6, 4, 3, 2,  20, 14, 9, 6, 4, 2,  45, 30, 20, 15, 10, 5 };
    int regular_stop_pos;
    int noskip_v = i1 - 5;
    if (last_pos >= noskip_v) {
        int table_offset = i1 == 15 ? 0 : i1 == 31 ? 6 : 12;
        regular_stop_pos = regular_stop_pos_table[table_offset + i1 - last_pos];
    }
    else {
        regular_stop_pos = 1;
    }
    if (last_pos != 0) {
        //sig and gt1
        coeff[last_pos] = 1; // if last coeff, it has to be significant
        ++coeff_ctr;
#if CABAC
        coeff[last_pos] += biari_decode_symbol(dep_dp, one_contexts + c1);
#else
        coeff[last_pos] += read_u_v(1, bs_ll);
#endif
        if (coeff[last_pos] == 2)
        {
            c1 = 0;
        }
        else if (c1)
        {
            c1 = imin(++c1, 7);
        }
    }
    for (i = last_pos - 1; i >= regular_stop_pos; --i)
    {
#if CABAC
        int non_zero_flag = biari_decode_symbol(dep_dp, map_ctx + pos2ctx_Map[i]);
#else
        int non_zero_flag = read_u_v(1, bs_ll);
#endif
        if (non_zero_flag) {
            coeff[i] = 1;
            ++coeff_ctr;
#if CABAC
            coeff[i] += biari_decode_symbol(dep_dp, one_contexts + c1);
#else
            coeff[i] += read_u_v(1, bs_ll);
#endif
            if (coeff[i] == 2)
            {
                c1 = 0;
            }
            else if (c1)
            {
                c1 = imin(++c1, 7);
            }
        }
    }

    i = maxpos[type];

    int  rice_param = 0;
    int lastCg = ((i+1) >> (LL_SUBBLOCK_LOG2));
    for (int subSet = lastCg; subSet > 0; subSet--) {
        int first = 0, last = -1;

        for (i = imin(last_pos, (subSet << LL_SUBBLOCK_LOG2) - 1); i >= ((subSet-1) << LL_SUBBLOCK_LOG2); i--)
        {
            if (i < regular_stop_pos) { // last_pos == i1&& i1 == 15
                if (i == 0 && type < 14)   //luma
                    rice_param = rice_param + 1;
                unsigned int symbol = read_u_v(1, bs_ll);
                if (symbol == 0)   //0+fixlength
                {
                    int rem = 0;
                    for (int index = 0; index < rice_param; index++)
                    {
                        symbol = read_u_v(1, bs_ll);;
                        rem = (rem << 1) + symbol;
                    }
                    coeff[i] += rem;
                    if (rem != 0 || i==last_pos) {
                        if (last == -1) {
                            last = i;
                        }
                        first = i;
                        coeff_ctr++;
                    }
                }
                else
                {
                    if (last == -1) {
                        last = i;
                    }
                    first = i;
                    coeff_ctr++;

                    int prefix_lenth = REM_PREFIX_LENGTH;
                    while (--prefix_lenth && symbol != 0)
                    {
                        symbol = read_u_v(1, bs_ll);;
                    }
                    if (symbol == 0)
                    {
                        int one_num = REM_PREFIX_LENGTH - prefix_lenth - 1;
                        int rem = 0;
                        for (int index = 0; index < rice_param; index++)
                        {
                            symbol = read_u_v(1, bs_ll);;
                            rem = (rem << 1) + symbol;
                        }
                        coeff[i] += rem + (one_num << rice_param);
                    }
                    else
                    {
                        int rem = REM_PREFIX_LENGTH << rice_param;
                        coeff[i] += rem;
                        coeff[i] += exp_golomb_decode_eq_prob(bs_ll, rice_param);
                    }
                }
                if(i==last_pos)
                    coeff[i] += 1;

                if (coeff[i] > (REM_PREFIX_LENGTH << rice_param))    //COEF_REMAIN_BIN_REDUCTION = 3
                {
                    rice_param = rice_param + 1;
                }
                if (rice_param > 4)
                    rice_param = 4;
            }
            else if (coeff[i] != 0)
            {
                if (last == -1) {
                    last = i;
                }
                first = i;

                if (coeff[i] == 2)
                {
                    if (i == 0 && type < 14)   //luma
                        rice_param = rice_param + 1;
                    unsigned int symbol = read_u_v(1, bs_ll);
                    if (symbol == 0)
                    {
                        int rem = 0;
                        for (int index = 0; index < rice_param; index++)
                        {
                            symbol = read_u_v(1, bs_ll);;
                            rem = (rem << 1) + symbol;
                        }
                        coeff[i] += rem;
                    }
                    else
                    {
                        int prefix_lenth = REM_PREFIX_LENGTH;
                        while (--prefix_lenth && symbol != 0)
                        {
                            symbol = read_u_v(1, bs_ll);;
                        }
                        if (symbol == 0)
                        {
                            int one_num = REM_PREFIX_LENGTH - prefix_lenth - 1;
                            int rem = 0;
                            for (int index = 0; index < rice_param; index++)
                            {
                                symbol = read_u_v(1, bs_ll);;
                                rem = (rem << 1) + symbol;
                            }
                            coeff[i] += rem + (one_num << rice_param);
                        }
                        else
                        {
                            int rem = REM_PREFIX_LENGTH << rice_param;
                            coeff[i] += rem;
                            coeff[i] += exp_golomb_decode_eq_prob(bs_ll, rice_param);
                        }
                    }
                }
                if (coeff[i] > (REM_PREFIX_LENGTH << rice_param))    //COEF_REMAIN_BIN_REDUCTION = 3
                {
                    rice_param = rice_param + 1;
                }
                if (rice_param > 4)
                    rice_param = 4;
            }
        }

        for (i = last; i >= first; i--) {
            if (coeff[i] != 0) {
                if (read_u_v(1, bs_ll)) {
                    coeff[i] = -coeff[i];
                }
            }
        }
    }
    return coeff_ctr;
}


int dec_readCoeff4x4_CABAC(Bitstream* bs_ll, DecodingEnvironment *dep_dp, TextureInfoContexts *tex_ctx, pel* coeff, int plane, int intra_pred_mode)
{
	int cbf = 0;
	int type = ((plane == 0) ? (LUMA_4x4) : ((plane == 1) ? CB_4x4 : CR_4x4));
#if CABAC
	cbf = biari_decode_symbol(dep_dp, tex_ctx->bcbp_contexts[type2ctx_bcbp[type]]);
#else
    cbf = read_u_v(1, bs_ll);
#endif
    int nnz_curr = 0;
	if (cbf)
	{
		int pCoeff[17];

		//===== decode significant coefficients =====
        nnz_curr = read_significant_coefficients(bs_ll, tex_ctx, dep_dp, type, pCoeff, plane);
		int i, j, k;
		int numcoeff = 16;
		for (k = 0; k < numcoeff; ++k)
		{
		    if (intra_pred_mode == INTRA_HOR)
		    {
			    i = SCAN_POS_TU4x4_HOR_PRE[k][0];
			    j = SCAN_POS_TU4x4_HOR_PRE[k][1];
		    }
		    else if (intra_pred_mode == INTRA_VER)
		    {
			    i = SCAN_POS_TU4x4_VER_PRE[k][0];
			    j = SCAN_POS_TU4x4_VER_PRE[k][1];
		    }
		    else
		    {
			    i = SCAN_POS_TU4x4_OTHER_MODE[k][0];
			    j = SCAN_POS_TU4x4_OTHER_MODE[k][1];
		    }
			coeff[4 * j + i] = pCoeff[k];
		}
	}
	else
	{
		memset(coeff, 0, (sizeof(pel) << 4));
	}
    return nnz_curr;
}

int dec_readCoeff8x8_CABAC(Bitstream* bs_ll, DecodingEnvironment *dep_dp, TextureInfoContexts *tex_ctx, pel* coeff, int intra_pred_mode)
{
	int cbf = 0;
	int type = LUMA_8x8;
#if CABAC
	cbf = biari_decode_symbol(dep_dp, tex_ctx->bcbp_contexts[type2ctx_bcbp[type]]);
#else
    cbf = read_u_v(1, bs_ll);
#endif
    int nnz_curr = 0;

	if (cbf)
	{
		int pCoeff[65];

		//===== decode significant coefficients =====
        nnz_curr = read_significant_coefficients(bs_ll, tex_ctx, dep_dp, type, pCoeff, 0);

		int i, j, k;
		int numcoeff = 64;
		for (k = 0; k < numcoeff; ++k)
		{
			if (intra_pred_mode == INTRA_HOR)
			{
				i = SCAN_POS_TU8x8_HOR_PRE[k][0];
				j = SCAN_POS_TU8x8_HOR_PRE[k][1];
			}
			else if (intra_pred_mode == INTRA_VER)
			{
				i = SCAN_POS_TU8x8_VER_PRE[k][0];
				j = SCAN_POS_TU8x8_VER_PRE[k][1];
			}
			else
			{
				i = SCAN_POS_TU8x8_OTHER_MODE[k][0];
				j = SCAN_POS_TU8x8_OTHER_MODE[k][1];
			}
			coeff[8 * j + i] = pCoeff[k];
		}
	}
	else
	{
		memset(coeff, 0, (sizeof(pel) << 6));
	}
    return nnz_curr;
}

int dec_readCoeff4x8_CABAC(Bitstream* bs_ll, DecodingEnvironment *dep_dp, TextureInfoContexts *tex_ctx, pel* coeff, int plane, int intra_pred_mode)
{
	int cbf = 0;
	int type = ((plane == 0) ? (LUMA_4x8) : ((plane == 1) ? CB_4x8 : CR_4x8));
#if CABAC
	cbf = biari_decode_symbol(dep_dp, tex_ctx->bcbp_contexts[type2ctx_bcbp[type]]);
#else
    cbf = read_u_v(1, bs_ll);
#endif
    int nnz_curr = 0;

	if (cbf)
	{
		int pCoeff[33];

		//===== decode significant coefficients =====
        nnz_curr = read_significant_coefficients(bs_ll, tex_ctx, dep_dp, type, pCoeff, plane);

		int i, j, k;
		int numcoeff = 32;
		for (k = 0; k < numcoeff; ++k)
		{
			if (intra_pred_mode == INTRA_HOR)
			{
				i = SCAN_POS_TU4x8_HOR_PRE[k][0];
				j = SCAN_POS_TU4x8_HOR_PRE[k][1];
			}
			else if (intra_pred_mode == INTRA_VER)
			{
				i = SCAN_POS_TU4x8_VER_PRE[k][0];
				j = SCAN_POS_TU4x8_VER_PRE[k][1];
			}
			else
			{
				i = SCAN_POS_TU4x8_OTHER_MODE[k][0];
				j = SCAN_POS_TU4x8_OTHER_MODE[k][1];
			}
			coeff[4 * j + i] = pCoeff[k];
		}
	}
	else
	{
		memset(coeff, 0, (sizeof(pel) << 5));
	}
    return nnz_curr;
}

static unsigned int exp_golomb_decode_eq_prob_0(Bitstream* bs_ll,
    int k)
{
    unsigned int l;
    int symbol = 0;
    int binary_symbol = 0;

    do
    {
        l = read_u_v(1, bs_ll);
        if (l == 0)
        {
            symbol += (1 << k);
            ++k;
        }
    } while (l != 1);

    while (k--)                             //next binary part
        if (read_u_v(1, bs_ll) == 1)
            binary_symbol |= (1 << k);

    return (unsigned int)(symbol + binary_symbol);
}

static unsigned int exp_golomb_decode_eq_prob(Bitstream* bs_ll,
                                              int k)
{
  unsigned int l;
  int symbol = 0;
  int binary_symbol = 0;

  do
  {
    l = read_u_v(1, bs_ll);
    if (l == 1)
    {
      symbol += (1<<k);
      ++k;
    }
  }
  while (l!=0);

  while (k--)                             //next binary part
    if (read_u_v(1, bs_ll) ==1)
      binary_symbol |= (1<<k);

  return (unsigned int) (symbol + binary_symbol);
}


static unsigned int unary_exp_golomb_mv_decode(Bitstream* bs_ll, DecodingEnvironmentPtr dep_dp,
	BiContextTypePtr ctx,
	unsigned int max_bin)
{
#if CABAC
    unsigned int symbol = biari_decode_symbol(dep_dp, ctx);
#else
    unsigned int symbol = read_u_v(1, bs_ll);
#endif
	if (symbol == 0)
		return 0;
	else
	{
        symbol += exp_golomb_decode_eq_prob(bs_ll, 3);
		return symbol;
	}
}


static inline int iabs(int x)
{
	static const int INT_BITS = (sizeof(int) * CHAR_BIT) - 1;
	int y = x >> INT_BITS;
	return (x ^ y) - y;
}

int read_MVD_CABAC(Bitstream* bs_ll, DecodingEnvironment *dep_dp, MotionInfoContexts *mot_ctx, int is_MVDy, int cond)
{
	MotionInfoContexts *ctx = mot_ctx;
	int a = 0;
	//int act_ctx;
	int act_sym;
	int k = is_MVDy; // MVD component

	a = 5 * k;

    if(is_MVDy && !cond)
    {
        act_sym = 1;
    }
    else
    {
#if CABAC
        act_sym = biari_decode_symbol(dep_dp, ctx->mv_res_contexts[0] + a);
#else
        act_sym = read_u_v(1, bs_ll);
#endif
    }

	if (act_sym != 0)
	{
		a = 5 * k;
		act_sym = unary_exp_golomb_mv_decode(bs_ll, dep_dp, ctx->mv_res_contexts[1] + a, 3) + 1;

		if (read_u_v(1, bs_ll))
			act_sym = -act_sym;
	}
	return act_sym;
}
