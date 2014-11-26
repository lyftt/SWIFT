#!/bin/bash
fname=compress
iname=compress.in.Z
build_dir=~/swift/Release+Asserts/lib

rm *.bc *.s *.dot *.exe

clang -emit-llvm -o $fname.bc -c $fname.c || { echo "Failed to emit llvm bc"; exit 1; }

opt -gvn < $fname.bc > $fname.opt.bc || { echo "Failed to opt-load"; exit 1; }

opt -dot-cfg $fname.opt.bc
mv cfg.main.dot non-swift-cfg.dot
dot -Tpdf non-swift-cfg.dot -o $fname-non-swift-cfg.pdf

opt -load $build_dir/swift.so -swift < $fname.opt.bc > $fname.swift.bc || { echo "Failed to opt-load"; exit 1; }

llc $fname.swift.bc -o $fname.swift.s
g++ -o $fname.swift.exe $fname.swift.s
./$fname.swift.exe < $iname

opt -dot-cfg $fname.swift.bc
mv cfg.main.dot swift-cfg.dot
dot -Tpdf swift-cfg.dot -o $fname-swift-cfg.pdf

rm *.bc *.s *.dot *.exe
