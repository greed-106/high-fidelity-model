/*!
 *************************************************************************************
 * \file biaridecod.c
 *
 * \brief
 *   Binary arithmetic decoder routines.
 *
 *   This modified implementation of the M Coder is based on JVT-U084 
 *   with the choice of M_BITS = 16.
 *
 * \date
 *    21. Oct 2000
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Detlev Marpe
 *    - Gabi Blaettermann
 *    - Gunnar Marten
 *************************************************************************************
 */

#include "biaridecod.h"
#include "cabac.h"
#include <assert.h>


#define B_BITS    10      // Number of bits to represent the whole coding interval
#define HALF      0x01FE  //(1 << (B_BITS-1)) - 2
#define QUARTER   0x0100  //(1 << (B_BITS-2))



/*!
 ************************************************************************
 * \brief
 *    finalize arithetic decoding():
 ************************************************************************
 */
void arideco_done_decoding(DecodingEnvironmentPtr dep)
{
  (*dep->Dcodestrm_len)++;

}

/*!
 ************************************************************************
 * \brief
 *    read one byte from the bitstream
 ************************************************************************
 */
static inline void getbyte(DecodingEnvironmentPtr dep)
{     
    if (dep->DbitsNeeded >= 0) {
        uint8_t new_byte = dep->Dcodestrm[(*dep->Dcodestrm_len)++];
        dep->Dvalue += new_byte << dep->DbitsNeeded;
        dep->DbitsNeeded -= 8;
    }
}

static inline int getbit(DecodingEnvironmentPtr dep, int bit)
{
    int i;
    for (i = 0; i < bit; i++) {
        if (--dep->DbitsNeeded < 0) {
            dep->Dbuffer = dep->Dcodestrm[(*dep->Dcodestrm_len)++];
            dep->DbitsNeeded = 7;
        }

        dep->Dvalue = (dep->Dvalue << 1) | ((dep->Dbuffer >> dep->DbitsNeeded) & 0x01);
    }
}
/*!
 ************************************************************************
 * \brief
 *    Initializes the DecodingEnvironment for the arithmetic coder
 ************************************************************************
 */
void arideco_start_decoding(DecodingEnvironmentPtr dep, unsigned char *code_buffer, int firstbyte, int *code_len)
{

  dep->Dcodestrm      = code_buffer;
  dep->Dcodestrm_len  = code_len;
  *dep->Dcodestrm_len = firstbyte;

  dep->Drange = 0x1FF;
  dep->Dvalue = 0;
  dep->DbitsNeeded = 0;
  getbit(dep, 9);
  /*
  dep->DbitsNeeded = 8;
  getbyte(dep);
  getbyte(dep);
  dep->Dvalue <<= 1;
  dep->DbitsNeeded = -7;
  */
}

int ace_get_shift(int v)
{
#ifdef _WIN32
    unsigned long index;
    _BitScanReverse(&index, v);
    return 8 - index;
#else
    return __builtin_clz(v) - 23;
#endif
}

static const uint16_t cwr2LGS[10] = { 427, 427, 427, 197, 95, 46, 23, 12, 6, 3 };

/*!
************************************************************************
* \brief
*    biari_decode_symbol():
* \return
*    the decoded symbol
************************************************************************
*/
unsigned int biari_decode_symbol(DecodingEnvironment *dep, BiContextType *bi_ct)
{
    uint16_t bit = bi_ct->MPS;
    uint16_t prob_lps = bi_ct->prob_lps;
    uint32_t rLPS = LBAC_GET_LG_PMPS(prob_lps);
    uint32_t rMPS = dep->Drange - rLPS;
    int s_flag = rMPS < LBAC_QUAR_HALF_PROB;
    rMPS |= 0x100;
    
    if (s_flag) {
        getbit(dep, 1);
    }
    if (dep->Dvalue < rMPS) { // MPS
        dep->Drange = rMPS;
        prob_lps -= (prob_lps >> LBAC_UPDATE_CWR) + (prob_lps >> (LBAC_UPDATE_CWR + 2));
    }
    else { // LPS
        bit = 1 - bit;
        rLPS = (dep->Drange << s_flag) - rMPS;
        int shift = ace_get_shift(rLPS);
        dep->Drange = rLPS << shift;
        dep->Dvalue = dep->Dvalue - rMPS;
        getbit(dep, shift);
        prob_lps += cwr2LGS[LBAC_UPDATE_CWR] >> (11 - LBAC_PROB_BITS);
        if (prob_lps > LBAC_HALF_PROB) {
            prob_lps = LBAC_MAX_PROB - prob_lps;
            bi_ct->MPS = !bi_ct->MPS;
        }
    }

    bi_ct->prob_lps = prob_lps;

    return bit;
}

