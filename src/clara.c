/*   Clustering LARge Applications
     ~		~~~   ~
     Clustering program based upon the k-medoid approach,
     and suitable for data sets of at least 100 objects.
     (for smaller data sets, please use program pam.)
 */

/* $Id$
 * original Id: clara.f,v 1.10 2002/08/27 15:43:58 maechler translated by
 * f2c (version 20010821) and run through f2c-clean,v 1.10 2002/03/28
 */

#include <math.h>

#include <R_ext/Print.h>/* for diagnostics */

#include "cluster.h"

void clara(int *n,  /* = number of objects */
	   int *jpp,/* = number of variables */
	   int *kk, /* = number of clusters, 1 <= kk <= n-1 */
	   double *x,	/* Input:  the data x[n, jpp] _rowwise_ (transposed)
			 * Output: the first `n' values are the `clustering'
			 *	   (integers in 1,2,..,kk) */
	   int *nran,	/* = #{random samples} drawn	   (= `samples' in R)*/
	   int *nsam,	/* = #{objects} drawn from data set (`sampsize' in R) */
	   double *dys,/* [1:(1 + (nsam * (nsam - 1))/2)]
			* Output: to contain the distances */
	   int *mdata,	/*= {0,1}; 1: min(x) is missing value (NA);  0: no NA */
	   double *valmd,/*[j]= missing value code (instead of NA) for x[,j]*/
	   int *jtmd,	/* [j]= {-1,1};	 -1: x[,j] has NA; 1: no NAs in x[,j] */
	   int *ndyst,	/* = {1,2};  1 : euclidean;  2 : manhattan*/
	   int *nrepr,	/* */
	   int *nsel, /* x[nsel[j]]  will be the j-th obs in the final sample */
	   int *nbest,
	   int *nr, int *nrx,
	   double *radus, double *ttd, double *ratt,
	   double *ttbes, double *rdbes, double *rabes,
	   int *mtt, double *obj,
	   double *avsyl, double *ttsyl, double *sylinf, int *jstop,
	   double *tmp, /* = double [ 3 * nsam ] */
	   int *itmp	/* = integer[ 6 * nsam ] */
    )
{

#define tmp1 tmp
#define tmp2 &tmp[*nsam]

#define ntmp1 itmp
#define ntmp2 &itmp[*nsam]
#define ntmp3 &itmp[nsamb]
#define ntmp4 &itmp[nsamb+ *nsam]
#define ntmp5 &itmp[2*nsamb]
#define ntmp6 &itmp[2*nsamb+ *nsam]


    /* Local variables */

    Rboolean nafs, kall, full_sample;
    int j, jk, jkk, js, jsm, jhalt;
    int kkm, kkp, nsm, ntt;
    int nadv, jran, kran, kans, less, nsub, nrun, l;
    int n_dys, nsamb, nad, nadvp, nexap, nexbp, nunfs;
    double ran, rnn, sky, zb, zba = -1., s = -1., sx = -1.;/* Wall */

    /* Parameter adjustments */
    --nsel;


    *jstop = 0;
    rnn = (double) (*n);
    /* n_dys := size of distance array dys[] */
    n_dys = *nsam * (*nsam - 1) / 2 + 1;/* >= 1 */

    full_sample = (*n == *nsam);/* only one sub sample == full data */

    nsamb = *nsam << 1;
    if (*n < nsamb)/* sample more than half */
	less = *n - *nsam;
    else
	less = *nsam;
    nunfs = 0;
    kall = FALSE;
    /* nrun : this is the ``random seed'' of the very simple randm() below */
    nrun = 0;

/* __LOOP__ :  random subsamples are drawn and partitioned into kk clusters */

    for (jran = 1; jran <= *nran; ++jran) {
	jhalt = 0;
	if (!full_sample) {/* `real' case: sample size < n */
	    ntt = 0;
	    if (jran != 1 && nunfs != jran && *n >= nsamb) {
		for (jk = 0; jk < *kk; ++jk)
		    nsel[jk+1] = nrx[jk];
		kkm = *kk - 1;
		for (jk = 1; jk <= kkm; ++jk) {
		    nsm = nsel[jk];
		    kkp = jk + 1;
		    jsm = jk;
		    for (jkk = kkp; jkk <= *kk; ++jkk) {
			if (nsel[jkk] < nsm) {
			    nsm = nsel[jkk];
			    jsm = jkk;
			}
		    }
		    nsel[jsm] = nsel[jk];
		    nsel[jk] = nsm;
		}
		ntt = *kk;
	    }
	    else {

		/* Loop 1 -- */
	    L180:
		randm(&nrun, &ran);
		kran = (int) (rnn * ran + 1.);
		if (kran > *n) {
		    kran = *n;
		}
		if (jran != 1) {
		    for (jk = 0; jk < *kk; ++jk) {
			if (kran == nrx[jk])
			    goto L180;
		    }
		}
		/* end Loop 1*/
		++ntt;
		nsel[ntt] = kran;
		if (less == ntt)
		    goto L290;
	    }

	    do {
	    L210:
		randm(&nrun, &ran);
		kran = (int) (rnn * ran + 1.);
		if (kran > *n) {
		    kran = *n;
		}
		if (jran != 1 && *n < nsamb) {
		    for (jk = 0; jk < *kk; ++jk) {
			if (kran == nrx[jk])
			    goto L210;
		    }
		}

		for (kans = 1; kans <= ntt; ++kans) {
		    if (nsel[kans] >= kran) {
			if (nsel[kans] == kran)
			    goto L210;
			else {
			    for (nad = kans; nad <= ntt; ++nad) {
				nadv = ntt - nad + kans;
				nadvp = nadv + 1;
				nsel[nadvp] = nsel[nadv];
			    }
			    ++ntt;
			    nsel[kans] = kran;
			    goto L290;
			}
		    }
		}
		++ntt;
		nsel[ntt] = kran;
	    L290:
		;
	    } while (ntt < less);

	    if (*n < nsamb) {
		for (j = 1, nexap = 1, nexbp = 0; j < *n; j++) {
		    if (nsel[nexap] == j)
			++nexap;
		    else
			nrepr[nexbp++] = j;
		}
		for (nsub = 0; nsub < *nsam; ++nsub)
		    nsel[nsub+1] = nrepr[nsub];
	    }
	}
	else { /* full_sample : *n = *nsam -- one sample is enough ! */
	    for (j = 1; j <= *nsam; ++j)
		nsel[j] = j;
	}

	dysta2(*nsam, *jpp, &nsel[1], x, *n, dys, *ndyst, jtmd, valmd, &jhalt);
	if (jhalt == 1)
	    continue;/* random sample*/

	s = 0.;
	l = 0;/* dys[0] is not used */
	do {
	    ++l;
	    if (s < dys[l])
		s = dys[l];
	} while (l+1 < n_dys);

	kall = TRUE;

	bswap2(*kk, *nsam, nrepr, dys, &sky, s,
	       /* dysma */tmp1, /*dysmb*/tmp2,
	       /* beter[], only used here */&tmp[nsamb]);

	selec(*kk, *n, *jpp, *ndyst, &zb, *nsam, *mdata, jtmd, valmd,
	      nrepr, &nsel[1], dys, x, nr, &nafs, ttd, radus, ratt,
	      ntmp1, ntmp2, ntmp3, ntmp4, ntmp5, ntmp6, tmp1, tmp2);

	if (nafs) {
	    ++nunfs;
	}
	else if (jran == 1 || zba > zb) {/* 1st time, or new best */

	    zba = zb;
	    for (jk = 0; jk < *kk; ++jk) {
		ttbes[jk] = ttd	 [jk];
		rdbes[jk] = radus[jk];
		rabes[jk] = ratt [jk];
		nrx  [jk] = nr	 [jk];
	    }
	    for (js = 0; js < *nsam; ++js)
		nbest[js] = nsel[js+1];
	    sx = s;
	}
	if(full_sample) break; /* out of resampling */
    }
/* --- end random sampling loop */

    if (nunfs >= *nran) { *jstop = 1; return; }

/*     for the best subsample, the objects of the entire data set
     are assigned to their clusters */

    if (!kall) { *jstop = 2; return; }

    *obj = zba / rnn;
    dysta2(*nsam, *jpp, nbest, x, *n, dys, *ndyst, jtmd, valmd, &jhalt);

    resul(*kk, *n, *jpp, *ndyst, *mdata, jtmd, valmd, x, nrx, mtt);

    if (*kk > 1)
	black(*kk, *jpp, *nsam, nbest, dys, sx, x,
	      /* compute --> */
	      avsyl, ttsyl, sylinf,
	      ntmp1, ntmp2, ntmp3, ntmp4, tmp1, tmp2);
    return;
} /* End clara() ---------------------------------------------------*/
#undef tmp1
#undef tmp2

