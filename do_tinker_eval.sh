#!/bin/bash
pwd=./

## needed modify
nnode=12
type=gcm3gmx
nrep=1000;

## let nodem1=nnode-1
kk=120.27221

exe=$HOME/software/gromacs-3.3/bin

time=`awk -v tt=${1}  'BEGIN{split(tt,aa,"_");split(aa[2],bb,".");print bb[1]}'`
node=`awk -v tt=${1}  'BEGIN{split(tt,aa,"traj");split(aa[2],bb,"_");print bb[1]}'`

ddn=`awk -v time=$time -v node=$node 'BEGIN{
nrep=1000;
aa=int(time/nrep+0.001)%2;

if ((node+aa)%2 == 0) ddn=-1
else
ddn=1

print ddn
}'`

let node2=node+ddn

ee=0

if [ $node2 -ge 0 ]; then
if [ $node2 -lt $nnode ]; then

b1=`awk -v node=$node  -v kk=$kk '{if ($1==node+1) {printf "%25.9f", kk/$2; exit}}' $pwd/prep/replica-temp.dat`
b2=`awk -v node=$node2 -v kk=$kk '{if ($1==node+1) {printf "%25.9f", kk/$2; exit}}' $pwd/prep/replica-temp.dat`
bt=`expr "$b1-$b2"|bc -l`

$exe/mdrun_33snompi -s $pwd/prep/vchmod${node}_${type}.tpr  -rerun $1 -g ${1}_$node.log -e ${1}_$node -o ${1}_$node  1>/dev/null 2>/dev/null 
echo "10 0 " | g_energy_33s -f ${1}_$node -o ${1}_$node 1>/dev/null 2>/dev/null
e1=`awk '{if (index($0,"Potential") > 0){getline;printf "%25.9f", $2*1.0; exit} }' ${1}_$node.xvg`

$exe/mdrun_33snompi -s $pwd/prep/vchmod${node2}_${type}.tpr -rerun $1 -g ${1}_$node2.log -e ${1}_$node2 -o ${1}_$node2  1>/dev/null 2>/dev/null
echo "10 0 " | g_energy_33s -f ${1}_$node2 -o ${1}_$node2 1>/dev/null 2>/dev/null
e2=`awk '{if (index($0,"Potential") > 0){getline; printf "%25.9f", $2*1.0; exit} }' ${1}_$node2.xvg`

ee=`expr "($b1*$e1-$b2*$e2)/$bt" | bc -l`
## echo $time $node $node2 $e1 $b1 $e2 $b2 $ee >> record_$node.log

fi
fi

rm ${1}*  1>/dev/null 2>/dev/null
echo $ee
