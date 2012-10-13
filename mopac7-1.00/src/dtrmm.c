/* dtrmm.f -- translated by f2c (version 19991025).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"

/* Subroutine */ int dtrmm_(side, uplo, transa, diag, m, n, alpha, a, lda, b, 
	ldb, side_len, uplo_len, transa_len, diag_len)
char *side, *uplo, *transa, *diag;
integer *m, *n;
doublereal *alpha, *a;
integer *lda;
doublereal *b;
integer *ldb;
ftnlen side_len;
ftnlen uplo_len;
ftnlen transa_len;
ftnlen diag_len;
{
    /* System generated locals */
    integer a_dim1, a_offset, b_dim1, b_offset, i__1, i__2, i__3;

    /* Local variables */
    static integer info;
    static doublereal temp;
    static integer i__, j, k;
    static logical lside;
    extern logical lsame_();
    static integer nrowa;
    static logical upper;
    extern /* Subroutine */ int xerbla_();
    static logical nounit;

/*     .. SCALAR ARGUMENTS .. */
/*     .. ARRAY ARGUMENTS .. */
/*     .. */

/*  PURPOSE */
/*  ======= */

/*  DTRMM  PERFORMS ONE OF THE MATRIX-MATRIX OPERATIONS */

/*     B := ALPHA*OP( A )*B,   OR   B := ALPHA*B*OP( A ), */

/*  WHERE  ALPHA  IS A SCALAR,  B  IS AN M BY N MATRIX,  A  IS A UNIT, OR */
/*  NON-UNIT,  UPPER OR LOWER TRIANGULAR MATRIX  AND  OP( A )  IS ONE  OF */

/*     OP( A ) = A   OR   OP( A ) = A'. */

/*  PARAMETERS */
/*  ========== */

/*  SIDE   - CHARACTER*1. */
/*           ON ENTRY,  SIDE SPECIFIES WHETHER  OP( A ) MULTIPLIES B FROM */
/*           THE LEFT OR RIGHT AS FOLLOWS: */

/*              SIDE = 'L' OR 'L'   B := ALPHA*OP( A )*B. */

/*              SIDE = 'R' OR 'R'   B := ALPHA*B*OP( A ). */

/*           UNCHANGED ON EXIT. */

/*  UPLO   - CHARACTER*1. */
/*           ON ENTRY, UPLO SPECIFIES WHETHER THE MATRIX A IS AN UPPER OR */
/*           LOWER TRIANGULAR MATRIX AS FOLLOWS: */

/*              UPLO = 'U' OR 'U'   A IS AN UPPER TRIANGULAR MATRIX. */

/*              UPLO = 'L' OR 'L'   A IS A LOWER TRIANGULAR MATRIX. */

/*           UNCHANGED ON EXIT. */

/*  TRANSA - CHARACTER*1. */
/*           ON ENTRY, TRANSA SPECIFIES THE FORM OF OP( A ) TO BE USED IN */
/*           THE MATRIX MULTIPLICATION AS FOLLOWS: */

/*              TRANSA = 'N' OR 'N'   OP( A ) = A. */

/*              TRANSA = 'T' OR 'T'   OP( A ) = A'. */

/*              TRANSA = 'C' OR 'C'   OP( A ) = A'. */

/*           UNCHANGED ON EXIT. */

/*  DIAG   - CHARACTER*1. */
/*           ON ENTRY, DIAG SPECIFIES WHETHER OR NOT A IS UNIT TRIANGULAR */
/*           AS FOLLOWS: */

/*              DIAG = 'U' OR 'U'   A IS ASSUMED TO BE UNIT TRIANGULAR. */

/*              DIAG = 'N' OR 'N'   A IS NOT ASSUMED TO BE UNIT */
/*                                  TRIANGULAR. */

/*           UNCHANGED ON EXIT. */

/*  M      - INTEGER. */
/*           ON ENTRY, M SPECIFIES THE NUMBER OF ROWS OF B. M MUST BE AT */
/*           LEAST ZERO. */
/*           UNCHANGED ON EXIT. */

/*  N      - INTEGER. */
/*           ON ENTRY, N SPECIFIES THE NUMBER OF COLUMNS OF B.  N MUST BE */
/*           AT LEAST ZERO. */
/*           UNCHANGED ON EXIT. */

/*  ALPHA  - DOUBLE PRECISION. */
/*           ON ENTRY,  ALPHA SPECIFIES THE SCALAR  ALPHA. WHEN  ALPHA IS */
/*           ZERO THEN  A IS NOT REFERENCED AND  B NEED NOT BE SET BEFORE */
/*           ENTRY. */
/*           UNCHANGED ON EXIT. */

/*  A      - DOUBLE PRECISION ARRAY OF DIMENSION ( LDA, K ), WHERE K IS M */
/*           WHEN  SIDE = 'L' OR 'L'  AND IS  N  WHEN  SIDE = 'R' OR 'R'. */
/*           BEFORE ENTRY  WITH  UPLO = 'U' OR 'U',  THE  LEADING  K BY K */
/*           UPPER TRIANGULAR PART OF THE ARRAY  A MUST CONTAIN THE UPPER */
/*           TRIANGULAR MATRIX  AND THE STRICTLY LOWER TRIANGULAR PART OF */
/*           A IS NOT REFERENCED. */
/*           BEFORE ENTRY  WITH  UPLO = 'L' OR 'L',  THE  LEADING  K BY K */
/*           LOWER TRIANGULAR PART OF THE ARRAY  A MUST CONTAIN THE LOWER */
/*           TRIANGULAR MATRIX  AND THE STRICTLY UPPER TRIANGULAR PART OF */
/*           A IS NOT REFERENCED. */
/*           NOTE THAT WHEN  DIAG = 'U' OR 'U',  THE DIAGONAL ELEMENTS OF */
/*           A  ARE NOT REFERENCED EITHER,  BUT ARE ASSUMED TO BE  UNITY. */
/*           UNCHANGED ON EXIT. */

/*  LDA    - INTEGER. */
/*           ON ENTRY, LDA SPECIFIES THE FIRST DIMENSION OF A AS DECLARED */
/*           IN THE CALLING (SUB) PROGRAM.  WHEN  SIDE = 'L' OR 'L'  THEN */
/*           LDA  MUST BE AT LEAST  MAX( 1, M ),  WHEN  SIDE = 'R' OR 'R' */
/*           THEN LDA MUST BE AT LEAST MAX( 1, N ). */
/*           UNCHANGED ON EXIT. */

/*  B      - DOUBLE PRECISION ARRAY OF DIMENSION ( LDB, N ). */
/*           BEFORE ENTRY,  THE LEADING  M BY N PART OF THE ARRAY  B MUST */
/*           CONTAIN THE MATRIX  B,  AND  ON EXIT  IS OVERWRITTEN  BY THE */
/*           TRANSFORMED MATRIX. */

/*  LDB    - INTEGER. */
/*           ON ENTRY, LDB SPECIFIES THE FIRST DIMENSION OF B AS DECLARED */
/*           IN  THE  CALLING  (SUB)  PROGRAM.   LDB  MUST  BE  AT  LEAST */
/*           MAX( 1, M ). */
/*           UNCHANGED ON EXIT. */


/*  LEVEL 3 BLAS ROUTINE. */

/*  -- WRITTEN ON 8-FEBRUARY-1989. */
/*     JACK DONGARRA, ARGONNE NATIONAL LABORATORY. */
/*     IAIN DUFF, AERE HARWELL. */
/*     JEREMY DU CROZ, NUMERICAL ALGORITHMS GROUP LTD. */
/*     SVEN HAMMARLING, NUMERICAL ALGORITHMS GROUP LTD. */


/*     .. EXTERNAL FUNCTIONS .. */
/*     .. EXTERNAL SUBROUTINES .. */
/*     .. INTRINSIC FUNCTIONS .. */
/*     .. LOCAL SCALARS .. */
/*     .. PARAMETERS .. */
/*     .. */
/*     .. EXECUTABLE STATEMENTS .. */

/*     TEST THE INPUT PARAMETERS. */

    /* Parameter adjustments */
    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    b_dim1 = *ldb;
    b_offset = 1 + b_dim1 * 1;
    b -= b_offset;

    /* Function Body */
    lside = lsame_(side, "L", (ftnlen)1, (ftnlen)1);
    if (lside) {
	nrowa = *m;
    } else {
	nrowa = *n;
    }
    nounit = lsame_(diag, "N", (ftnlen)1, (ftnlen)1);
    upper = lsame_(uplo, "U", (ftnlen)1, (ftnlen)1);

    info = 0;
    if (! lside && ! lsame_(side, "R", (ftnlen)1, (ftnlen)1)) {
	info = 1;
    } else if (! upper && ! lsame_(uplo, "L", (ftnlen)1, (ftnlen)1)) {
	info = 2;
    } else if (! lsame_(transa, "N", (ftnlen)1, (ftnlen)1) && ! lsame_(transa,
	     "T", (ftnlen)1, (ftnlen)1) && ! lsame_(transa, "C", (ftnlen)1, (
	    ftnlen)1)) {
	info = 3;
    } else if (! lsame_(diag, "U", (ftnlen)1, (ftnlen)1) && ! lsame_(diag, 
	    "N", (ftnlen)1, (ftnlen)1)) {
	info = 4;
    } else if (*m < 0) {
	info = 5;
    } else if (*n < 0) {
	info = 6;
    } else if (*lda < max(1,nrowa)) {
	info = 9;
    } else if (*ldb < max(1,*m)) {
	info = 11;
    }
    if (info != 0) {
	xerbla_("DTRMM ", &info, (ftnlen)6);
	return 0;
    }

/*     QUICK RETURN IF POSSIBLE. */

    if (*n == 0) {
	return 0;
    }

/*     AND WHEN  ALPHA.EQ.ZERO. */

    if (*alpha == 0.) {
	i__1 = *n;
	for (j = 1; j <= i__1; ++j) {
	    i__2 = *m;
	    for (i__ = 1; i__ <= i__2; ++i__) {
		b[i__ + j * b_dim1] = 0.;
/* L10: */
	    }
/* L20: */
	}
	return 0;
    }

/*     START THE OPERATIONS. */

    if (lside) {
	if (lsame_(transa, "N", (ftnlen)1, (ftnlen)1)) {

/*           FORM  B := ALPHA*A*B. */

	    if (upper) {
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    i__2 = *m;
		    for (k = 1; k <= i__2; ++k) {
			if (b[k + j * b_dim1] != 0.) {
			    temp = *alpha * b[k + j * b_dim1];
			    i__3 = k - 1;
			    for (i__ = 1; i__ <= i__3; ++i__) {
				b[i__ + j * b_dim1] += temp * a[i__ + k * 
					a_dim1];
/* L30: */
			    }
			    if (nounit) {
				temp *= a[k + k * a_dim1];
			    }
			    b[k + j * b_dim1] = temp;
			}
/* L40: */
		    }
/* L50: */
		}
	    } else {
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    for (k = *m; k >= 1; --k) {
			if (b[k + j * b_dim1] != 0.) {
			    temp = *alpha * b[k + j * b_dim1];
			    b[k + j * b_dim1] = temp;
			    if (nounit) {
				b[k + j * b_dim1] *= a[k + k * a_dim1];
			    }
			    i__2 = *m;
			    for (i__ = k + 1; i__ <= i__2; ++i__) {
				b[i__ + j * b_dim1] += temp * a[i__ + k * 
					a_dim1];
/* L60: */
			    }
			}
/* L70: */
		    }
/* L80: */
		}
	    }
	} else {

/*           FORM  B := ALPHA*B*A'. */

	    if (upper) {
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    for (i__ = *m; i__ >= 1; --i__) {
			temp = b[i__ + j * b_dim1];
			if (nounit) {
			    temp *= a[i__ + i__ * a_dim1];
			}
			i__2 = i__ - 1;
			for (k = 1; k <= i__2; ++k) {
			    temp += a[k + i__ * a_dim1] * b[k + j * b_dim1];
/* L90: */
			}
			b[i__ + j * b_dim1] = *alpha * temp;
/* L100: */
		    }
/* L110: */
		}
	    } else {
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    i__2 = *m;
		    for (i__ = 1; i__ <= i__2; ++i__) {
			temp = b[i__ + j * b_dim1];
			if (nounit) {
			    temp *= a[i__ + i__ * a_dim1];
			}
			i__3 = *m;
			for (k = i__ + 1; k <= i__3; ++k) {
			    temp += a[k + i__ * a_dim1] * b[k + j * b_dim1];
/* L120: */
			}
			b[i__ + j * b_dim1] = *alpha * temp;
/* L130: */
		    }
/* L140: */
		}
	    }
	}
    } else {
	if (lsame_(transa, "N", (ftnlen)1, (ftnlen)1)) {

/*           FORM  B := ALPHA*B*A. */

	    if (upper) {
		for (j = *n; j >= 1; --j) {
		    temp = *alpha;
		    if (nounit) {
			temp *= a[j + j * a_dim1];
		    }
		    i__1 = *m;
		    for (i__ = 1; i__ <= i__1; ++i__) {
			b[i__ + j * b_dim1] = temp * b[i__ + j * b_dim1];
/* L150: */
		    }
		    i__1 = j - 1;
		    for (k = 1; k <= i__1; ++k) {
			if (a[k + j * a_dim1] != 0.) {
			    temp = *alpha * a[k + j * a_dim1];
			    i__2 = *m;
			    for (i__ = 1; i__ <= i__2; ++i__) {
				b[i__ + j * b_dim1] += temp * b[i__ + k * 
					b_dim1];
/* L160: */
			    }
			}
/* L170: */
		    }
/* L180: */
		}
	    } else {
		i__1 = *n;
		for (j = 1; j <= i__1; ++j) {
		    temp = *alpha;
		    if (nounit) {
			temp *= a[j + j * a_dim1];
		    }
		    i__2 = *m;
		    for (i__ = 1; i__ <= i__2; ++i__) {
			b[i__ + j * b_dim1] = temp * b[i__ + j * b_dim1];
/* L190: */
		    }
		    i__2 = *n;
		    for (k = j + 1; k <= i__2; ++k) {
			if (a[k + j * a_dim1] != 0.) {
			    temp = *alpha * a[k + j * a_dim1];
			    i__3 = *m;
			    for (i__ = 1; i__ <= i__3; ++i__) {
				b[i__ + j * b_dim1] += temp * b[i__ + k * 
					b_dim1];
/* L200: */
			    }
			}
/* L210: */
		    }
/* L220: */
		}
	    }
	} else {

/*           FORM  B := ALPHA*B*A'. */

	    if (upper) {
		i__1 = *n;
		for (k = 1; k <= i__1; ++k) {
		    i__2 = k - 1;
		    for (j = 1; j <= i__2; ++j) {
			if (a[j + k * a_dim1] != 0.) {
			    temp = *alpha * a[j + k * a_dim1];
			    i__3 = *m;
			    for (i__ = 1; i__ <= i__3; ++i__) {
				b[i__ + j * b_dim1] += temp * b[i__ + k * 
					b_dim1];
/* L230: */
			    }
			}
/* L240: */
		    }
		    temp = *alpha;
		    if (nounit) {
			temp *= a[k + k * a_dim1];
		    }
		    if (temp != 1.) {
			i__2 = *m;
			for (i__ = 1; i__ <= i__2; ++i__) {
			    b[i__ + k * b_dim1] = temp * b[i__ + k * b_dim1];
/* L250: */
			}
		    }
/* L260: */
		}
	    } else {
		for (k = *n; k >= 1; --k) {
		    i__1 = *n;
		    for (j = k + 1; j <= i__1; ++j) {
			if (a[j + k * a_dim1] != 0.) {
			    temp = *alpha * a[j + k * a_dim1];
			    i__2 = *m;
			    for (i__ = 1; i__ <= i__2; ++i__) {
				b[i__ + j * b_dim1] += temp * b[i__ + k * 
					b_dim1];
/* L270: */
			    }
			}
/* L280: */
		    }
		    temp = *alpha;
		    if (nounit) {
			temp *= a[k + k * a_dim1];
		    }
		    if (temp != 1.) {
			i__1 = *m;
			for (i__ = 1; i__ <= i__1; ++i__) {
			    b[i__ + k * b_dim1] = temp * b[i__ + k * b_dim1];
/* L290: */
			}
		    }
/* L300: */
		}
	    }
	}
    }

    return 0;

/*     END OF DTRMM . */

} /* dtrmm_ */

