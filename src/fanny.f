C-- $Id$
C   program for Fuzzy cluster ANalysis
C
      subroutine fanny(nn,jpp,kk,x,dss,jdyss,valmd,jtmd,ndyst,
     1	   nsend,nelem,negbr, syl,p,dp,pt,nfuzz,esp,ef,dvec,
     2	   ttsyl,eda,edb,obj,ncluv,sylinf,eps)
c Arguments
      integer nn,jpp,kk, jdyss, ndyst
C	  nn   = number of objects
C	  jpp  = number of variables for clustering
C	  kk   = number of clusters
      double precision ttsyl, eda,edb, eps
      double precision x(nn,jpp),p(nn,kk),dp(nn,kk)
      double precision dss(nn*(nn-1)/2)

C  dimension of nsend,negbr,nelem,ncluv,dvec,syl is maxnn:
      integer nsend(nn),nelem(nn),negbr(nn),ncluv(nn)
      double precision dvec(nn),syl(nn), sylinf(nn,4)
C  dim. valmd,jtmd,esp,ef,pt,nfuzz(maxkk):
      double precision valmd(jpp), obj(2)
      integer jtmd(jpp), nfuzz(kk)
      double precision pt(kk),esp(kk),ef(kk)
C
C VARs
      double precision s
      integer ktrue, l, jhalt, nhalf
C
      if(jdyss.ne.1) then
c	 compute dissimilarities from data
	 jhalt=0
	 call dysta3(nn,jpp,x,dss,ndyst,jtmd,valmd,jhalt)
	 if(jhalt.ne.0) then
	    jdyss=-1
	    return
	 endif
      endif
C
      s=0.0
      nhalf=nn*(nn-1)/2
      l=1
 130  l=l+1
      if(dss(l).gt.s)s=dss(l)
      if(l.lt.nhalf)go to 130
      call fuzzy(nn,nhalf,p,dp,pt,dss,esp,ef,eda,edb,kk,obj,eps)
      call caddy(nn,p,kk,ktrue,nfuzz,ncluv,pt,nelem)
      if(2.le.ktrue .and. ktrue.lt.nn) then
	 call fygur(ktrue,nn,kk,nhalf,ncluv,nsend,nelem,
     1	      negbr,syl,dvec,pt,ttsyl,dss,s,sylinf)
      endif
      end
C     --- fanny

C
      subroutine dysta3(nn,jpp,x,dys,ndyst,jtmd,valmd,jhalt)
c Args
      integer nn,jpp
      double precision x(nn,jpp), dys(nn*(nn-1)/2), valmd(jpp)
      integer ndyst, jtmd(jpp), jhalt
c VARs
      double precision pp, clk, rpres
      integer j,k,l, nlk, npres

      pp=jpp
      nlk=0
      do 100 l=1,nn-1
	 do 20 k=l+1,nn
	    clk=0.0
	    nlk=nlk+1
	    npres=0
	    do 30 j=1,jpp
	       if(jtmd(j).lt.0) then
		  if(x(l,j).eq.valmd(j))goto 30
		  if(x(k,j).eq.valmd(j))goto 30
	       endif
	       npres=npres+1
	       if(ndyst.eq.1) then
		  clk=clk+(x(l,j)-x(k,j))*(x(l,j)-x(k,j))
	       else
		  clk=clk+dabs(x(l,j)-x(k,j))
	       endif
 30	    continue
	    rpres=npres
	    if(npres.eq.0) then
	       jhalt=1
	       dys(nlk)=-1.0
	    else
	       if(ndyst.eq.1) then
		  dys(nlk)=dsqrt(clk*(pp/rpres))
	       else
		  dys(nlk)=clk*(pp/rpres)
	       endif
	    endif
 20	 continue
 100  continue
      end
C
C
      subroutine fuzzy(nn,hh,p,dp,pt,dss,esp,ef,eda,edb,k,obj,eps)

c     Args
      integer nn,hh, k
      double precision p(nn,k),dp(nn,k),pt(k), dss(hh)
      double precision esp(k),ef(k), eda,edb, obj(2), eps
C     eps is the precision for the iterations

C
c     VARs
      integer kaunt, j,j1,j2,l,lx,m, nd,ndk, nyt
      double precision ann,crt,cryt,dt,ddd, r, reen, rvers,rkme, xx, zk
C     r	  is the exponent, strictly larger than 1.0
C     nyt is the maximal number of iterations
C
      r=2.0
      rvers=1./r
      nyt=500
C
C     initial fuzzy clustering
C
      rkme=k-1
      do 30 m=1,nn
	 do 20 l=1,k
	    dp(m,l)=0.
	    p(m,l)=0.1/rkme
 20	 continue
 30   continue
      ndk=nn/k
      nd=ndk
      l=1
      do 50 m=1,nn
	 p(m,l)=0.9
	 if(m.lt.nd)go to 35
	 nd=nd+ndk
	 l=l+1
	 if(l.eq.k)nd=nn
 35	 do 40 lx=1,k
	    p(m,lx)=p(m,lx)**r
 40	 continue
 50   continue
