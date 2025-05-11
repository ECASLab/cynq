#!/usr/local/share/pynq-venv/bin/python3

from pynq import Overlay
import os
os.environ["XILINX_XRT"] = "/usr"

ol = Overlay("./examples/zynq-mpsoc/reckon/pynqKria.bit")
exit(0)
