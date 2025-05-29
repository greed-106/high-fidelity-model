
/*!
 ***************************************************************************
 * \file
 *    cabac.h
 *
 * \brief
 *    Headerfile for entropy coding routines
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

//typedef unsigned char byte;
typedef int32_t       pel;

typedef struct encoding_environment EncodingEnvironment;
typedef EncodingEnvironment *EncodingEnvironmentPtr;
struct encoding_environment
{
    unsigned int  Elow, Erange;
    unsigned int  Ebuffer;
    unsigned int  Ebits_to_go;
    unsigned int  Echunks_outstanding;
    int           Epbuf;
    byte          *Ecodestrm;
    int           *Ecodestrm_len;

    int           cabac_encoding;

    int byte_pos;
};

static const byte maxpos       [] = {15, 14, 63, 31, 31, 15,  3, 14,  7, 31, 15, 14, 63, 31, 31, 15, 15, 31, 63, 31, 31, 15};
static const byte type2ctx_bcbp[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21};
static const byte type2ctx_map [] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 }; // 8
static const byte type2ctx_last[] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 }; // 8
static const byte type2ctx_one [] = { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 }; // 7

extern const byte* pos2ctx_map     [];
extern const byte* pos2ctx_map_int [];
extern const byte* pos2ctx_last    [];

int enc_writePuSize_CABAC(EncodingEnvironmentPtr eep_dp, TextureInfoContexts* tex_ctx, int pu_size);
int enc_writePreMode_CABAC(EncodingEnvironmentPtr eep_dp, TextureInfoContexts* tex_ctx,int pred_mode, int component);
int enc_writeCoeff4x4_CABAC(Bitstream *bitstream, EncodingEnvironmentPtr eep_dp, TextureInfoContexts* tex_ctx, pel* coeff, int plane, int pred_mode);
int enc_writeCoeff8x8_CABAC(Bitstream *bitstream, EncodingEnvironmentPtr eep_dp, TextureInfoContexts* tex_ctx, pel* coeff, int plane, int pred_mode);
int enc_writeCoeff4x8_CABAC(Bitstream *bitstream, EncodingEnvironmentPtr eep_dp, TextureInfoContexts* tex_ctx, pel* coeff, int plane, int pred_mode);
//void write_significance_map(EncodingEnvironmentPtr eep_dp, int type, int coeff[], TextureInfoContexts*  tex_ctx);
//int write_significant_coefficients(Bitstream *bitstream, EncodingEnvironmentPtr eep_dp, int type, int coeff[], TextureInfoContexts*  tex_ctx);
int writeMVD_CABAC(Bitstream *bitstream, EncodingEnvironmentPtr eep_dp, MotionInfoContexts* mot_ctx,int MVD, int MVP, int is_MVDy, int cond);
int write_MB_mode_CABAC(EncodingEnvironmentPtr eep_dp, MotionInfoContexts* mot_ctx, int mb_mode);
int write_inter_mode_CABAC(EncodingEnvironmentPtr eep_dp, MotionInfoContexts* mot_ctx, int inter_no_residual_flag);
int write_inter_mvd_CABAC(EncodingEnvironmentPtr eep_dp, MotionInfoContexts* mot_ctx, int mvd_flag);
#endif  // CABAC_H

