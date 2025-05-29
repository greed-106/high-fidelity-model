
/*!
 *************************************************************************************
 * \file biariencode.c
 *
 * \brief
 *   Routines for binary arithmetic encoding.
 *
 *   This modified implementation of the M Coder is based on JVT-U084 
 *   with the choice of M_BITS = 16.
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Detlev Marpe
 *    - Gabi Blaettermann
 *    - Gunnar Marten
 *************************************************************************************
 */


#include "biariencode.h"
#include "cabac.h"
#include <assert.h>

/*!
 ************************************************************************
 * inline functions for writing bytes of code
 ***********************************************************************
 */

static inline void put_buffer(EncodingEnvironmentPtr eep)
{
    while (eep->Epbuf >= 0) {
        eep->Ecodestrm[(*eep->Ecodestrm_len)++] = (byte)((eep->Ebuffer >> ((eep->Epbuf--) << 3)) & 0xFF);
    }
    eep->Ebuffer = 0;
}

static inline void put_one_byte(EncodingEnvironmentPtr eep, int b)
{
    if (eep->Epbuf == 3) {
        put_buffer(eep);
    }
    eep->Ebuffer <<= 8;
    eep->Ebuffer += (b);

    ++(eep->Epbuf);
}

static inline void propagate_carry(EncodingEnvironmentPtr eep)
{
    uint32_t lead_byte = eep->Elow >> (24 - eep->Ebits_to_go);
    eep->Ebits_to_go += 8;
    eep->Elow &= (0xffffffffu >> eep->Ebits_to_go);

    if (lead_byte < 0xFF) {
        while (eep->Echunks_outstanding > 0) {
            put_one_byte(eep, 0xFF);
            --(eep->Echunks_outstanding);
        }
        put_one_byte(eep, lead_byte);
    } else if (lead_byte > 0xFF) {
        eep->Ebuffer++;
        while (eep->Echunks_outstanding > 0) {
            put_one_byte(eep, 0);
            --(eep->Echunks_outstanding);
        }
        put_one_byte(eep, lead_byte & 0xFF);
    } else {
        eep->Echunks_outstanding++;
    }
}

/*!
 ************************************************************************
 * \brief
 *    Initializes the EncodingEnvironment for the arithmetic coder
 ************************************************************************
 */
void arienco_start_encoding(EncodingEnvironmentPtr eep, unsigned char *code_buffer, int *code_len)
{
    eep->Elow = 0;
    eep->Echunks_outstanding = 0;
    eep->Ebuffer = 0;
    eep->Epbuf = -1;  // to remove redundant chunk ^^
    eep->Ebits_to_go = 23; // to swallow first redundant bit

    eep->Ecodestrm = code_buffer;
    eep->Ecodestrm_len = code_len;

    eep->Erange = 0x1FF;
}

/*!
 ************************************************************************
 * \brief
 *    Terminates the arithmetic codeword, writes stop bit and stuffing bytes (if any)
 ************************************************************************
 */
void arienco_done_encoding(EncodingEnvironmentPtr eep)
{
    if (eep->Elow >> (32 - eep->Ebits_to_go)) {
        while (eep->Echunks_outstanding != 0) {
            put_one_byte(eep, 0x00);
            eep->Echunks_outstanding--;
        }
        eep->Elow -= 1 << (32 - eep->Ebits_to_go);
    } else {
        while (eep->Echunks_outstanding != 0) {
            put_one_byte(eep, 0xFF);
            eep->Echunks_outstanding--;
        }
    }

    eep->Elow |= (1 << 7);
    while (24 > eep->Ebits_to_go) {
        uint32_t lead_byte = eep->Elow >> (24 - eep->Ebits_to_go);
        eep->Ebits_to_go += 8;
        eep->Elow &= (0xffffffffu >> eep->Ebits_to_go);
        put_one_byte(eep, lead_byte);
    }
    put_buffer(eep);

    // add termination slice padding bits
    if (eep->Ebits_to_go & 0x07) {
        // add the termination slice padding bits
        eep->Elow = (eep->Elow << 1) + 1;
        eep->Elow <<= eep->Ebits_to_go & 0x07;
        // write the last byte of low
        put_one_byte(eep, eep->Elow);
    }
    put_buffer(eep);
}

static const uint16_t cwr2LGS[10] = { 427, 427, 427, 197, 95, 46, 23, 12, 6, 3 };

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

/*!
 ************************************************************************
 * \brief
 *    Actually arithmetic encoding of one binary symbol by using
 *    the probability estimate of its associated context model
 ************************************************************************
 */
void biari_encode_symbol(EncodingEnvironmentPtr eep, int symbol, BiContextTypePtr bi_ct)
{
    unsigned int range = eep->Erange;

    uint32_t prob_lps = bi_ct->prob_lps;
    uint32_t rLPS = LBAC_GET_LG_PMPS(prob_lps);
    uint32_t rMPS = range - rLPS;

    int s_flag = rMPS < LBAC_QUAR_HALF_PROB;
    rMPS |= 0x100;
    assert(range >= rLPS);

    if ((symbol != 0) == bi_ct->MPS) { // MPS
        if (s_flag) {
            eep->Elow <<= 1;
            if (--eep->Ebits_to_go < 12) {
                propagate_carry(eep);
            }
        }
        eep->Erange = rMPS;

        prob_lps = prob_lps - (prob_lps >> LBAC_UPDATE_CWR) - (prob_lps >> (LBAC_UPDATE_CWR + 2));
    } else { // LPS
        rLPS = (range << s_flag) - rMPS;
        int shift = ace_get_shift(rLPS);
        eep->Erange = rLPS << shift;
        eep->Elow = ((eep->Elow << s_flag) + rMPS) << shift;
        eep->Ebits_to_go -= (shift + s_flag);
        if (eep->Ebits_to_go < 12) {
            propagate_carry(eep);
        }

        // update probability estimation
        prob_lps += cwr2LGS[LBAC_UPDATE_CWR] >> (11 - LBAC_PROB_BITS);
        if (prob_lps > LBAC_HALF_PROB) {
            prob_lps = LBAC_MAX_PROB - prob_lps;
            bi_ct->MPS = !bi_ct->MPS;
        }
    }

    bi_ct->prob_lps = prob_lps;
}

/*!
 ************************************************************************
 * \brief
 *    Arithmetic encoding for last symbol before termination
 ************************************************************************
 */
void biari_encode_symbol_final(EncodingEnvironmentPtr eep, int symbol)
{
    int s_flag = (eep->Erange == LBAC_QUAR_HALF_PROB);
    uint32_t rMPS = (eep->Erange - 1) | 0x100;
    eep->Erange -= 2;
    if (symbol) {
        eep->Erange = LBAC_QUAR_HALF_PROB;
        eep->Elow = ((eep->Elow << s_flag) + rMPS) << 8;
        eep->Ebits_to_go -= (8 + s_flag);
        if (eep->Ebits_to_go < 12) {
            propagate_carry(eep);
        }
    } else {
        if (s_flag) {
            eep->Elow <<= 1;
            if (--eep->Ebits_to_go < 12) {
                propagate_carry(eep);
            }
        }
        eep->Erange = rMPS;
    }
}
