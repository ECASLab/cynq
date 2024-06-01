#
# See LICENSE for more information about licensing
#  Copyright 2023-2024
#
# Author: Diego Arturo Avila Torres <dandida95@gmail.com>
#         Luis G. Leon Vega <luis.leon@ieee.org>
#

sources += [
  files('accelerator.cpp'),
  files('datamover.cpp'),
  files('execution-graph.cpp'),
  files('hardware.cpp'),
  files('memory.cpp'),
]

# Detect the dependencies
# XRT Dependency is a must in the meantime. It will be optional
xrt_dep = dependency('xrt', required: true)
uuid_dep = cc.find_library('uuid')

project_deps += [xrt_dep, uuid_dep]

subdir('alveo')
subdir('ultrascale')
subdir('execution-graph')
subdir('dma')
subdir('mmio')
subdir('xrt')