unsigned int biari_decode_symbol_v2(DecodingEnvironment *dep, BiContextType *bi_ct )
{
    uint16_t bit = bi_ct->MPS;
    uint16_t prob_lps = bi_ct->prob_lps;
    uint32_t rLPS = LBAC_GET_LG_PMPS(prob_lps);
    uint32_t rMPS = dep->Drange - rLPS;
    int s_flag = rMPS < LBAC_QUAR_HALF_PROB;
    rMPS |= 0x100;
    uint32_t scaled_rMPS = rMPS << (8 - s_flag);

    if (dep->Dvalue < scaled_rMPS) { // MPS
        dep->Drange = rMPS;
        if (s_flag) {
            dep->Dvalue <<= 1;
            dep->DbitsNeeded++;
            getbyte(dep);
        }
        prob_lps -= (prob_lps >> LBAC_UPDATE_CWR) + (prob_lps >> (LBAC_UPDATE_CWR + 2));
    } else { // LPS
        bit = 1 - bit;
        rLPS = (dep->Drange << s_flag) - rMPS;
        int shift = ace_get_shift(rLPS);
        dep->Drange = rLPS << shift;
        dep->Dvalue = (dep->Dvalue - scaled_rMPS) << (s_flag + shift);
        dep->DbitsNeeded += (s_flag + shift);
        while (dep->DbitsNeeded >= 0) {
            getbyte(dep);
        }
        prob_lps += cwr2LGS[LBAC_UPDATE_CWR] >> (11 - LBAC_PROB_BITS);
        if (prob_lps > LBAC_HALF_PROB) {
            prob_lps = LBAC_MAX_PROB - prob_lps;
            bi_ct->MPS = !bi_ct->MPS;
        }
    }

    bi_ct->prob_lps = prob_lps;

    return bit;
}

/*!
 ************************************************************************
 * \brief
 *    biari_decode_symbol_final():
 * \return
 *    the decoded symbol
 ************************************************************************
 */

unsigned int biari_decode_final(DecodingEnvironmentPtr dep)
{
    uint8_t s_flag = dep->Drange - 1 < LBAC_QUAR_HALF_PROB;
    uint32_t rMPS = (dep->Drange - 1) | 0x100;
    
    if (s_flag) {
        getbit(dep, 1);
    }

    if (dep->Dvalue < rMPS) { // MPS
        dep->Drange = rMPS;
        return 0;
    }
    else { // LPS
        uint32_t rLPS = s_flag ? ((dep->Drange << 1) - rMPS) : 1;
        int shift = ace_get_shift(rLPS);
        dep->Drange = rLPS << shift;
        dep->Dvalue = (dep->Dvalue - rMPS) ;
        getbit(dep,  shift);

        return 1;
    }
}


unsigned int biari_decode_final_v2(DecodingEnvironmentPtr dep)
{
    uint8_t s_flag = dep->Drange - 1 < LBAC_QUAR_HALF_PROB;
    uint32_t rMPS = (dep->Drange - 1) | 0x100;
    uint32_t scaled_rMPS = rMPS << (8 - s_flag);
    
    if (dep->Dvalue < scaled_rMPS) { // MPS
        dep->Drange = rMPS;
        if (s_flag) {
            dep->Dvalue <<= 1;
            dep->DbitsNeeded++;
            getbyte(dep);
        }

        return 0;
    } else { // LPS
        uint32_t rLPS = s_flag ? ((dep->Drange << 1) - rMPS) : 1;
        int shift = ace_get_shift(rLPS);
        dep->Drange = rLPS << shift;
        dep->Dvalue = (dep->Dvalue - scaled_rMPS) << (shift + s_flag);
        dep->DbitsNeeded += (s_flag + shift);
        while (dep->DbitsNeeded > 0) {
            getbyte(dep);
        }

        return 1;
    }
}
