# protolite

##### *Fast and Simple Object Serialization to Protocol Buffers*

[![Build Status](https://travis-ci.org/jeroenooms/protolite.svg?branch=master)](https://travis-ci.org/jeroenooms/protolite)
[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/jeroenooms/protolite?branch=master&svg=true)](https://ci.appveyor.com/project/jeroenooms/protolite)
[![Coverage Status](https://codecov.io/github/jeroenooms/protolite/coverage.svg?branch=master)](https://codecov.io/github/jeroenooms/protolite?branch=master)
[![CRAN_Status_Badge](http://www.r-pkg.org/badges/version/protolite)](https://cran.r-project.org/package=protolite)
[![CRAN RStudio mirror downloads](http://cranlogs.r-pkg.org/badges/protolite)](https://cran.r-project.org/package=protolite)
[![Github Stars](https://img.shields.io/github/stars/jeroenooms/protolite.svg?style=social&label=Github)](https://github.com/jeroenooms/protolite)

> Optimized, permissively licensed C++ implementations for reading
  and writing protocol-buffers. Currently supports rexp.proto for serializing
  R objects and geobuf.proto for geojson data. This lightweight package is
  complementary to the much larger 'RProtoBuf' package which provides a full
  featured toolkit for working with protocol-buffers in R.

## Serializing R Objects

This small package contains optimized C++ implementations for reading and writing protocol-buffers. Currently it supports [`rexp.proto`](https://github.com/jeroenooms/protolite/blob/master/src/rexp.proto) for serializing R objects and 
[`geobuf.proto`](https://github.com/jeroenooms/protolite/blob/master/src/geobuf.proto) for geojson data. 

The protolite package is much faster than RProtoBuf because it binds directly to [generated C++](https://developers.google.com/protocol-buffers/docs/reference/cpp-generated) code from the `protoc` compiler. RProtoBuf on the other hand uses the more flexible but slower reflection-based interface, which parses the descriptors at runtime. With RProtoBuf you can create new protocol buffers of a schema, read in arbitrary .proto files, manipulate fields, and generate / parse .prototext ascii format protocol buffers. For more details have a look at our paper: [*RProtoBuf: Efficient Cross-Language Data Serialization in R*](http://arxiv.org/abs/1401.7372).

## Serializing R objects

```r
# Serialize and unserialize an object
library(protolite)
buf <- serialize_pb(iris)
out <- unserialize_pb(buf)
stopifnot(identical(iris, out))

# Fully compatible with RProtoBuf
buf <- RProtoBuf::serialize_pb(iris, NULL)
out <- protolite::unserialize_pb(buf)
stopifnot(identical(iris, out))

# Other way around
buf <- protolite::serialize_pb(mtcars, NULL)
out <- RProtoBuf::unserialize_pb(buf)
stopifnot(identical(mtcars, out))

```

## Converting between GeoJSON and GeoBuf

Use the [countries.geo.json](https://github.com/johan/world.geo.json/blob/master/countries.geo.json) example data:

```r
# Example data
download.file("https://github.com/johan/world.geo.json/raw/master/countries.geo.json",
  "countries.geo.json")

# Convert geojson to geobuf
buf <- json2geobuf("countries.geo.json")
writeBin(buf, "countries.buf")

# The other way around
geobuf2json(buf) #either in memory
geobuf2json("countries.buf") #or from disk

# Read directly from geobuf
mydata <- unserialize_geobuf("countries.buf")
```


## Installation

Binary packages for __OS-X__ or __Windows__ can be installed directly from CRAN:

```r
install.packages("protolite")
```

Installation from source on Linux or OSX requires Google's [Protocol Buffers](https://developers.google.com/protocol-buffers/) library. On __Debian or Ubuntu__ install [libprotobuf-dev](https://packages.debian.org/testing/libprotobuf-dev) and [protobuf-compiler](https://packages.debian.org/testing/protobuf-compiler):

```
sudo apt-get install -y libprotobuf-dev protobuf-compiler
```

On __Fedora__ we need [protobuf-devel](https://apps.fedoraproject.org/packages/protobuf-devel):

```
sudo yum install protobuf-devel
````

On __CentOS / RHEL__ we install [protobuf-devel](https://apps.fedoraproject.org/packages/protobuf-devel) via EPEL:

```
sudo yum install epel-release
sudo yum install protobuf-devel
```

On __OS-X__ use [protobuf](https://github.com/Homebrew/homebrew-core/blob/master/Formula/protobuf.rb) from Homebrew:

```
brew install protobuf
```
