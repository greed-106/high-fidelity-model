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
 *    read bits from the bitstream
 ************************************************************************
 */
static inline void readBits(DecodingEnvironmentPtr dep, int n)
{
    while (dep->Dremained < n) {
        uint8_t new_byte = dep->Dcodestrm[(*dep->Dcodestrm_len)++];
        dep->Dbuffer = (dep->Dbuffer << 8) + new_byte;
        dep->Dremained += 8;
    }

    dep->Dremained -= n;
#if LBAC_OPT
    dep->Dvalue = (dep->Dvalue << n) | (dep->Dbuffer >> dep->Dremained);
#else
    dep->value_t = (dep->value_t << n) | (dep->Dbuffer >> dep->Dremained);
#endif
    dep->Dbuffer &= (1 << dep->Dremained) - 1;
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

  dep->Dremained = 0;
  dep->Dbuffer = 0;

#if LBAC_OPT
  dep->Drange = 0x1FF;
  dep->Dvalue = 0;
#else
  dep->value_s_bound = NUN_VALUE_BOUND;
  dep->is_value_bound;
  dep->max_value_s = 0;
  dep->is_value_domain = 1;

  dep->s1 = 0;
  dep->t1 = QUARTER - 1;
  dep->value_s = 0;
  dep->value_t = 0;
#endif

  readBits(dep, B_BITS - 1);
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
    if (s_flag) {
        readBits(dep, 1);
    }
    rMPS |= 0x100;

    if (dep->Dvalue < rMPS) { // MPS
        dep->Drange = rMPS;
        prob_lps -= (prob_lps >> LBAC_UPDATE_CWR) + (prob_lps >> (LBAC_UPDATE_CWR + 2));
    } else { // LPS
        bit = 1 - bit;
        rLPS = (dep->Drange << s_flag) - rMPS;
        dep->Drange = rLPS;
        dep->Dvalue = dep->Dvalue - rMPS;
        while (dep->Drange < 256) {
            dep->Drange <<= 1;
            readBits(dep, 1);
        }
        prob_lps += cwr2LGS[LBAC_UPDATE_CWR] >> (11 - LBAC_PROB_BITS);
        if (prob_lps > LBAC_HALF_PROB) {
            prob_lps = LBAC_MAX_PROB - prob_lps;
            bi_ct->MPS = !bi_ct->MPS;
        }
    }

    assert(dep->Drange <= 0x1FF);
    assert(dep->Dvalue <= 0x1FF);

    bi_ct->prob_lps = prob_lps;
#else
    unsigned char s1 = dep->s1;
    unsigned int t1 = dep->t1;

    if (dep->is_value_domain == 1 || (s1 == dep->value_s_bound && dep->is_value_bound == 1)) {
        // value_t is in R domain s1=0 or s1 == value_s_bound
        s1 = 0;
        dep->value_s = 0;
        while (dep->value_t < QUARTER && dep->value_s < dep->value_s_bound) {
            readBits(dep, 1);
            dep->value_s++;
        }
        if (dep->value_t < QUARTER) {
            dep->is_value_bound = 1;
        } else {
            dep->is_value_bound = 0;
        }

        dep->value_t = dep->value_t & 0xff;
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

    assert(dep->value_s <= dep->value_s_bound);

    unsigned char bit = bi_ct->MPS;
    if ((s2 > dep->value_s || (s2 == dep->value_s && dep->value_t >= t2)) && dep->is_value_bound == 0) { // LPS
        bit = !bit; //LPS
        dep->is_value_domain = 1;
        unsigned int t_rlps = (s_flag == 0) ? (lg_pmps) : (t1 + lg_pmps);

        if (s2 == dep->value_s) {
            dep->value_t = (dep->value_t - t2);
        } else {
            readBits(dep, 1);
            dep->value_t += 256 - t2;
        }

        // restore range
        while (t_rlps < QUARTER) {
            t_rlps = t_rlps << 1;
            readBits(dep, 1);
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
    if (s_flag) {
        readBits(dep, 1);
    }
    uint32_t rMPS = (dep->Drange - 1) | 0x100;

    if (dep->Dvalue < rMPS) { // MPS
        dep->Drange = rMPS;
        return 0;
    } else { // LPS
        uint32_t rLPS = s_flag ? ((dep->Drange << 1) - rMPS) : 1;
        int shift = ace_get_shift(rLPS);
        dep->Drange = rLPS << shift;
        dep->Dvalue = dep->Dvalue - rMPS;
        readBits(dep, s_flag);

        return 1;
    }
#else
    unsigned char s1 = dep->s1;
    unsigned int t1 = dep->t1;

    if (dep->is_value_domain == 1 || (s1 == dep->value_s_bound && dep->is_value_bound == 1)) {
        // value_t is in R domain s1=0 or s1 == value_s_bound
        s1 = 0;
        dep->value_s = 0;
        while (dep->value_t < QUARTER && dep->value_s < dep->value_s_bound) {
            readBits(dep, 1);
            dep->value_s++;
        }
        if (dep->value_t < QUARTER) {
            dep->is_value_bound = 1;
        } else {
            dep->is_value_bound = 0;
        }

        dep->value_t = dep->value_t & 0xff;
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

    assert(dep->value_s <= dep->value_s_bound);

    unsigned char bit = 0;
    if ((s2 > dep->value_s || (s2 == dep->value_s && dep->value_t >= t2)) && dep->is_value_bound == 0) { // LPS
        bit = !bit; //LPS
        dep->is_value_domain = 1;
        unsigned int t_rlps = (s_flag == 0) ? (lg_pmps) : (t1 + lg_pmps);

        if (s2 == dep->value_s) {
            dep->value_t = (dep->value_t - t2);
        } else {
            readBits(dep, 1);
            dep->value_t = 256 + dep->value_t - t2;
        }

        // restore range
        while (t_rlps < QUARTER) {
            t_rlps = t_rlps << 1;
            readBits(dep, 1);
        }

        dep->s1 = 0;
        dep->t1 = t_rlps & 0xff;
    } else { // MPS
        dep->s1 = s2;
        dep->t1 = t2;
        dep->is_value_domain = 0;
    }

    return bit;
#endif
}
