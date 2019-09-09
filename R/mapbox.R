#' Mapbox Vector Tiles
#'
#' Read Mapbox vector-tile (mvt) files and returns the list of layers.
#'
#' @export
#' @rdname mapbox
#' @name mapbox
#' @param x file path or raw vector with the mvt file
read_mvt <- function(x){
  if(is.character(x)){
    x <- readBin(normalizePath(x, mustWork = TRUE), raw(), file.info(x)$size)
  }
  stopifnot(is.raw(x))
  cpp_unserialize_mvt(x)
}
