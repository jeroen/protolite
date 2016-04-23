# protolite

##### *Fast and Simple Object Serialization to Protocol Buffers*

[![Build Status](https://travis-ci.org/jeroenooms/protolite.svg?branch=master)](https://travis-ci.org/jeroenooms/protolite)
[![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/github/jeroenooms/protolite?branch=master&svg=true)](https://ci.appveyor.com/project/jeroenooms/protolite)
[![Coverage Status](https://codecov.io/github/jeroenooms/protolite/coverage.svg?branch=master)](https://codecov.io/github/jeroenooms/protolite?branch=master)
[![CRAN_Status_Badge](http://www.r-pkg.org/badges/version/protolite)](http://cran.r-project.org/package=protolite)
[![CRAN RStudio mirror downloads](http://cranlogs.r-pkg.org/badges/protolite)](http://cran.r-project.org/package=protolite)
[![Github Stars](https://img.shields.io/github/stars/jeroenooms/protolite.svg?style=social&label=Github)](https://github.com/jeroenooms/protolite)

> High-performance C++ reimplementation of the object serialization
  functionality from the RProtoBuf package. Permissively licensed, fully
  compatible, no bloat.

## Hello World

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
