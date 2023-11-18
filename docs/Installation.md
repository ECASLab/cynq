# Installation

CYNQ has been already tested in the **Xilinx Kria KV260** using Ubuntu 22.04. At the moment, the **XRT library** is the only relevant dependency that is already included in the Certified Ubuntu.

Apart from that, if you want to compile the documentation, you may need the following optional dependencies:

```bash
sudo apt install doxygen graphviz openjdk-17-jre texlive-font-utils
```

Finally, you can start with the compilation process:

```bash
meson builddir -Dbuild-docs=false -Ddeveloper-mode=false
ninja -C builddir
sudo ninja -C builddir install
```

You can switch the "-Dbuild-docs" to `true` if you want to compile the documentation.
