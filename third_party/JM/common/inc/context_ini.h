
/*!
 *************************************************************************************
 * \file context_ini.h
 *
 * \brief
 *    CABAC context initializations
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Detlev Marpe
 *    - Heiko Schwarz
 **************************************************************************************
 */

#ifndef _CONTEXT_INI_
#define _CONTEXT_INI_


#define LL_SUBBLOCK_LOG2    (4)
#define REM_PREFIX_LENGTH 3
#define NUM_MB_PRED_CTX  16
#define NUM_MV_RES_CTX   10
#define NUM_IPR_CTX    32
#define NUM_BLOCK_TYPES 22 
#define NUM_BCBP_CTX   4
#define NUM_MAP_CTX   15
#define NUM_LAST_CTX  15
#define NUM_ONE_CTX   8


enum MODEL_INDXES {
    ZFLAG_ROW_MODEL_MODE_band0 = 0,
    ZFLAG_ROW_MODEL_MODE_band1,
    ZFLAG_ROW_MODEL_MODE_band2,
    ZFLAG_ROW_MODEL_TS_band0,
    ZFLAG_ROW_MODEL_TS_band1,
    ZFLAG_ROW_MODEL_TS_band2,
    ZFLAG_ROW_MODEL_2x2_band0,
    ZFLAG_ROW_MODEL_2x2_band1,
    ZFLAG_ROW_MODEL_2x2_band2,

    ZFLAG_ROW_MODEL_ACDC_1_band0_Y,
    ZFLAG_ROW_MODEL_ACDC_1_band0_U,
    ZFLAG_ROW_MODEL_ACDC_1_band0_V,
    ZFLAG_ROW_MODEL_ACDC_1_band1_Y,
    ZFLAG_ROW_MODEL_ACDC_1_band1_U,
    ZFLAG_ROW_MODEL_ACDC_1_band1_V,
    ZFLAG_ROW_MODEL_ACDC_1_band2_Y,
    ZFLAG_ROW_MODEL_ACDC_1_band2_U,
    ZFLAG_ROW_MODEL_ACDC_1_band2_V,

    ZFLAG_ROW_MODEL_ACDC_2_band0_Y,
    ZFLAG_ROW_MODEL_ACDC_2_band0_U,
    ZFLAG_ROW_MODEL_ACDC_2_band0_V,
    ZFLAG_ROW_MODEL_ACDC_2_band1_Y,
    ZFLAG_ROW_MODEL_ACDC_2_band1_U,
    ZFLAG_ROW_MODEL_ACDC_2_band1_V,
    ZFLAG_ROW_MODEL_ACDC_2_band2_Y,
    ZFLAG_ROW_MODEL_ACDC_2_band2_U,
    ZFLAG_ROW_MODEL_ACDC_2_band2_V,

    ZFLAG_ROW_MODEL_ACDC_3_band0_Y,
    ZFLAG_ROW_MODEL_ACDC_3_band0_U,
    ZFLAG_ROW_MODEL_ACDC_3_band0_V,
    ZFLAG_ROW_MODEL_ACDC_3_band1_Y,
    ZFLAG_ROW_MODEL_ACDC_3_band1_U,
    ZFLAG_ROW_MODEL_ACDC_3_band1_V,
    ZFLAG_ROW_MODEL_ACDC_3_band2_Y,
    ZFLAG_ROW_MODEL_ACDC_3_band2_U,
    ZFLAG_ROW_MODEL_ACDC_3_band2_V,

    ZFLAG_ROW_MODEL_ACDC_4_band0_Y,
    ZFLAG_ROW_MODEL_ACDC_4_band0_U,
    ZFLAG_ROW_MODEL_ACDC_4_band0_V,
    ZFLAG_ROW_MODEL_ACDC_4_band1_Y,
    ZFLAG_ROW_MODEL_ACDC_4_band1_U,
    ZFLAG_ROW_MODEL_ACDC_4_band1_V,
    ZFLAG_ROW_MODEL_ACDC_4_band2_Y,
    ZFLAG_ROW_MODEL_ACDC_4_band2_U,
    ZFLAG_ROW_MODEL_ACDC_4_band2_V,

