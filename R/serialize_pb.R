#' Serialize to Protocol Buffers
#'
#' Serializes R objects to a general purpose protobuf message. It uses the same
#' \href{https://github.com/jeroenooms/protolite/blob/master/src/rexp.proto}{rexp.proto}
#' descriptor and mapping between R objects and protobuf messages as RHIPE and the
#' \link[RProtoBuf:serialize_pb]{RProtoBuf} package.
#'
#' The \code{serialize_pb} and \code{unserialize_pb} reimplement the identically
#' named functions from the \code{RProtoBuf} package in pure \code{C++}. This makes
#' the function faster and simpler, but the output should be identical.
#'
#' @importFrom Rcpp sourceCpp
#' @useDynLib protolite
#' @rdname serialize_pb
#' @export
#' @aliases protolite
#' @param object an R object to serialize
#' @param connection a connection, file, or \code{NULL} for a raw vector
#' @param skip_native do not serialize 'native' (non-data) R objects. Setting to \code{TRUE}
#' will only serialize \emph{data} types (numeric, boolean, string, raw, list). The default
#' behavior is to fall back on base R \code{\link{serialize}} for non-data objects.
#' @param msg raw vector with the serialized \code{rexp.proto} message
#' @examples # Serialize and unserialize an object
#' buf <- serialize_pb(iris)
#' out <- unserialize_pb(buf)
#' stopifnot(identical(iris, out))
#'
#' \dontrun{ #Fully compatible with RProtoBuf
#' buf <- RProtoBuf::serialize_pb(iris, NULL)
#' out <- protolite::unserialize_pb(buf)
#' stopifnot(identical(iris, out))
#'
#' # Other way around
#' buf <- protolite::serialize_pb(mtcars, NULL)
#' out <- RProtoBuf::unserialize_pb(buf)
#' stopifnot(identical(mtcars, out))
#' }
serialize_pb <- function(object, connection = NULL, skip_native = FALSE){
  stopifnot(is.logical(skip_native))
  buf <- cpp_serialize_pb(object, skip_native)
  if(is.null(connection))
    return(buf)
  writeBin(buf, con = connection)
}

#' @export
#' @rdname serialize_pb
unserialize_pb <- function(msg){
  stopifnot(is.raw(msg))
  cpp_unserialize_pb(msg)
}