#undef ntmp1
#undef ntmp2
#undef ntmp3
#undef ntmp4
#undef ntmp5
#undef ntmp6


void dysta2(int nsam, int jpp, int *nsel,
	    double *x, int n, double *dys, int ndyst,
	    int *jtmd, double *valmd, int *jhalt)
{

    /* Local variables */
    int j, k, kj, l, lj, ksel, lsel, nlk, npres;
    double clk, d1;

    nlk = 0;
    dys[nlk] = 0.;
    for (l = 1; l < nsam; ++l) {
	lsel = nsel[l];
	for (k = 0; k < l; ++k) {
	    ksel = nsel[k];
	    clk = 0.;
	    ++nlk;
	    npres = 0;
	    for (j = 0; j < jpp; ++j) {
		lj = (lsel - 1) * jpp + j;
		kj = (ksel - 1) * jpp + j;
		if (jtmd[j] < 0) {
/* in the following line, x[-2] ==> seg.fault {BDR to R-core, Sat, 3 Aug 2002} */
		    if (x[lj] == valmd[j]) {
			continue /* next j */;
		    }
		    if (x[kj] == valmd[j]) {
			continue /* next j */;
		    }
		}
		++npres;
		if (ndyst == 1)
		    clk += (x[lj] - x[kj]) * (x[lj] - x[kj]);
		else
		    clk += fabs(x[lj] - x[kj]);
	    }
	    if (npres == 0) {
		*jhalt = 1;
		dys[nlk] = -1.;
	    } else {
		d1 = clk * (((double) (jpp)) / (double) npres);
		dys[nlk] = (ndyst == 1) ? sqrt(d1) : d1 ;
	    }
	}
    }
    return;
} /* End dysta2() -----------------------------------------------------------*/

