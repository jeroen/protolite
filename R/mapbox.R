#' Mapbox Vector Tiles
#'
#' Read Mapbox vector-tile (mvt) files and returns the list of layers.
#'
#' @export
#' @name mapbox
#' @rdname mapbox
#' @name mapbox
#' @param x file path or raw vector with the mvt file
read_mvt_data <- function(x){
  if(is.character(x)){
    x <- readBin(normalizePath(x, mustWork = TRUE), raw(), file.info(x)$size)
  }
  stopifnot(is.raw(x))
  cpp_unserialize_mvt(x)
}


#' @export
#' @rdname mapbox
read_mvt_sf <- function(x){
  layers <- read_mvt_data(x)
  collections <- lapply(layers, function(layer){
    g <- sf::st_sfc(lapply(layer$features, function(feature){
      switch(feature$type,
             POINT = mvt_sf_point(feature$geometry),
             LINESTRING = mvt_sf_linestring(feature$geometry),
             POLYGON = mvt_sf_polygon(feature$geometry),
             UNKNOWN = feature$geometry,
             stop("Unknown type:", feature$type)
      )
    }))
    if(!length(layer$keys)){
      sf::st_sf(g)
    } else {
      df <- lapply(layer$keys, function(key){
        sapply(layer$features, function(feature){
          val <- feature$attributes[[key]]
          ifelse(length(val), val, NA)
        })
      })
      names(df) <- layer$keys
      sf::st_sf(df, g)
    }
  })
  layer_names <- sapply(layers, `[[`, 'name')
  if(length(layer_names) == length(collections))
    names(collections) <- layer_names
  return(collections)
}

mvt_sf_point <- function(mat){
  if(nrow(mat) == 1){
    sf::st_point(mat[1:2])
  } else {
    sf::st_multipoint(mat[,1:2])
  }
}

mvt_sf_linestring <- function(mat){
  if(length(unique(mat)) == 1){
    sf::st_linestring(mat[,1:2])
  } else {
    sf::st_multilinestring(split_matrix_groups(mat))
  }
}

#FIXME: calculate winding order to detect MULTIPOLYGONS
mvt_sf_polygon <- function(mat){
  sf::st_polygon(split_matrix_groups(mat))
}

split_matrix_groups <- function(mat){
  groups <- unname(split(mat[,1:2], mat[,3]))
  lapply(groups, matrix, ncol = 2)
}
