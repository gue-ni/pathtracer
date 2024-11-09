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

![](./doc/results/render_5000s_3b_1731087163.png)

![](./doc/results/render_5000s_3b_1731060148.png)

![](./doc/results/cornell_box_10000s_3b_1730735703.png)

![](./doc/results/bunny_10000s_3b.png)
