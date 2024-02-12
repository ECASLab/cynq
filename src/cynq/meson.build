#
# See LICENSE for more information about licensing
#  Copyright 2023
#
# Author: Diego Arturo Avila Torres <dandida95@gmail.com>
#         Luis G. Leon Vega <luis.leon@ieee.org>
#

sources += [
  files('accelerator.cpp'),
  files('datamover.cpp'),
  files('hardware.cpp'),
  files('memory.cpp'),
]

# Detect the dependencies
# XRT Dependency is a must in the meantime. It will be optional
xrt_dep = dependency('xrt', required: true)
xrt_inc = include_directories('/usr/include/xrt')

project_deps += xrt_dep
projectinc += xrt_inc

subdir('ultrascale')
subdir('dma')
subdir('mmio')
subdir('xrt')
