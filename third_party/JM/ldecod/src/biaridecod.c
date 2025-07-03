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

#define NUN_VALUE_BOUND   254

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
#if LBAC_OPT
        dep->Dvalue += new_byte << dep->DbitsNeeded;
#else
        dep->Dbuffer = new_byte;
#endif
        dep->DbitsNeeded -= 8;
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

  dep->DbitsNeeded = 8;


#if LBAC_OPT
  dep->Drange = 0x1FF;
  dep->Dvalue = 0;
  getbyte(dep);
  getbyte(dep);
  dep->Dvalue <<= 1;
  dep->DbitsNeeded = -7;

#else
  dep->value_s_bound = NUN_VALUE_BOUND;
  dep->is_value_bound;
  dep->max_value_s = 0;
  dep->is_value_domain = 1;

  dep->s1 = 0;
  dep->t1 = QUARTER - 1;
  dep->value_s = 0;
  dep->value_t = 0;

  dep->DbitsNeeded = 0;
  for (int i = 0; i < B_BITS - 1; i++) {
      if (++dep->DbitsNeeded > 0) {
          getbyte(dep);
      }
      dep->value_t = (dep->value_t << 1) | ((dep->Dbuffer >> (-dep->DbitsNeeded)) & 0x01);
  }
#endif
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
unsigned int biari_decode_symbol(DecodingEnvironment *dep, BiContextType *bi_ct )
{
#if LBAC_OPT
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
#else
    unsigned char s1 = dep->s1;
    unsigned char value_s = dep->value_s;
    unsigned int t1 = dep->t1;
    unsigned int value_t = dep->value_t;

    if (dep->is_value_domain == 1 || (s1 == dep->value_s_bound && dep->is_value_bound == 1)) {
        // value_t is in R domain s1=0 or s1 == value_s_bound
        s1 = 0;
        value_s = 0;
        while (value_t < QUARTER && value_s < dep->value_s_bound) {
            if (++dep->DbitsNeeded > 0) {
                getbyte(dep);
            }
            // Shift in next bit and add to value
            value_t = (value_t << 1) | ((dep->Dbuffer >> (-dep->DbitsNeeded)) & 0x01);
            value_s++;
        }
        if (value_t < QUARTER) {
            dep->is_value_bound = 1;
        } else {
            dep->is_value_bound = 0;
        }

        value_t = value_t & 0xff;
    }

    unsigned char s_flag;
    unsigned int t2;
    unsigned char s2;
    unsigned int lg_pmps = bi_ct->prob_lps;
    if (t1 >= lg_pmps) {
        s2 = s1;
        t2 = t1 - lg_pmps; //8bits
        s_flag = 0;
    } else {
        s2 = s1 + 1;
        t2 = 256 + t1 - lg_pmps; //8bits
        s_flag = 1;
    }

    assert(value_s <= dep->value_s_bound);

    unsigned char bit = bi_ct->MPS;
    if ((s2 > value_s || (s2 == value_s && value_t >= t2)) && dep->is_value_bound == 0) { // LPS
        bit = !bit; //LPS
        dep->is_value_domain = 1;
        unsigned int t_rlps = (s_flag == 0) ? (lg_pmps) : (t1 + lg_pmps);

        if (s2 == value_s) {
            value_t = (value_t - t2);
        } else {
            if (++dep->DbitsNeeded > 0) {
                getbyte(dep);
            }
            // Shift in next bit and add to value
            value_t = (value_t << 1) | ((dep->Dbuffer >> (-dep->DbitsNeeded)) & 0x01);
            value_t = 256 + value_t - t2;
        }

        // restore range
        while (t_rlps < QUARTER) {
            t_rlps = t_rlps << 1;

            if (++dep->DbitsNeeded > 0) {
                getbyte(dep);
            }
            // Shift in next bit and add to value
            value_t = (value_t << 1) | ((dep->Dbuffer >> (-dep->DbitsNeeded)) & 0x01);
        }

        dep->s1 = 0;
        dep->t1 = t_rlps & 0xff;

        lg_pmps += cwr2LGS[LBAC_UPDATE_CWR] >> (11 - LBAC_PROB_BITS);
        if (lg_pmps > LBAC_HALF_PROB) {
            lg_pmps = LBAC_MAX_PROB - lg_pmps;
            bi_ct->MPS = !bi_ct->MPS;
        }
    } else { // MPS
        dep->s1 = s2;
        dep->t1 = t2;
        dep->is_value_domain = 0;
        lg_pmps -= (lg_pmps >> LBAC_UPDATE_CWR) + (lg_pmps >> (LBAC_UPDATE_CWR + 2));
    }

    dep->value_s = value_s;
    dep->value_t = value_t;
    bi_ct->prob_lps = lg_pmps;

#endif // LBAC_OPT

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
#if LBAC_OPT
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
#else
    unsigned char s1 = dep->s1;
    unsigned char value_s = dep->value_s;
    unsigned int t1 = dep->t1;
    unsigned int value_t = dep->value_t;

    if (dep->is_value_domain == 1 || (s1 == dep->value_s_bound && dep->is_value_bound == 1)) {
        // value_t is in R domain s1=0 or s1 == value_s_bound
        s1 = 0;
        value_s = 0;
        while (value_t < QUARTER && value_s < dep->value_s_bound) {
            if (++dep->DbitsNeeded > 0) {
                getbyte(dep);
            }
            // Shift in next bit and add to value
            value_t = (value_t << 1) | ((dep->Dbuffer >> (-dep->DbitsNeeded)) & 0x01);
            value_s++;
        }
        if (value_t < QUARTER) {
            dep->is_value_bound = 1;
        } else {
            dep->is_value_bound = 0;
        }

        value_t = value_t & 0xff;
    }

    unsigned char s_flag;
    unsigned int t2;
    unsigned char s2;
    unsigned int lg_pmps = 1;
    if (t1 >= lg_pmps) {
        s2 = s1;
        t2 = t1 - lg_pmps; //8bits
        s_flag = 0;
    } else {
        s2 = s1 + 1;
        t2 = 256 + t1 - lg_pmps; //8bits
        s_flag = 1;
    }

    assert(value_s <= dep->value_s_bound);

    unsigned char bit = 0;
    if ((s2 > value_s || (s2 == value_s && value_t >= t2)) && dep->is_value_bound == 0) { // LPS
        bit = !bit; //LPS
        dep->is_value_domain = 1;
        unsigned int t_rlps = (s_flag == 0) ? (lg_pmps) : (t1 + lg_pmps);

        if (s2 == value_s) {
            value_t = (value_t - t2);
        } else {
            if (++dep->DbitsNeeded > 0) {
                getbyte(dep);
            }
            // Shift in next bit and add to value
            value_t = (value_t << 1) | ((dep->Dbuffer >> (-dep->DbitsNeeded)) & 0x01);
            value_t = 256 + value_t - t2;
        }

        // restore range
        while (t_rlps < QUARTER) {
            t_rlps = t_rlps << 1;

            if (++dep->DbitsNeeded > 0) {
                getbyte(dep);
            }
            // Shift in next bit and add to value
            value_t = (value_t << 1) | ((dep->Dbuffer >> (-dep->DbitsNeeded)) & 0x01);
        }

        dep->s1 = 0;
        dep->t1 = t_rlps & 0xff;
    } else { // MPS
        dep->s1 = s2;
        dep->t1 = t2;
        dep->is_value_domain = 0;
    }

    dep->value_s = value_s;
    dep->value_t = value_t;

    return bit;
#endif
}
