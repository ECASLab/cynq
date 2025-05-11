== Steps to execute ==

1. Log as super user:

```bash
sudo su
```

2. Bring up the environment

```bash
source /etc/profile.d/pynq_venv.sh
```

3. Compile as usual (see instructions in the top README)

4. Execute reckon from the top directory

```bash
./examples/zynq-mpsoc/reckon/load_design.py
./builddir/examples/reckon-kria
```