C
C     initial criterion value
C
      cryt=0.
      do 100 l=1,k
	 esp(l)=0.
	 ef(l)=0.
	 do 90 m=1,nn
	    esp(l)=esp(l)+p(m,l)
	    do 80 j=1,nn
	       if(j.ne.m) then
		  j2=min0(m,j)
		  j1=(j2-1)*nn-(j2*(j2+1))/2+max0(m,j)
		  dp(m,l)=dp(m,l)+ p(j,l)*dss(j1)
		  ef(l)	 =ef(l)	 + p(j,l)*p(m,l)*dss(j1)
	       endif
 80	    continue
 90	 continue
	 cryt=cryt+ef(l)/(esp(l)*2.)
 100  continue
      crt=cryt
      reen=1./(r-1.)
C
C     start of iterations
C
      kaunt=1
      m=0
C
C     the new membership coefficients of the objects are calculated,
C     and the resulting value of the criterion is computed.
C
 200  m=m+1
      dt=0.
      do 210 l=1,k
	 pt(l)=((2.*esp(l)*esp(l))/(2.*esp(l)*dp(m,l)-ef(l)))**reen
	 dt=dt+pt(l)
 210  continue
      xx=0.
      do 220 l=1,k
	 pt(l)=pt(l)/dt
	 if(pt(l).le.0.) xx=xx+pt(l)
 220  continue
      do 240 l=1,k
	 if(pt(l).le.0.)pt(l)=0.
	 pt(l)=(pt(l)/(1-xx))**r
	 esp(l)=esp(l)+pt(l)-p(m,l)
	 do 230 j=1,nn
	    if(j.ne.m) then
	       j2=min0(m,j)
	       j1=(j2-1)*nn-(j2*(j2+1))/2+max0(m,j)
	       ddd=(pt(l)-p(m,l))*dss(j1)
	       dp(j,l)= dp(j,l)+ ddd
	       ef(l)  = ef(l)  + 2.*p(j,l)*ddd
	    endif
 230	 continue
	 p(m,l)=pt(l)
 240  continue
      if(m.lt.nn)go to 200
      cryt=0.
      eda=0.
      do 250 l=1,k
	 ann=nn
	 eda=eda+esp(l)/ann
	 cryt=cryt+ef(l)/(esp(l)*2.)
 250  continue
C
C     criterion is printed and tested for convergence
C
      if((crt/cryt-1.).le.eps)go to 500
      if(kaunt.lt.nyt)go to 300
      go to 500
 300  m=0
      kaunt=kaunt+1
      crt=cryt
      go to 200
C
C     non-fuzzyness index of libert is computed
C
 500  obj(1)=kaunt
      obj(2)=cryt
      zk=k
      edb=(zk*eda-1.)/(zk-1.)
      do 520 m=1,nn
	 do 510 l=1,k
	    p(m,l)=p(m,l)**rvers
 510	 continue
 520  continue
      return
      end
C
C
      subroutine caddy(nn,p,k,ktrue,nfuzz,ncluv,rdraw,nelem)

c Args
      integer nn, k, ktrue
      integer nfuzz(k), ncluv(nn), nelem(nn)
      double precision p(nn,k), rdraw(k)
C
c VARs
      integer l,m, nbest, jstay, ktry, knext, kwalk, kleft, ksup, lfuzz
      double precision pbest

      pbest=p(1,1)
      nbest=1
      do 10 l=2,k
	 if(p(1,l).le.pbest)go to 10
	 pbest=p(1,l)
	 nbest=l
 10   continue
      nfuzz(1)=nbest
      ncluv(1)=1
      ktrue=1
      do 20 m=2,nn
	 pbest=p(m,1)
	 nbest=1
	 do 30 l=2,k
	    if(p(m,l).le.pbest)go to 30
	    pbest=p(m,l)
	    nbest=l
 30	 continue
	 jstay=0
	 do 40 ktry=1,ktrue
	    if(nfuzz(ktry).ne.nbest)go to 40
	    ncluv(m)=ktry
	    jstay=1
 40	 continue
	 if(jstay.eq.1)go to 20
	 ktrue=ktrue+1
	 nfuzz(ktrue)=nbest
	 ncluv(m)=ktrue
 20   continue
      if(ktrue.ge.k)go to 100
      knext=ktrue+1
      do 60 kwalk=knext,k
	 do 70 kleft=1,k
	    jstay=0
	    ksup=kwalk-1
	    do 80 ktry=1,ksup
	       if(nfuzz(ktry).ne.kleft)go to 80
	       jstay=1
 80	    continue
	    if(jstay.eq.1)go to 70
	    nfuzz(kwalk)=kleft
	    go to 60
 70	 continue
 60   continue
 100  do 110 m=1,nn
	 do 120 l=1,k
	    lfuzz=nfuzz(l)
	    rdraw(l)=p(m,lfuzz)
 120	 continue
	 do 130 l=1,k
	    p(m,l)=rdraw(l)
 130	 continue
 110  continue
      end
