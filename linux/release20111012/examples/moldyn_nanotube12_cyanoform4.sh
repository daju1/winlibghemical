#!/bin/sh

set -e
set -x

./moldyn_nanotube12_cyanoform4_B0_T.sh &
./moldyn_nanotube12_cyanoform4_B0_T_I.sh &
./moldyn_nanotube12_cyanoform4_B0_T_P.sh &
./moldyn_nanotube12_cyanoform4_B0_T_P_I.sh &
./moldyn_nanotube12_cyanoform4_B2_T.sh &
./moldyn_nanotube12_cyanoform4_B2_T_I.sh &
./moldyn_nanotube12_cyanoform4_B2_T_P.sh &
./moldyn_nanotube12_cyanoform4_B2_T_P_I.sh &

./moldyn_nanotube12_cyanoform4_B0_H.sh &
./moldyn_nanotube12_cyanoform4_B0_H_I.sh &
./moldyn_nanotube12_cyanoform4_B0_H_P.sh &
./moldyn_nanotube12_cyanoform4_B0_H_P_I.sh &
./moldyn_nanotube12_cyanoform4_B2_H.sh &
./moldyn_nanotube12_cyanoform4_B2_H_I.sh &
./moldyn_nanotube12_cyanoform4_B2_H_P.sh &
./moldyn_nanotube12_cyanoform4_B2_H_P_I.sh &