void randm(int *nrun, double *ran)
{
    int k;
    double ry;

/*   we programmed this generator ourselves because we wanted it
   to be machine independent. it should run on most computers
   because the largest int used is less than 2**30 . the period
   is 2**16=65536, which is good enough for our purposes. */
    *nrun = *nrun * 5761 + 999;
    k = *nrun / 65536;
    *nrun -= k << 16;
    ry = (double) (*nrun);
    *ran = ry / 65536.f;
    return;
} /* randm() */

/* bswap2() : called once [per random sample] from clara() : */
void bswap2(int kk, int nsam, int *nrepr,
	    double *dys, double *sky, double s,
	    double *dysma, double *dysmb, double *beter)
{
    int j, ja, k, kbest = -1, nbest = -1;/* init for -Wall */
    int nkj, njn, njaj, nny, nmax;

    double ammax, small, asky, dzsky, dz, cmd;

    /* Parameter adjustments */
    --nrepr;
    --beter;

    --dys;/*index via meet_() */
    --dysma;	--dysmb;


/* ====== first algorithm: BUILD. ====== */

    for (j = 1; j <= nsam; ++j) {
	nrepr[j] = 0;
	dysma[j] = s * 1.1 + 1.;
    }

    for(nny = 0; nny < kk; nny++) {
	for (ja = 1; ja <= nsam; ++ja) {
	    if (nrepr[ja] == 0) {
		beter[ja] = 0.;
		for (j = 1; j <= nsam; ++j) {
		    njaj = meet_(&ja, &j);
		    cmd = dysma[j] - dys[njaj];
		    if (cmd > 0.)
			beter[ja] += cmd;
		}
	    }
	}
	ammax = 0.;
	for (ja = 1; ja <= nsam; ++ja) {
	    if (nrepr[ja] == 0 && ammax <= beter[ja]) {
		ammax = beter[ja];
		nmax = ja;
	    }
	}
	nrepr[nmax] = 1;
	for (j = 1; j <= nsam; ++j) {
	    njn = meet_(&nmax, &j);
	    if (dysma[j] > dys[njn])
		dysma[j] = dys[njn];
	}
    }

    *sky = 0.;
    for (j = 1; j <= nsam; ++j)
	*sky += dysma[j];

    if (kk == 1)
	return;

    asky = *sky / ((double) nsam);

/* ====== second algorithm: SWAP. ====== */

/* Big LOOP : */
L60:

    for (j = 1; j <= nsam; ++j) {
	dysma[j] = s * 1.1 + 1.;
	dysmb[j] = s * 1.1 + 1.;
	for (ja = 1; ja <= nsam; ++ja) {
	    if (nrepr[ja] != 0) {
		njaj = meet_(&ja, &j);
		if (dys[njaj] < dysma[j]) {
		    dysmb[j] = dysma[j];
		    dysma[j] = dys[njaj];
		} else if (dys[njaj] < dysmb[j])
		    dysmb[j] = dys[njaj];
	    }
	}
    }

    dzsky = 1.;
    for (k = 1; k <= nsam; ++k) {
	if (nrepr[k] != 1) {
	    for (ja = 1; ja <= nsam; ++ja) {
		if (nrepr[ja] != 0) {
		    dz = 0.;
		    for (j = 1; j <= nsam; ++j) {
			njaj = meet_(&ja, &j);
			nkj  = meet_(&k, &j);
			if (dys[njaj] == dysma[j]) {
			    small = dysmb[j];
			    if (small > dys[njaj])
				small = dys[nkj];
			    dz = dz - dysma[j] + small;
			}
			else if (dys[nkj] < dysma[j])
			    dz = dz - dysma[j] + dys[nkj];
		    }
		    if (dz < dzsky) {
			dzsky = dz;
			kbest = k;
			nbest = ja;
		    }
		}
	    }
	}
    }
    if (dzsky >= 0.)
	return;

    nrepr[kbest] = 1;
    nrepr[nbest] = 0;
    *sky += dzsky;
    goto L60;

} /* End of bswap2() -------------------------------------------------- */

