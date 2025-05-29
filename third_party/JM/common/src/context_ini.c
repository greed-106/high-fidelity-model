
#include "context_ini.h"

void InitContexts(BiContextTypePtr ctx)
{
    ctx->MPS = 0;
    ctx->prob_lps = LBAC_HALF_PROB;
}

#define BIARI_CTX_INIT2(ii,jj,ctx) \
{ \
  for (i=0; i<ii; i++) \
  for (j=0; j<jj; j++) \
  { \
    InitContexts(&(ctx[i][j])); \
  } \
}

static inline void binary_context_init1(int jj, BiContextType *ctx)
{
	for (int j = 0; j < jj; j++) {
        InitContexts(&(ctx[j]));
	}
}

void InitContextsLl(TextureInfoContexts *texCtx, MotionInfoContexts *motCtx)
{
	TextureInfoContexts* tc = texCtx;
	int model_number = 0;
	int i, j;

	//--- motion coding contexts ---
	MotionInfoContexts*  mc = motCtx;
	BIARI_CTX_INIT2(2, NUM_MV_RES_CTX, mc->mv_res_contexts);
	binary_context_init1(NUM_MB_PRED_CTX, mc->mb_pred_context);

	//--- texture coding contexts ---
	binary_context_init1(NUM_IPR_CTX, tc->ipr_contexts);   //pre and transform mode
	binary_context_init1(NUM_IPR_CTX, tc->trans_contexts);   //pre and transform mode
	BIARI_CTX_INIT2(NUM_BLOCK_TYPES, NUM_BCBP_CTX, tc->bcbp_contexts);
	BIARI_CTX_INIT2(NUM_BLOCK_TYPES, NUM_MAP_CTX, tc->map_contexts[0]);
	BIARI_CTX_INIT2(NUM_BLOCK_TYPES, NUM_LAST_CTX, tc->last_contexts[0]);
	BIARI_CTX_INIT2(NUM_BLOCK_TYPES, NUM_ONE_CTX, tc->one_contexts);
}

void InitContextsHf(HighBandInfoContexts *highBandCtx)
{
    char defaulTable[2] = { 0, 64 };
    HighBandInfoContexts*  hb = highBandCtx;
    for (int i = 0; i < NUM_MODELS_Z_FLAGS; i++) {
        InitContexts(&(hb->zFlag[i]));
    }
}
