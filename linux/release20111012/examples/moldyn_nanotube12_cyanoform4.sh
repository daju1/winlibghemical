#!/bin/sh

set -e
set -x

./moldyn_nanotube12_cyanoform4_B0_E_P.sh &
./moldyn_nanotube12_cyanoform4_B0_E_P_I.sh &
./moldyn_nanotube12_cyanoform4_B0_T_P.sh &
./moldyn_nanotube12_cyanoform4_B0_T_P_I.sh &
./moldyn_nanotube12_cyanoform4_B2_E_P.sh &
./moldyn_nanotube12_cyanoform4_B2_E_P_I.sh &
./moldyn_nanotube12_cyanoform4_B2_T_P.sh &
./moldyn_nanotube12_cyanoform4_B2_T_P_I.sh &