/* selec() : called once [per random sample] from clara() */
void selec(int kk, int n, int jpp, int ndyst,
	   double *zb, int nsam, int mdata, int *jtmd, double *valmd,
	   int *nrepr, int *nsel, double *dys, double *x, int *nr,
	   Rboolean *nafs, /* := TRUE if a distance cannot be calculated */
	   double *ttd, double *radus, double *ratt,
	   int *nrnew, int *nsnew, int *npnew, int *ns, int *np, int *new,
	   double *ttnew, double *rdnew)
{

    /* Local variables */
    int j, jk, jj, jp, jnew, ka, kb, jkabc = -1/* -Wall */;
    int newf, nrjk,  npab, nstrt, na, nb, npa, npb, njk, nobs;

    double dsum, pp, tra, rns, dnull = -9./* -Wall */;

/* Parameter adjustments */
    --nsel;    --nrepr;

    --ratt;
    --radus; --ttd;    --np;	--nr;	 --ns;

    --rdnew; --ttnew; --npnew; --nrnew; --nsnew;
    --new;

    --dys;

    /* Function Body */
    *nafs = FALSE;

/* identification of representative objects, and initializations */

    jk = 0;
    for (j = 1; j <= nsam; ++j) {
	if (nrepr[j] != 0) {
	    ++jk;
	    nr	 [jk] = nsel[j];
	    ns	 [jk] = 0;
	    ttd	 [jk] = 0.;
	    radus[jk] = -1.;
	    np	 [jk] = j;
	}
    }

/* - assignment of the objects of the entire data set to a cluster,
 * - computation of some statistics,
 * - determination of the new ordering of the clusters */

    *zb = 0.;
    pp = (double) (jpp);
    newf = 0;

    for(jj = 1; jj <= n; jj++) {
	if (!mdata) {
	    for (jk = 1; jk <= kk; ++jk) {
		dsum = 0.;
		nrjk = nr[jk];
		if (nrjk != jj) {
		    for (jp = 0; jp < jpp; ++jp) {
			na = (nrjk - 1) * jpp + jp;
			nb = (jj   - 1) * jpp + jp;
			tra = fabs(x[na] - x[nb]);
			if (ndyst == 1)
			    tra *= tra;
			dsum += tra;
		    }
		    if (jk != 1 && dsum >= dnull)
			continue /* next jk */;
		}
		dnull = dsum;
		jkabc = jk;
	    }
	}
	else { /* _has_ missing data */
	    Rboolean pres;
	    pres = FALSE;
	    for (jk = 1; jk <= kk; ++jk) {
		dsum = 0.;
		nrjk = nr[jk];
		if (nrjk != jj) {
		    nobs = 0;
		    for (jp = 0; jp < jpp; ++jp) {
			na = (nrjk - 1) * jpp + jp;
			nb = (jj   - 1) * jpp + jp;
			if (jtmd[jp] < 0) {
			    if (x[na] == valmd[jp] || x[nb] == valmd[jp])
				continue /* next jp */;
			}
			nobs++;
			tra = fabs(x[na] - x[nb]);
			if (ndyst == 1)
			    tra *= tra;
			dsum += tra;
		    }
		    if (nobs == 0) /* all pairs partially missing */
			continue /* next jk */;
		    dsum *= (nobs / pp);
		}
		if (!pres)
		    pres = TRUE;
		else if (dnull <= dsum)
		    continue /* next jk */;
		/* here : pres was FALSE {i.e. 1st time} or
		 *	  dnull > dsum	 {i.e. new best} */
		dnull = dsum;
		jkabc = jk;
	    }/* for(jk ..) */

	    if (!pres) { /* found nothing */
		*nafs = TRUE; return;
	    }
	} /* if (mdata..) else */

	if (ndyst == 1)
	    dnull = sqrt(dnull);

	*zb += dnull;
	ttd[jkabc] += dnull;
	if (radus[jkabc] < dnull)
	    radus[jkabc] = dnull;

	++ns[jkabc];
	if (newf < kk) {
	    if (newf != 0) {
		for (jnew = 1; jnew <= newf; ++jnew) {
		    if (jkabc == new[jnew])
			goto L90;/* next jj */
		}
	    }
	    ++newf;
	    new[newf] = jkabc;
	}
    L90:
	;
    } /* for( jj = 1..n ) */


/*     a permutation is carried out on vectors nr,ns,np,ttd,radus
     using the information in vector new. */

    for (jk = 1; jk <= kk; ++jk) {
	njk = new[jk];
	nrnew[jk] = nr[njk];
	nsnew[jk] = ns[njk];
	npnew[jk] = np[njk];
	ttnew[jk] = ttd[njk];
	rdnew[jk] = radus[njk];
    }
    for (jk = 1; jk <= kk; ++jk) {
	nr[jk] = nrnew[jk];
	ns[jk] = nsnew[jk];
	np[jk] = npnew[jk];
	ttd[jk] = ttnew[jk];
	radus[jk] = rdnew[jk];
    }
    for (j = 1; j <= kk; ++j) {
	rns = (double) ns[j];
	ttd[j] /= rns;
    }

    if (kk > 1) {

	/* computation of minimal distance of medoid ka to any
	   other medoid for comparison with the radius of cluster ka. */

	for (ka = 1; ka <= kk; ++ka) {
	    nstrt = 0;
	    npa = np[ka];
	    for (kb = 1; kb <= kk; ++kb) {
		if (kb == ka)
		    continue /* next kb */;

		npb = np[kb];
		npab = meet_(&npa, &npb);
		if (nstrt == 0)
		    nstrt = 1;
		else if (dys[npab] >= ratt[ka])
		    continue /* next kb */;

		ratt[ka] = dys[npab];
		if (ratt[ka] != 0.)
		    continue /* next kb */;

		ratt[ka] = -1.;
	    }
	    if (ratt[ka] > -0.5)
		ratt[ka] = radus[ka] / ratt[ka];
	}
    }
    return;
} /* End selec() -----------------------------------------------------------*/

