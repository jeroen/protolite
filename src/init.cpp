#include <R_ext/Rdynload.h>
#include <google/protobuf/stubs/common.h>

// [[Rcpp::export]]
void R_start_protobuf() {
  // Verify that the version of the library that we linked against is
  // compatible with the version of the headers we compiled against.
  GOOGLE_PROTOBUF_VERIFY_VERSION;
}

/*
void R_unload_protolite(DllInfo *info) {
  //Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();
}
*/
