mkdir bc
file_dir="test"
cd $file_dir
cfiles=$(ls .)
for file in $cfiles; do
    prefix=${file:0:6}
    bc_file="$prefix.bc"
    clang -emit-llvm -c -O0 -g3 $file -o $bc_file
    # opt -mem2reg $bc_file -o $bc_file
    # clang -S -emit-llvm $file -o /dev/stdout | opt -S -mem2reg -o example1-opt.ll
done
cd ..
mv $file_dir/*.bc bc
# mv $file_dir/*.ll bc