void resul(int kk, int n, int jpp, int ndyst, int mdata,
	   int *jtmd, double *valmd, double *x, int *nrx, int *mtt)
{
    /* Local variables */
    int j, jk, jj, ka, na, nb, njnb, nrjk, jksky = -1/* Wall */;
    double pp, abc, dsum, tra, dnull = -9./* Wall */;

/* clustering vector is incorporated into x, and ``printed''. */

    pp = (double) (jpp);

    for(jj = 0; jj < n; jj++) {

	for (jk = 0; jk < kk; ++jk) {
	    if (nrx[jk] == jj + 1)/* 1-indexing */
		goto L220; /* continue next jj (i.e., outer loop) */
	}
	njnb = jj * jpp;

	if (!mdata) {
	    for (jk = 0; jk < kk; ++jk) {
		dsum = 0.;
		nrjk = (nrx[jk] - 1) * jpp;
		for (j = 0; j < jpp; ++j) {
		    na = nrjk + j;
		    nb = njnb + j;
		    tra = fabs(x[na] - x[nb]);
		    if (ndyst == 1)
			tra *= tra;
		    dsum += tra;
		}
		if (ndyst == 1)
		    dsum = sqrt(dsum);
		if (jk == 0)
		    dnull = dsum + .1f;
		if (dnull > dsum) {
		    dnull = dsum;
		    jksky = jk;
		}
	    }
	}
	else { /* _has_ missing data */
	    for (jk = 0; jk < kk; ++jk) {
		dsum = 0.;
		nrjk = (nrx[jk] - 1) * jpp;
		abc = 0.;
		for (j = 0; j < jpp; ++j) {
		    na = nrjk + j;
		    nb = njnb + j;
		    if (jtmd[j] < 0) {
			if (x[na] == valmd[j] || x[nb] == valmd[j])
			    continue /* next j */;
		    }
		    abc += 1.;
		    tra = fabs(x[na] - x[nb]);
		    if (ndyst == 1)
			tra *= tra;
		    dsum += tra;
		}
		if (ndyst == 1)
		    dsum = sqrt(dsum);
		dsum *= (abc / pp);
		if (jk == 0)
		    dnull = dsum + .1f;

		if (dnull > dsum) {
		    dnull = dsum;
		    jksky = jk;
		}
	    }
	}
	x[njnb] = (double) jksky + 1;/* 1-indexing */

    L220:
	;
    } /* for(jj)  while (jj < n);*/

    for (jk = 0; jk < kk; ++jk) {
	nrjk = nrx[jk];
	x[(nrjk - 1) * jpp] = (double) jk + 1;/* 1-indexing */
    }
    for (ka = 0; ka < kk; ++ka) {
	mtt[ka] = 0;
	for(j = 0; j < n; j++) {
	    if (((int) (x[j * jpp] + .1f)) == ka + 1)/* 1-indexing */
		++mtt[ka];
	}
    }
    return;
} /* end resul() -----------------------------------------------------------*/


