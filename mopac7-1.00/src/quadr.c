/* quadr.f -- translated by f2c (version 19991025).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Subroutine */ int quadr_(f0, f1, f2, x1, x2, a, b, c__)
doublereal *f0, *f1, *f2, *x1, *x2, *a, *b, *c__;
{
    /* System generated locals */
    doublereal d__1, d__2;

/* *************************************************** */
/*                                                  * */
/*    QUADR CALCULATES THE A, B AND C IN THE EQUNS. * */
/*                                                  * */
/*     A                   =   F0                   * */
/*     A + B.X0 + C.X0**2  =   F1                   * */
/*     A + B.X2 + C.X2**2  =   F2                   * */
/*                                                  * */
/* *************************************************** */
/* Computing 2nd power */
    d__1 = *x1;
/* Computing 2nd power */
    d__2 = *x2;
    *c__ = (*x2 * (*f1 - *f0) - *x1 * (*f2 - *f0)) / (*x2 * (d__1 * d__1) - *
	    x1 * (d__2 * d__2));
/* Computing 2nd power */
    d__1 = *x1;
    *b = (*f1 - *f0 - *c__ * (d__1 * d__1)) / *x1;
    *a = *f0;
    return 0;
} /* quadr_ */

