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

executable('ad08-sequential-kria',
  ['zynq-mpsoc/ad08-sequential.cpp'],
  include_directories: [projectinc],
  cpp_args : cpp_args,
  dependencies : [project_deps, libcynq_dep]
)

executable('ad08-streams-kria',
  ['zynq-mpsoc/ad08-streams.cpp'],
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

# ---------------------------------------------
# Structure examples
# ---------------------------------------------

executable('execution-stream',
  ['structures/execution-stream.cpp'],
  include_directories: [projectinc],
  cpp_args : cpp_args,
  dependencies : [project_deps, libcynq_dep]
)