void black(int kk, int jpp, int nsam, int *nbest,
	   double *dys, double s, double *x,
	   /* --> Output : */
	   double *avsyl, double *ttsyl, double *sylinf,
	   int *ncluv, int *nsend, int *nelem, int *negbr,
	   double *syl, double *srank)
{
/* Silhouettes computation and "drawing"  --> syl[] and sylinf[] */

    /* System generated locals */
    int sylinf_dim1, sylinf_offset;

    /* Local variables */

    double att, btt, db, dysa, dysb, symax;
    int lang = -1/* -Wall */;
    int j, l, lplac, nj, nl, nbb, ncase, nclu, numcl, nsylr, ntt;

/* Parameter adjustments */
    --avsyl;

    --srank;
    --syl;
    --negbr;
    --nelem;
    --nsend;
    --ncluv;	--nbest;
    --dys;

    sylinf_dim1 = nsam;
    sylinf_offset = 1 + sylinf_dim1 * 1;
    sylinf -= sylinf_offset;

/*
     construction of clustering vector (ncluv)
     of selected sample (nbest).
*/

    /* Function Body */
    for (l = 1; l <= nsam; ++l) {
	ncase = nbest[l];
	ncluv[l] = (int) (x[(ncase - 1) * jpp] + .1f);
    }

/*     drawing of the silhouettes */

    nsylr = 0;
    *ttsyl = 0.;
    for (numcl = 1; numcl <= kk; ++numcl) {
	ntt = 0;
	for (j = 1; j <= nsam; ++j) {
	    if (ncluv[j] == numcl) {
		++ntt;
		nelem[ntt] = j;
	    }
	}
	for (j = 1; j <= ntt; ++j) {
	    nj = nelem[j];
	    dysb = s * 1.1 + 1.;
	    negbr[j] = -1;

	    for (nclu = 1; nclu <= kk; ++nclu) {
		if (nclu != numcl) {
		    nbb = 0;
		    db = 0.;
		    for (l = 1; l <= nsam; ++l) {
			if (ncluv[l] == nclu) {
			    ++nbb;
			    db += dys[meet_(&nj, &l)];
			}
		    }
		    btt = (double) nbb;
		    db /= btt;
		    if (db < dysb) {
			dysb = db;
			negbr[j] = nclu;
		    }
		}
	    }

	    if (ntt == 1) {
		syl[j] = 0.;	    continue /* j */;
	    }
	    dysa = 0.;
	    for (l = 1; l <= ntt; ++l) {
		nl = nelem[l];
		dysa += dys[meet_(&nj, &nl)];
	    }
	    att = (double) (ntt - 1);
	    dysa /= att;
	    if (dysa <= 0.) {
		if (dysb > 0.)
		    syl[j] = 1.;
		else
		    syl[j] = 0.;

		continue /* j */;
	    }

	    if (dysb > 0.) {
		if (dysb > dysa)
		    syl[j] = 1. - dysa / dysb;
		else if (dysb < dysa)
		    syl[j] = dysb / dysa - 1.;
		else /* (dysb == dysa) */
		    syl[j] = 0.;
	    }
	    else {
		syl[j] = -1.;
	    }

	    if (syl[j] < -1.)
		syl[j] = -1.;
	    else if (syl[j] > 1.)
		syl[j] = 1.;

	} /* for(j ..) */

	avsyl[numcl] = 0.;
	for (j = 1; j <= ntt; ++j) {
	    symax = -2.;
	    for (l = 1; l <= ntt; ++l) {
		if (syl[l] > symax) {
		    symax = syl[l];
		    lang = l;
		}
	    }
	    nsend[j] = lang;
	    srank[j] = syl[lang];
	    avsyl[numcl] += srank[j];
	    syl[lang] = -3.;
	}
	*ttsyl += avsyl[numcl];
	avsyl[numcl] /= ntt;

	if (ntt >= 2) {
	    for (l = 1; l <= ntt; ++l) {
		lplac = nsend[l];
		ncase = nelem[lplac];
		++nsylr;
		sylinf[nsylr + sylinf_dim1] = (double) numcl;
		sylinf[nsylr + (sylinf_dim1 << 1)] = (double) negbr[lplac];
		sylinf[nsylr + sylinf_dim1 * 3] = srank[l];
		sylinf[nsylr + (sylinf_dim1 << 2)] = (double) nbest[ncase];
	    }
	}
	else {
	    ncase = nelem[1];
	    ++nsylr;
	    sylinf[nsylr + sylinf_dim1] = (double) numcl;
	    sylinf[nsylr + (sylinf_dim1 << 1)] = (double) negbr[1];
	    sylinf[nsylr + sylinf_dim1 * 3] = 0.;
	    sylinf[nsylr + (sylinf_dim1 << 2)] = (double) nbest[ncase];
	}

    }
    *ttsyl /= (double) (nsam);
    return;
} /* black */
