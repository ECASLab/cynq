## Examples

Please, take the following commands for executing the examples

This readme assumes that you are in the root directory of the repo.

### Kria

Matrix Multiplication:

```bash
sudo ./builddir/examples/matrix-multiplication-kria
```

Warp Perspective:

```bash
IMG_PATH=examples/misc/1280x720.png
sudo ./builddir/examples/xfopencv-warp-perspective-kria ${IMG_PATH}
```

Filter2D:

```bash
IMG_PATH=examples/misc/1280x720.png
sudo ./builddir/examples/xfopencv-filter2d-kria ${IMG_PATH}
```

Ad08:

```bash
A_ROWS=4
B_COLS=4
C_COLS=4
sudo ./builddir/examples/ad08-streams-kria ${A_ROWS} ${B_COLS} ${C_COLS} 
sudo ./builddir/examples/ad08-sequential-kria ${A_ROWS} ${B_COLS} ${C_COLS} 
```

### Alveo Card

Vadd:

```bash
./builddir/vadd-example-alveo
# For custom xclbin
XCLBIN_PATH=third-party/resources/alveo-xclbin/vadd/vadd.xclbin
./builddir/examples/vadd-example-alveo ${XCLBIN_PATH}
```
