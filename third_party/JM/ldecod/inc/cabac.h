
/*!
 ***************************************************************************
 * \file
 *    cabac.h
 *
 * \brief
 *    Header file for entropy coding routines
 *
 * \author
 *    Detlev Marpe                                                         \n
 *    Copyright (C) 2000 HEINRICH HERTZ INSTITUTE All Rights Reserved.
 *
 * \date
 *    21. Oct 2000 (Changes by Tobias Oelbaum 28.08.2001)
 ***************************************************************************
 */

#ifndef _CABAC_H_
#define _CABAC_H_

#include "context_ini.h"
#include "vlc.h"

typedef int32_t pel;

typedef struct
{
    unsigned int    Drange;
    unsigned int    Dvalue;
    int             DbitsNeeded;
    byte            *Dcodestrm;
    int             *Dcodestrm_len;
} DecodingEnvironment;

typedef DecodingEnvironment* DecodingEnvironmentPtr;

int dec_readTuSize_CABAC(DecodingEnvironment *dep_dp, TextureInfoContexts *tex_ctx);
int dec_readPreMode_CABAC(DecodingEnvironment *dep_dp, TextureInfoContexts *tex_ctx, int component);

int dec_read_MB_Mode_CABAC(DecodingEnvironment *dep_dp, MotionInfoContexts *mot_ctx);
int dec_read_inter_Mode_CABAC(DecodingEnvironment *dep_dp, MotionInfoContexts *mot_ctx);
int dec_read_inter_split_CABAC(DecodingEnvironment *dep_dp, MotionInfoContexts *mot_ctx);
int dec_read_inter_mvd_CABAC(DecodingEnvironment *dep_dp, MotionInfoContexts *mot_ctx);
int read_MVD_CABAC(Bitstream* bs_ll, DecodingEnvironment *dep_dp, MotionInfoContexts *mot_ctx, int is_MVDy, int cond);

int dec_readCoeff4x4_CABAC(Bitstream*bs_ll,DecodingEnvironment *dep_dp, TextureInfoContexts *tex_ctx, pel* coeff,  int plane, int intra_pred_mode);
int dec_readCoeff8x8_CABAC(Bitstream*bs_ll, DecodingEnvironment *dep_dp, TextureInfoContexts *tex_ctx, pel* coeff, int intra_pred_mode);
int dec_readCoeff4x8_CABAC(Bitstream*bs_ll, DecodingEnvironment *dep_dp, TextureInfoContexts *tex_ctx, pel* coeff, int plane, int intra_pred_mode);

int read_significant_coefficients(Bitstream*bs_ll, TextureInfoContexts *tex_ctx, DecodingEnvironmentPtr dep_dp,
	int                     type,
	int                    *coeff,
	int plane);

#endif  // _CABAC_H_

