#
# See LICENSE for more information about licensing
#  Copyright 2023
#
# Author: Diego Arturo Avila Torres <dandida95@gmail.com>
#         Luis G. Leon Vega <luis.leon@ieee.org>
#

wget = find_program('wget', required: true, native: true)
unzip = find_program('unzip', required: true, native: true)

res_link = 'https://www.dropbox.com/scl/fo/9qxbbc2h3cc1vhplpi5nu/h?rlkey=mj03e6xulqu3e046gke3e1bvd&dl=1'
res_output = 'resources.zip'

run_command(wget, '-nc', '-O', res_output, res_link, check: false)
run_command(unzip, res_output, check: false)

fmt = '"@0@/@1@"'
matrix_bitstream_location = fmt.format(meson.global_source_root(), files('kria-bitstreams/matrix-multiply/multiply-stream.bit')[0])
default_xclbin_location = fmt.format(meson.global_source_root(), files('kria-bitstreams/default.xclbin')[0])

cpp_args += [
  '-DEXAMPLE_KRIA_BITSTREAM_LOCATION=' + matrix_bitstream_location,
  '-DEXAMPLE_KRIA_DEFAULT_XCLBIN_LOCATION=' + default_xclbin_location,
  '-DEXAMPLE_KRIA_ACCEL_ADDR=0xa0000000',
  '-DEXAMPLE_KRIA_DMA_ADDR=0xa0010000'
]