C
C
      subroutine fygur(ktrue,nn,kk,hh,ncluv,nsend,nelem,
     1	   negbr,syl,srank,avsyl,ttsyl,dss,s,sylinf)
c     Args
      integer ktrue,nn,kk,hh
      integer ncluv(nn), nsend(nn), nelem(nn), negbr(nn)
      double precision syl(nn),srank(nn),avsyl(kk),ttsyl,dss(hh)
      double precision s, sylinf(nn,4)
c     VARs
      integer j,l, lang, lplac, mjl
      integer nsylr, ntt, nj, numcl, nclu, nbb, nl, njl
      double precision db,dysa,dysb, att,btt, rtt, rnn, symax

      nsylr=0
      ttsyl=0.0
      do 100 numcl=1,ktrue
	 ntt=0
	 do 30 j=1,nn
	    if(ncluv(j).eq.numcl) then
	       ntt=ntt+1
	       nelem(ntt)=j
	    endif
 30	 continue
	 do 40 j=1,ntt
	    nj=nelem(j)
	    dysb=1.1*s+1.0
	    negbr(j)=-1
	    do 41 nclu=1,ktrue
	       if(nclu.ne.numcl) then
		  nbb=0
		  db=0.0
		  do 43 l=1,nn
		     if(ncluv(l).eq.nclu) then
			nbb=nbb+1
			if(l.lt.nj) then
			   mjl=nn*(l-1)+nj-l*(l+1)/2
			   db=db+dss(mjl)
			else if(l.gt.nj) then
			   mjl=nn*(nj-1)+l-nj*(nj+1)/2
			   db=db+dss(mjl)
			endif
		     endif
 43		  continue
		  btt=nbb
		  db=db/btt
		  if(dysb .gt. db)then
		     dysb=db
		     negbr(j)=nclu
		  endif
	       endif
 41	    continue
	    if(ntt.eq.1)go to 50
	    dysa=0.0
	    do 45 l=1,ntt
	       nl=nelem(l)
	       if(nj.lt.nl) then
		  njl=nn*(nj-1)+nl-nj*(nj+1)/2
		  dysa=dysa+dss(njl)
	       else if(nj.gt.nl) then
		  njl=nn*(nl-1)+nj-nl*(nl+1)/2
		  dysa=dysa+dss(njl)
	       endif
 45	    continue
	    att=ntt-1
	    dysa=dysa/att
	    if(dysa.gt.0.0)go to 51
	    if(dysb.gt.0.0)go to 52
 50	    syl(j)=0.0
	    go to 40
 52	    syl(j)=1.0
	    go to 40
 51	    if(dysb.le.0.0)go to 53
	    if(dysb.gt.dysa)syl(j)=1.0-dysa/dysb
	    if(dysb.lt.dysa)syl(j)=dysb/dysa-1.0
	    if(dysb.eq.dysa)syl(j)=0.0
	    go to 54
 53	    syl(j)=-1.0
 54	    if(syl(j).le.(-1.0))syl(j)=-1.0
	    if(syl(j).ge.1.0)syl(j)=1.0
 40	 continue
	 avsyl(numcl)=0.0
	 do 60 j=1,ntt
	    symax=-2.0
	    do 70 l=1,ntt
	       if(symax.lt. syl(l)) then
		  symax=syl(l)
		  lang=l
	       endif
 70	    continue
	    nsend(j)=lang
	    srank(j)=syl(lang)
	    avsyl(numcl)=avsyl(numcl)+srank(j)
	    syl(lang)=-3.0
 60	 continue
	 ttsyl=ttsyl+avsyl(numcl)
	 rtt=ntt
	 avsyl(numcl)=avsyl(numcl)/rtt

	 if(ntt.lt.2) then
	    nsylr=nsylr+1
	    sylinf(nsylr,1)=numcl
	    sylinf(nsylr,2)=negbr(1)
	    sylinf(nsylr,3)=0.0
	    sylinf(nsylr,4)=nelem(1)
	 else
 75	    do 80 l=1,ntt
	       nsylr=nsylr+1
	       lplac=nsend(l)
	       sylinf(nsylr,1)=numcl
	       sylinf(nsylr,2)=negbr(lplac)
	       sylinf(nsylr,3)=srank(l)
	       sylinf(nsylr,4)=nelem(lplac)
 80	    continue
	 endif
 100  continue
      rnn=nn
      ttsyl=ttsyl/rnn
      end
