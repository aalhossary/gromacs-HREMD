#!/bin/bash
pwd=./

## needed modify
nnode=8
type=gcm3gmx
nrep=1000;

## let nodem1=nnode-1
kk=120.27221
exe=/home/n1302239h/soft/gromacs-HREMD_5.1rc+/bin
#gmx_mpi=gmx_5.1hremdmpi
gmx=gmx_5.1hremd

time=`awk -v tt=${1}  'BEGIN{split(tt,aa,"_");split(aa[3],bb,".");print bb[1]}'`
node=`awk -v tt=${1}  'BEGIN{split(tt,aa,"_");print aa[2]}'`
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

#echo $node    $node2

if [ $node2 -ge 0 ]; then
if [ $node2 -lt $nnode ]; then

b1=`awk -v node=$node  -v kk=$kk '{if ($1==node+1) {printf "%25.9f", kk/$2; exit}}' $pwd/replica-temp.dat`
b2=`awk -v node=$node2 -v kk=$kk '{if ($1==node+1) {printf "%25.9f", kk/$2; exit}}' $pwd/replica-temp.dat`
bt=`expr "$b1-$b2"|bc -l`
#echo $bt

$exe/$gmx mdrun -s $pwd/p1_${node}.tpr  -rerun $1 -g ${1}_$node.log -e ${1}_$node -o ${1}_$node  1>/dev/null 2>/dev/null 
echo "10 0 " | $exe/$gmx energy -f ${1}_$node -o ${1}_$node 1>/dev/null 2>/dev/null
e1=`awk '{if (index($0,"Potential") > 0){getline;printf "%25.9f", $2*1.0; exit} }' ${1}_$node.xvg`
#echo $e1

$exe/$gmx mdrun -s $pwd/p1_${node2}.tpr -rerun $1 -g ${1}_$node2.log -e ${1}_$node2 -o ${1}_$node2  1>/dev/null 2>/dev/null
echo "10 0 " | $exe/$gmx energy -f ${1}_$node2 -o ${1}_$node2 1>/dev/null 2>/dev/null
e2=`awk '{if (index($0,"Potential") > 0){getline; printf "%25.9f", $2*1.0; exit} }' ${1}_$node2.xvg`
#echo $e2
ee=`expr "($b1*$e1-$b2*$e2)/$bt" | bc -l`

echo $time $node $node2 $e1 $b1 $e2 $b2 $ee >> record_$node.log
#echo $ee
fi
fi

rm ${1}*  1>/dev/null 2>/dev/null
echo $ee

