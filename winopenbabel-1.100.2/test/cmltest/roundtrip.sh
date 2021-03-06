#!/bin/sh
# test OB with CML
# $1 is inputfile root
# $2 is inputfile suffix
# $3 is inputfile type
# e.g. roundtrip foo mol mdl

# Make sure we know the absolute path of the programs we're trying to 
# call, in case the test is called from somewhere eles
if `env | grep ^builddir > /dev/null 2>&1`; then
   BABEL=$builddir/../src/babel
else
   builddir=..
   BABEL=../../src/babel
fi

# CML1 output
$BABEL -i$3 $1.$2 -ocml $1.1.cml -x1v
$builddir/roundtrip $1.$2 $1.1.cml
# CML2 output
$BABEL -i$3 $1.$2 -ocml $1.2.cml -x2v
$builddir/roundtrip $1.$2 $1.2.cml
# CML1+array output
$BABEL -i$3 $1.$2 -ocml $1.a1.cml -xa1v
$builddir/roundtrip $1.$2 $1.a1.cml
# CML2+array output
$BABEL -i$3 $1.$2 -ocml $1.a2.cml -xa2v
$builddir/roundtrip $1.$2 $1.a2.cml

# roundtrip to MOL; should be identical
$BABEL -icml $1.1.cml  -o$3 $1.1.$2 -x2v
$builddir/roundtrip $1.1.cml $1.1.$2
$BABEL -icml $1.2.cml  -o$3 $1.2.$2 -x2v
$builddir/roundtrip $1.2.cml $1.2.$2
$BABEL -icml $1.a1.cml -o$3 $1.a1.$2 -x2v
$builddir/roundtrip $1.a1.cml $1.a1.$2
$BABEL -icml $1.a2.cml -o$3 $1.a2.$2 -x2v
$builddir/roundtrip $1.a2.cml $1.a2.$2

# And check to make sure the four $2 files are the same !
$builddir/roundtrip $1.1.$2 $1.2.$2
$builddir/roundtrip $1.1.$2 $1.a1.$2
$builddir/roundtrip $1.1.$2 $1.a2.$2
