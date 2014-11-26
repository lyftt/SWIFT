#!/bin/bash
fname=compress
iname=compress.in.Z
build_dir=~/SWIFT/Debug+Asserts/lib

rm *.bc *.s *.dot *.exe

clang -emit-llvm -o $fname.bc -c $fname.c || { echo "Failed to emit llvm bc"; exit 1; }
size1=$(stat -c %s $fname.bc)
llc $fname.bc -o $fname.s
g++ -o $fname.exe $fname.s
t1=$(date +%s%N)
./$fname.exe < $iname
t2=$(date +%s%N)

opt -gvn < $fname.bc > $fname.opt.bc || { echo "Failed to opt-load"; exit 1; }

#opt -dot-cfg $fname.opt.bc
#mv cfg.main.dot non-swift-cfg.dot
#dot -Tpdf non-swift-cfg.dot -o $fname-non-swift-cfg.pdf

opt -load $build_dir/swift.so -swift < $fname.opt.bc > $fname.swift.bc || { echo "Failed to opt-load"; exit 1; }
size2=$(stat -c %s $fname.swift.bc)

llc $fname.swift.bc -o $fname.swift.s
g++ -o $fname.swift.exe $fname.swift.s
t3=$(date +%s%N)
./$fname.swift.exe < $iname
ret=$?
t4=$(date +%s%N)

#opt -dot-cfg $fname.swift.bc
#mv cfg.main.dot swift-cfg.dot
#dot -Tpdf swift-cfg.dot -o $fname-swift-cfg.pdf

rm *.bc *.s *.dot *.exe

echo "scale=3;"$(($t4-$t3))/$(($t2-$t1)) | bc
echo "scale=3;"$size2/$size1 | bc
echo $ret
