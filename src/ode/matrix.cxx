/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/

#include "common.h"
#include "matrix.h"

// misc defines
#define ALLOCA dALLOCA16


void dSetZero (vec_t *a, int n)
{
  dAASSERT (a && n >= 0);
  while (n > 0) {
    *(a++) = 0;
    n--;
  }
}


void dSetValue (vec_t *a, int n, vec_t value)
{
  dAASSERT (a && n >= 0);
  while (n > 0) {
    *(a++) = value;
    n--;
  }
}


void dMultiply0 (vec_t *A, const vec_t *B, const vec_t *C, int p, int q, int r)
{
  int i,j,k,qskip,rskip,rpad;
  dAASSERT (A && B && C && p>0 && q>0 && r>0);
  qskip = dPAD(q);
  rskip = dPAD(r);
  rpad = rskip - r;
  vec_t sum;
  const vec_t *b,*c,*bb;
  bb = B;
  for (i=p; i; i--) {
    for (j=0 ; j<r; j++) {
      c = C + j;
      b = bb;
      sum = 0;
      for (k=q; k; k--, c+=rskip) sum += (*(b++))*(*c);
      *(A++) = sum; 
    }
    A += rpad;
    bb += qskip;
  }
}


void dMultiply1 (vec_t *A, const vec_t *B, const vec_t *C, int p, int q, int r)
{
  int i,j,k,pskip,rskip;
  vec_t sum;
  dAASSERT (A && B && C && p>0 && q>0 && r>0);
  pskip = dPAD(p);
  rskip = dPAD(r);
  for (i=0; i<p; i++) {
    for (j=0; j<r; j++) {
      sum = 0;
      for (k=0; k<q; k++) sum += B[i+k*pskip] * C[j+k*rskip];
      A[i*rskip+j] = sum;
    }
  }
}


void dMultiply2 (vec_t *A, const vec_t *B, const vec_t *C, int p, int q, int r)
{
  int i,j,k,z,rpad,qskip;
  vec_t sum;
  const vec_t *bb,*cc;
  dAASSERT (A && B && C && p>0 && q>0 && r>0);
  rpad = dPAD(r) - r;
  qskip = dPAD(q);
  bb = B;
  for (i=p; i; i--) {
    cc = C;
    for (j=r; j; j--) {
      z = 0;
      sum = 0;
      for (k=q; k; k--,z++) sum += bb[z] * cc[z];
      *(A++) = sum; 
      cc += qskip;
    }
    A += rpad;
    bb += qskip;
  }
}


int dFactorCholesky (vec_t *A, int n)
{
  int i,j,k,nskip;
  vec_t sum,*a,*b,*aa,*bb,*cc,*recip;
  dAASSERT (n > 0 && A);
  nskip = dPAD (n);
  recip = (vec_t*) ALLOCA (n * sizeof(vec_t));
  aa = A;
  for (i=0; i<n; i++) {
    bb = A;
    cc = A + i*nskip;
    for (j=0; j<i; j++) {
      sum = *cc;
      a = aa;
      b = bb;
      for (k=j; k; k--) sum -= (*(a++))*(*(b++));
      *cc = sum * recip[j];
      bb += nskip;
      cc++;
    }
    sum = *cc;
    a = aa;
    for (k=i; k; k--, a++) sum -= (*a)*(*a);
    if (sum <= REAL(0.0)) return 0;
    *cc = X_sqrt(sum);
    recip[i] = X_recip (*cc);
    aa += nskip;
  }
  return 1;
}


void dSolveCholesky (const vec_t *L, vec_t *b, int n)
{
  int i,k,nskip;
  vec_t sum,*y;
  dAASSERT (n > 0 && L && b);
  nskip = dPAD (n);
  y = (vec_t*) ALLOCA (n*sizeof(vec_t));
  for (i=0; i<n; i++) {
    sum = 0;
    for (k=0; k < i; k++) sum += L[i*nskip+k]*y[k];
    y[i] = (b[i]-sum)/L[i*nskip+i];
  }
  for (i=n-1; i >= 0; i--) {
    sum = 0;
    for (k=i+1; k < n; k++) sum += L[k*nskip+i]*b[k];
    b[i] = (y[i]-sum)/L[i*nskip+i];
  }
}


