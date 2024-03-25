#
# See LICENSE for more information about licensing
#  Copyright 2023
#
# Author: Diego Arturo Avila Torres <diego.avila@uned.cr>
#         Luis G. Leon Vega <luis.leon@ieee.org>
#
#

# ---------------------------------------------
# Kria examples
# ---------------------------------------------

executable('matrix-multiplication-kria',
  ['zynq-mpsoc/matrix-multiplication.cpp'],
  include_directories: [projectinc],
  cpp_args : cpp_args,
  dependencies : [project_deps, libcynq_dep]
)

executable('xfopencv-filter2d-kria',
  ['zynq-mpsoc/xfopencv-filter2d.cpp'],
  include_directories: [projectinc],
  cpp_args : cpp_args,
  dependencies : [project_deps, libcynq_dep]
)

executable('xfopencv-warp-perspective-kria',
  ['zynq-mpsoc/xfopencv-warp-perspective.cpp'],
  include_directories: [projectinc],
  cpp_args : cpp_args,
  dependencies : [project_deps, libcynq_dep]
)

executable('xfcolor-detect-kria',
  ['zynq-mpsoc/xfcolor-detect.cpp'],
  include_directories: [projectinc],
  cpp_args : cpp_args,
  dependencies : [project_deps, libcynq_dep]
)


executable('matmul-kria',
  ['zynq-mpsoc/matmul.cpp'],
  include_directories: [projectinc],
  cpp_args : cpp_args,
  dependencies : [project_deps, libcynq_dep]
)

# ---------------------------------------------
# Alveo examples
# ---------------------------------------------

executable('vadd-example-alveo',
  ['alveo/vadd-host.cpp'],
  include_directories: [projectinc],
  cpp_args : cpp_args,
  dependencies : [project_deps, libcynq_dep]
)
