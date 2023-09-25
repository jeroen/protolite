if(!file.exists("../windows/protobuf/include/google/protobuf/descriptor.h")){
  unlink("../windows", recursive = TRUE)
  url <- if(grepl("aarch", R.version$platform)){
    "https://github.com/r-windows/bundles/releases/download/protobuf-21.12/protobuf-21.12-clang-aarch64.tar.xz"
  } else if(grepl("clang", Sys.getenv('R_COMPILED_BY'))){
    "https://github.com/r-windows/bundles/releases/download/protobuf-21.12/protobuf-21.12-clang-x86_64.tar.xz"
  } else if(getRversion() >= "4.3") {
    "https://github.com/r-windows/bundles/releases/download/protobuf-21.12/protobuf-21.12-ucrt-x86_64.tar.xz"
  } else {
    "https://github.com/rwinlib/protobuf/archive/v21.12.tar.gz"
  }
  download.file(url, basename(url), quiet = TRUE)
  dir.create("../windows", showWarnings = FALSE)
  untar(basename(url), exdir = "../windows", tar = 'internal')
  unlink(basename(url))
  setwd("../windows")
  file.rename(list.files(), 'protobuf')
}