int dInvertPDMatrix (const vec_t *A, vec_t *Ainv, int n)
{
  int i,j,nskip;
  vec_t *L,*x;
  dAASSERT (n > 0 && A && Ainv);
  nskip = dPAD (n);
  L = (vec_t*) ALLOCA (nskip*n*sizeof(vec_t));
  memcpy (L,A,nskip*n*sizeof(vec_t));
  x = (vec_t*) ALLOCA (n*sizeof(vec_t));
  if (dFactorCholesky (L,n)==0) return 0;
  dSetZero (Ainv,n*nskip);	// make sure all padding elements set to 0
  for (i=0; i<n; i++) {
    for (j=0; j<n; j++) x[j] = 0;
    x[i] = 1;
    dSolveCholesky (L,x,n);
    for (j=0; j<n; j++) Ainv[j*nskip+i] = x[j];
  }
  return 1;  
}


int dIsPositiveDefinite (const vec_t *A, int n)
{
  vec_t *Acopy;
  dAASSERT (n > 0 && A);
  int nskip = dPAD (n);
  Acopy = (vec_t*) ALLOCA (nskip*n * sizeof(vec_t));
  memcpy (Acopy,A,nskip*n * sizeof(vec_t));
  return dFactorCholesky (Acopy,n);
}


/***** this has been replaced by a faster version
void dSolveL1T (const vec_t *L, vec_t *b, int n, int nskip)
{
  int i,j;
  dAASSERT (L && b && n >= 0 && nskip >= n);
  vec_t sum;
  for (i=n-2; i>=0; i--) {
    sum = 0;
    for (j=i+1; j<n; j++) sum += L[j*nskip+i]*b[j];
    b[i] -= sum;
  }
}
*/


void dVectorScale (vec_t *a, const vec_t *d, int n)
{
  dAASSERT (a && d && n >= 0);
  for (int i=0; i<n; i++) a[i] *= d[i];
}


void dSolveLDLT (const vec_t *L, const vec_t *d, vec_t *b, int n, int nskip)
{
  dAASSERT (L && d && b && n > 0 && nskip >= n);
  dSolveL1 (L,b,n,nskip);
  dVectorScale (b,d,n);
  dSolveL1T (L,b,n,nskip);
}


void dLDLTAddTL (vec_t *L, vec_t *d, const vec_t *a, int n, int nskip)
{
  int j,p;
  vec_t *W1,*W2,W11,W21,alpha1,alpha2,alphanew,gamma1,gamma2,k1,k2,Wp,ell,dee;
  dAASSERT (L && d && a && n > 0 && nskip >= n);

  if (n < 2) return;
  W1 = (vec_t*) ALLOCA (n*sizeof(vec_t));
  W2 = (vec_t*) ALLOCA (n*sizeof(vec_t));

  W1[0] = 0;
  W2[0] = 0;
  for (j=1; j<n; j++) W1[j] = W2[j] = a[j] * M_SQRT1_2;
  W11 = (REAL(0.5)*a[0]+1)*M_SQRT1_2;
  W21 = (REAL(0.5)*a[0]-1)*M_SQRT1_2;

  alpha1=1;
  alpha2=1;

  dee = d[0];
  alphanew = alpha1 + (W11*W11)*dee;
  dee /= alphanew;
  gamma1 = W11 * dee;
  dee *= alpha1;
  alpha1 = alphanew;
  alphanew = alpha2 - (W21*W21)*dee;
  dee /= alphanew;
  gamma2 = W21 * dee;
  alpha2 = alphanew;
  k1 = REAL(1.0) - W21*gamma1;
  k2 = W21*gamma1*W11 - W21;
  for (p=1; p<n; p++) {
    Wp = W1[p];
    ell = L[p*nskip];
    W1[p] =    Wp - W11*ell;
    W2[p] = k1*Wp +  k2*ell;
  }

  for (j=1; j<n; j++) {
    dee = d[j];
    alphanew = alpha1 + (W1[j]*W1[j])*dee;
    dee /= alphanew;
    gamma1 = W1[j] * dee;
    dee *= alpha1;
    alpha1 = alphanew;
    alphanew = alpha2 - (W2[j]*W2[j])*dee;
    dee /= alphanew;
    gamma2 = W2[j] * dee;
    dee *= alpha2;
    d[j] = dee;
    alpha2 = alphanew;

    k1 = W1[j];
    k2 = W2[j];
    for (p=j+1; p<n; p++) {
      ell = L[p*nskip+j];
      Wp = W1[p] - k1 * ell;
      ell += gamma1 * Wp;
      W1[p] = Wp;
      Wp = W2[p] - k2 * ell;
      ell -= gamma2 * Wp;
      W2[p] = Wp;
      L[p*nskip+j] = ell;
    }
  }
}


