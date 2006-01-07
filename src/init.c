#include <R.h>
#include <Rinternals.h>

#include "cluster.h"

#include <R_ext/Rdynload.h>

static R_NativePrimitiveArgType clara_t[32] = {
    /*n:*/ INTSXP, INTSXP, INTSXP, REALSXP, INTSXP, INTSXP, REALSXP, INTSXP,
    /*valmd:*/ REALSXP, INTSXP, INTSXP, /* rng_R: */ LGLSXP,
    /*nrepr: */ INTSXP, INTSXP, INTSXP, INTSXP, INTSXP,
    /*radus:*/ REALSXP, REALSXP, REALSXP, REALSXP, REALSXP, REALSXP, INTSXP,
    /*obj: */ REALSXP, REALSXP, REALSXP, REALSXP,  INTSXP, INTSXP,
    /*tmp: */ REALSXP,INTSXP
};

static R_NativePrimitiveArgType pam_t[23] = {
    INTSXP, INTSXP, INTSXP, REALSXP, REALSXP,
    /*jdyss: */ INTSXP, REALSXP, INTSXP,  INTSXP, INTSXP,
    /*nrepr: */ LGLSXP, INTSXP, /*radus: */ REALSXP, REALSXP, REALSXP, REALSXP,
    /*ttsyl: */ REALSXP, REALSXP, INTSXP, INTSXP,  REALSXP, REALSXP, INTSXP
};

static R_NativePrimitiveArgType spannel_t[12] = {
    INTSXP, INTSXP, REALSXP, REALSXP, REALSXP, REALSXP,
    /*varss: */ REALSXP, REALSXP, REALSXP, REALSXP, INTSXP, INTSXP
};

/* is only .C()-called from ../tests/sweep-ex.R : */
static R_NativePrimitiveArgType sweep_t[5] = {
    REALSXP, INTSXP, INTSXP, INTSXP, REALSXP
};

static const R_CMethodDef CEntries[]  = {
    {"clara", (DL_FUNC) &clara, 32, clara_t},
    {"pam", (DL_FUNC) &pam, 23, pam_t},
    {"spannel", (DL_FUNC) &spannel, 12, spannel_t},
    {"sweep", (DL_FUNC) &sweep, 5, sweep_t},
    {NULL, NULL, 0}
};

/* static R_CallMethodDef CallEntries[] = {
 *     {NULL, NULL, 0}
 * };
 */

static R_FortranMethodDef FortEntries[] = {
    {"bncoef", (DL_FUNC) &F77_SUB(bncoef), 3},/* ./twins.f */
    {"daisy", (DL_FUNC) &F77_SUB(daisy), 10},
    {"fanny", (DL_FUNC) &F77_SUB(fanny), 29},
    {"mona", (DL_FUNC) &F77_SUB(mona), 9},
    {"twins", (DL_FUNC) &F77_SUB(twins), 17},
    {"dysta", (DL_FUNC) &F77_SUB(dysta), 8},
    {"dysta3", (DL_FUNC) &F77_SUB(dysta3), 8},/* ./fanny.f */
    {NULL, NULL, 0}
};

void R_init_cluster(DllInfo *dll)
{
    R_registerRoutines(dll, CEntries, NULL/*CallEntries*/, FortEntries, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