    ZFLAG_BLOCK_MODEL_8x8nnz_band0_Y,
    ZFLAG_BLOCK_MODEL_8x8nnz_band0_U,
    ZFLAG_BLOCK_MODEL_8x8nnz_band0_V,
    ZFLAG_BLOCK_MODEL_8x8nnz_band1_Y,
    ZFLAG_BLOCK_MODEL_8x8nnz_band1_U,
    ZFLAG_BLOCK_MODEL_8x8nnz_band1_V,
    ZFLAG_BLOCK_MODEL_8x8nnz_band2_Y,
    ZFLAG_BLOCK_MODEL_8x8nnz_band2_U,
    ZFLAG_BLOCK_MODEL_8x8nnz_band2_V,

    ZFLAG_BLOCK_MODEL_8x8_allone_band0_Y,
    ZFLAG_BLOCK_MODEL_8x8_allone_band0_U,
    ZFLAG_BLOCK_MODEL_8x8_allone_band0_V,
    ZFLAG_BLOCK_MODEL_8x8_allone_band1_Y,
    ZFLAG_BLOCK_MODEL_8x8_allone_band1_U,
    ZFLAG_BLOCK_MODEL_8x8_allone_band1_V,
    ZFLAG_BLOCK_MODEL_8x8_allone_band2_Y,
    ZFLAG_BLOCK_MODEL_8x8_allone_band2_U,
    ZFLAG_BLOCK_MODEL_8x8_allone_band2_V,

    PATTERN0001_MODEL_band0,
    PATTERN0001_MODEL_band1,
    PATTERN0001_MODEL_band2,
    STRIP_ZERO_FLAG_MODEL,
    ZFLAG_MODEL_4x4_band0,
    ZFLAG_MODEL_4x4_band1,
    ZFLAG_MODEL_4x4_band2,
    TABLE_IDX_4x4_band0,
    TABLE_IDX_4x4_band1,
    TABLE_IDX_4x4_band2,

    MAX_COEF_GRT1_band0,
    MAX_COEF_GRT1_band1,
    MAX_COEF_GRT1_band2,
    NUM_MODELS_Z_FLAGS,
};

typedef struct bi_context_type BiContextType;
typedef BiContextType *BiContextTypePtr;

struct bi_context_type
{
    unsigned long  count;
    unsigned char state; // index into state-table CP
    unsigned char  MPS;  //Least Probable Symbol 0/1 CP  
};

typedef struct
{
    BiContextType mb_pred_context[NUM_MB_PRED_CTX];
    BiContextType mv_res_contexts[2][NUM_MV_RES_CTX];
} MotionInfoContexts;

typedef struct
{
    BiContextType  ipr_contexts[NUM_IPR_CTX];
    BiContextType  trans_contexts[NUM_IPR_CTX];
    BiContextType  bcbp_contexts[NUM_BLOCK_TYPES][NUM_BCBP_CTX];
    BiContextType  one_contexts[NUM_BLOCK_TYPES][NUM_ONE_CTX];
    BiContextType  map_contexts[4][NUM_BLOCK_TYPES][NUM_MAP_CTX];
    BiContextType  last_contexts[4][NUM_BLOCK_TYPES][NUM_LAST_CTX];
} TextureInfoContexts;

typedef struct
{
    BiContextType  zFlag[NUM_MODELS_Z_FLAGS];
} HighBandInfoContexts;

void InitContextsLl(int qp, TextureInfoContexts *texCtx, MotionInfoContexts *motCtx);
void InitContextsHf(int qp, HighBandInfoContexts *highBandCtx);
extern void biari_init_context(int qp, BiContextTypePtr ctx, const char* ini);
#endif

