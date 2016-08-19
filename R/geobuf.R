#' Geobuf
#'
#' The \href{https://github.com/mapbox/geobuf}{geobuf} format is an optimized
#' binary format for storing \code{geojson} data with protocol buffers. These
#' functions are compatible with the \code{geobuf2json} and \code{json2geobuf}
#' utilities from the geobuf \href{https://www.npmjs.com/package/geobuf}{npm package}.
#'
#' @export
#' @rdname geobuf
#' @name geobuf
#' @param x file path or raw vector with the serialized \code{geobuf.proto} message
#' @param as_data_frame simplify geojson data into data frames
read_geobuf <- function(x, as_data_frame = TRUE){
  if(is.character(x) && file.exists(x)){
    x <- readBin(x, raw(), file.info(x)$size)
  }
  stopifnot(is.raw(x))
  data <- cpp_unserialize_geobuf(x)
  out <- jsonlite:::simplify(data, simplifyDataFrame = as_data_frame, simplifyMatrix = FALSE)

  # add geojson class here?
  structure(out, precision = attr(data, "precision"))
}

#' @export
#' @rdname geobuf
#' @param pretty indent json, see \link[jsonlite:toJSON]{jsonlite::toJSON}
geobuf2json <- function(x, pretty = FALSE){
  out <- read_geobuf(x, as_data_frame = FALSE)
  digits <- max(6, attr(out, "precision"))
  jsonlite::toJSON(out, auto_unbox = TRUE, digits = 6, null = "null", pretty = pretty)
}

#' @export
#' @rdname geobuf
#' @param json a text string with geojson data
#' @param decimals how many decimals (digits behind the dot) to store for numbers
json2geobuf <- function(json, decimals = 6){
  if(is.character(json) && length(json) == 1 && nchar(json) < 1000){
    if(file.exists(json)){
      json <- file(json)
    } else if(grepl("^https?://", json)){
      json <- url(json)
    }
  }
  if(inherits(json, "connection")){
    json <- readLines(json)
  }
  stopifnot(is.character(json))
  json <- paste(json, collapse = "\n")
  stopifnot(jsonlite::validate(json))
  object <- jsonlite::fromJSON(json, simplifyVector = FALSE)
  serialize_geobuf(object, decimals = 6)
}

# Not exported for now
serialize_geobuf <- function(object, decimals){
  stopifnot(is.numeric(decimals))
  cpp_serialize_geobuf(object, decimals)
}

# These wrappers are called from Rcpp!
#' @importFrom jsonlite fromJSON
#' @importFrom jsonlite toJSON
parse_json <- function(x){
  # do not simplify properties to distinguish arrays.
  # everything gets simplified later on in R
  jsonlite::fromJSON(x, simplifyVector = FALSE)
}

make_json <- function(x){
  jsonlite::toJSON(x, auto_unbox = TRUE, always_decimal = TRUE, null = "null")
}
