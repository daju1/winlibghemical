/* exchng.f -- translated by f2c (version 19991025).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Subroutine */ int exchng_(a, b, c__, d__, x, y, t, q, n)
doublereal *a, *b, *c__, *d__, *x, *y, *t, *q;
integer *n;
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i__;

/* ******************************************************************** */

/* THE CONTENTS OF A, C, T, AND X ARE STORED IN B, D, Q, AND Y! */

/*   THIS IS A DEDICATED ROUTINE, IT IS CALLED BY LINMIN AND LOCMIN ONLY. */

/* ******************************************************************** */
    /* Parameter adjustments */
    --y;
    --x;

    /* Function Body */
    *b = *a;
    *d__ = *c__;
    *q = *t;
    i__1 = *n;
    for (i__ = 1; i__ <= i__1; ++i__) {
	y[i__] = x[i__];
/* L10: */
    }
    return 0;

} /* exchng_ */

