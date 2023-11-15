#
# See LICENSE for more information about licensing
#  Copyright 2023
#
# Author: Diego Arturo Avila Torres <dandida95@gmail.com>
#         Luis G. Leon Vega <luis.leon@ieee.org>
#

fmt = '"@0@/@1@"'
matrix_bitstream_location = fmt.format(meson.global_source_root(), files('matrix-multiply/multiply-stream.bit')[0])
default_xclbin_location = fmt.format(meson.global_source_root(), files('default.xclbin')[0])

cpp_args += [
  '-DEXAMPLE_BITSTREAM_LOCATION=' + matrix_bitstream_location,
  '-DEXAMPLE_DEFAULT_XCLBIN_LOCATION=' + default_xclbin_location
]
