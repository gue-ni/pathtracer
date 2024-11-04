# Pathtracer

## Build

```bash
mkdir build cd build
cmake ..
cmake --build .
```

## Check Mem Leaks

```bash
valgrind --leak-check=full --log-file=valgrind_report.txt ./pt 2
```

## Results

![](./doc/results/bunny_10000s_3b.png)
