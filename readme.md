## Anderson pointer analysis

A simple prototype of anderson pointer analysis.

### Build & run

```shell
  mkdir build
  cd build
  cmake .. # you may need to change the llvm path in `CMakeList.txt`

  # run a simple test
  chmod +x run.sh
  ./run.sh bc/test03.bc -dump-module -dump-cons -node2name -debug-info  2>bc/03.ll
```

for `test03.c`, we have output constraint:

```shell
Constraints 10
%s <- &%s(obj)
%a <- &%a(obj)
%b <- &%b(obj)
%p <- %s
*%p <- %a
%p1 <- %s
*%p1 <- %b
%p2 <- %s
%0 <- *%p2
%f <- %0
```

and points to set:

```shell
%f: { %a(obj), %b(obj),  }
%s: { %s(obj),  }
%a: { %a(obj),  }
%b: { %b(obj),  }
%p: { %s(obj),  }
%p1: { %s(obj),  }
%p2: { %s(obj),  }
%0: { %a(obj), %b(obj),  }
%s(obj): { %a(obj), %b(obj),  }
%a(obj): {  }
%b(obj): {  }
```

### reference

https://github.com/grievejia/andersen

> A really good anderson implementation for study. My implementation is kind of a simplied version. If you just want a prototype, maybe you could have a try~