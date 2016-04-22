# Generates rexp.pb.cc and rexp.pb.h
# Only need to do this once
protoc rexp.proto --cpp_out=.
