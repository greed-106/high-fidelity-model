
/*!
 ***************************************************************************
 * \file
 *    biaridecod.h
 *
 * \brief
 *    Headerfile for binary arithmetic decoder routines
 *
 * \author
 *    Detlev Marpe,
 *    Gabi Blaettermann
 *    Copyright (C) 2000 HEINRICH HERTZ INSTITUTE All Rights Reserved.
 *
 * \date
 *    21. Oct 2000
 **************************************************************************
 */

#ifndef _BIARIDECOD_H_
#define _BIARIDECOD_H_

#include "context_ini.h"
#include "cabac.h"

/************************************************************************
 * D e f i n i t i o n s
 ***********************************************************************
 */

extern void arideco_start_decoding(DecodingEnvironmentPtr dep, unsigned char *code_buffer, int firstbyte, int *code_len);
extern void arideco_done_decoding(DecodingEnvironmentPtr dep);
extern unsigned int biari_decode_symbol(DecodingEnvironment *dep, BiContextType *bi_ct );
extern unsigned int biari_decode_final(DecodingEnvironmentPtr dep);
#endif  // BIARIDECOD_H_

