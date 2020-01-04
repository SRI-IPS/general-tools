# Description:
#   FFmpeg is a free software project that produces libraries and programs for handling multimedia
#   data. FFmpeg includes libavcodec, an audio/video codec library used by several other projects,
#   libavformat (Lavf),[6] an audio/video container mux and demux library, and the ffmpeg command
#   line program for transcoding multimedia files. FFmpeg is published under the GNU Lesser General
#   Public License 2.1+ or GNU General Public License 2+ (depending on which options are enabled)

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # GNU Lesser Public License 2.1+ or GPL 2+

cc_library(
    name = "ffmpeg",
    visibility = ["//visibility:public"],
    deps = [
        ":avcodec",
        ":swresample",
    ],
)

cc_library(
    name = "avcodec",
    srcs = ["lib/libavcodec.so"],
)

cc_library(
    name = "swresample",
    srcs = ["lib/libswresample.so"],
)