// macros for dLDLTRemove() for accessing A - either access the matrix
// directly or access it via row pointers. we are only supposed to reference
// the lower triangle of A (it is symmetric), but indexes i and j come from
// permutation vectors so they are not predictable. so do a test on the
// indexes - this should not slow things down too much, as we don't do this
// in an inner loop.

#define _GETA(i,j) (A[i][j])
//#define _GETA(i,j) (A[(i)*nskip+(j)])
#define GETA(i,j) ((i > j) ? _GETA(i,j) : _GETA(j,i))


void dLDLTRemove (vec_t **A, const int *p, vec_t *L, vec_t *d,
		  int n1, int n2, int r, int nskip)
{
  int i;
  dAASSERT(A && p && L && d && n1 > 0 && n2 > 0 && r >= 0 && r < n2 &&
	   n1 >= n2 && nskip >= n1);
  #ifndef dNODEBUG
  for (i=0; i<n2; i++) dIASSERT(p[i] >= 0 && p[i] < n1);
  #endif

  if (r==n2-1) {
    return;		// deleting last row/col is easy
  }
  else if (r==0) {
    vec_t *a = (vec_t*) ALLOCA (n2 * sizeof(vec_t));
    for (i=0; i<n2; i++) a[i] = -GETA(p[i],p[0]);
    a[0] += REAL(1.0);
    dLDLTAddTL (L,d,a,n2,nskip);
  }
  else {
    vec_t *t = (vec_t*) ALLOCA (r * sizeof(vec_t));
    vec_t *a = (vec_t*) ALLOCA ((n2-r) * sizeof(vec_t));
    for (i=0; i<r; i++) t[i] = L[r*nskip+i] / d[i];
    for (i=0; i<(n2-r); i++)
      a[i] = dDot(L+(r+i)*nskip,t,r) - GETA(p[r+i],p[r]);
    a[0] += REAL(1.0);
    dLDLTAddTL (L + r*nskip+r, d+r, a, n2-r, nskip);
  }

  // snip out row/column r from L and d
  dRemoveRowCol (L,n2,nskip,r);
  if (r < (n2-1)) memmove (d+r,d+r+1,(n2-r-1)*sizeof(vec_t));
}


void dRemoveRowCol (vec_t *A, int n, int nskip, int r)
{
  int i;
  dAASSERT(A && n > 0 && nskip >= n && r >= 0 && r < n);
  if (r >= n-1) return;
  if (r > 0) {
    for (i=0; i<r; i++)
      memmove (A+i*nskip+r,A+i*nskip+r+1,(n-r-1)*sizeof(vec_t));
    for (i=r; i<(n-1); i++)
      memcpy (A+i*nskip,A+i*nskip+nskip,r*sizeof(vec_t));
  }
  for (i=r; i<(n-1); i++)
    memcpy (A+i*nskip+r,A+i*nskip+nskip+r+1,(n-r-1)*sizeof(vec_t));
}
