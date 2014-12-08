#!/bin/bash
fname=compress
iname=compress.in.Z
oname=output
build_dir=../Release+Asserts/lib

rm *.bc *.s *.dot *.exe

clang -emit-llvm -o $fname.bc -c $fname.c || { echo "Failed to emit llvm bc"; exit 1; }

opt -mem2reg < $fname.bc > $fname.opt.bc || { echo "Failed to opt-load"; exit 1; }
size1=$(stat -c %s $fname.opt.bc)
llc $fname.opt.bc -o $fname.s
g++ -o $fname.exe $fname.s

opt -dot-cfg $fname.opt.bc
mv cfg.main.dot non-swift-cfg.dot
dot -Tpdf non-swift-cfg.dot -o $fname-non-swift-cfg.pdf

opt -load $build_dir/swift.so -swift-r -mem2reg -fault < $fname.opt.bc > $fname.swift.bc || { echo "Failed to opt-load"; exit 1; }
size2=$(stat -c %s $fname.swift.bc)


t1=$(date +%s%N)
./$fname.exe < $iname > $oname.0
ret1=$?
t2=$(date +%s%N)


llc $fname.swift.bc -o $fname.swift.s
g++ -o $fname.swift.exe $fname.swift.s
t3=$(date +%s%N)
./$fname.swift.exe < $iname > $oname.1
ret2=$?
t4=$(date +%s%N)

opt -dot-cfg $fname.swift.bc
mv cfg.main.dot swift-cfg.dot
dot -Tpdf swift-cfg.dot -o $fname-swift-cfg.pdf
mv cfg.majority0.dot majority0.dot
dot -Tpdf majority0.dot -o $fname-majority0-cfg.pdf
mv cfg.majority1.dot majority1.dot
dot -Tpdf majority1.dot -o $fname-majority1-cfg.pdf

rm *.bc *.s *.dot

echo "scale=3;"$(($t4-$t3))/$(($t2-$t1)) | bc
echo "scale=3;"$size2/$size1 | bc
echo $ret1
echo $ret2

diff $oname.0 $oname.1
