## Anderson pointer analysis

A simple prototype of anderson pointer analysis.

### Build & run

```shell
  mkdir build
  cd build
  cmake .. # you may need to change the llvm path in `CMakeList.txt`

  # run a simple test
  ./run.sh bc/test00.bc -dump-module -dump-cons -node2name -debug-info  2>bc/00.ll
```

output constraint:

```shell
Constraints 12
a.addr <- &a.addr
b.addr <- &b.addr
c <- &c
p <- &p
q <- &q
*p <- a.addr
*q <- b.addr
w.0 <- p
w.0 <- q
*w.0 <- c
f <- w.0
call <- f
```