#' Mapbox Vector Tiles
#'
#' Read Mapbox vector-tile (mvt) files and returns the list of layers.
#'
#' @export
#' @name mapbox
#' @rdname mapbox
#' @param data url, path or raw vector with the mvt data
#' @param zxy vector of length 3 with respsectively z (zoom), x (column) and y (row).
#' For file/url in the standard `../{z}/{x}/{y}.mvt` format, these are automatically
#' inferred from the input path.
#' @param as_latlon return the data as lat/lon instead of raw EPSG:3857 positions
read_mvt_data <- function(data, as_latlon = TRUE, zxy = NULL){
  if(!is.numeric(zxy) || length(zxy) != 3){
    zxy <- parse_mvt_params(data)
  }
  z <- zxy[1]
  x <- zxy[2]
  y <- zxy[3]
  if(is.character(data)){
    data <- if(grepl('^https?://', data)){
      curl::curl_fetch_memory(data, handle = curl::new_handle(failonerror = TRUE))$content
    } else {
      readBin(normalizePath(data, mustWork = TRUE), raw(), file.info(data)$size)
    }
  }
  stopifnot(is.raw(data))
  layers <- cpp_unserialize_mvt(data)
  lapply(layers, function(layer){
    layer$features <- lapply(layer$features, function(feature){
      if(isTRUE(as_latlon)){
        feature$geometry[,1] <- x_to_lon((x + feature$geometry[,1]) / 2^z)
        feature$geometry[,2] <- y_to_lat((y + feature$geometry[,2]) / 2^z)
      } else {
        feature$geometry[,1] <- scale_wsg((x + feature$geometry[,1]) / 2^z)
        feature$geometry[,2] <- -1 * scale_wsg((y + feature$geometry[,2]) / 2^z)
      }
      return(feature)
    })
    return(layer)
  })
}

parse_mvt_params <- function(url){
  url <- sub("\\#.*", "", url)
  url <- sub("\\?.*", "", url)
  regex <- "^.*[-/]([0-9]+)[-/]([0-9]+)[-/]([0-9]+)\\.[^/]*$"
  if(!is.character(url) || !length(url) || !grepl(regex, url)){
    stop("Input path does have standard /{z}/{x}/{y}.mvt format so you must provide zxy parameters manually")
  }
  z <- sub(regex, "\\1", url)
  x <- sub(regex, "\\2", url)
  y <- sub(regex, "\\3", url)
  as.integer(c(z,x,y))
}

#' @export
#' @rdname mapbox
#' @param crs desired output coordinate system (passed to [sf::st_transform]).
#' Note that mvt input is always by definition 3857.
read_mvt_sf <- function(data, crs = 4326, zxy = NULL){
  layers <- read_mvt_data(data, zxy = zxy, as_latlon = FALSE)
  collections <- lapply(layers, function(layer){
    geometry <- sf::st_sfc(lapply(layer$features, function(feature){
      switch(feature$type,
             POINT = mvt_sf_point(feature$geometry),
             LINESTRING = mvt_sf_linestring(feature$geometry),
             POLYGON = mvt_sf_polygon(feature$geometry),
             UNKNOWN = feature$geometry,
             stop("Unknown type:", feature$type)
      )
    }), crs = 3857)
    geometry <- sf::st_transform(geometry, crs = crs)
    out <- if(!length(layer$keys)){
      sf::st_sf(geometry)
    } else {
      df <- lapply(layer$keys, function(key){
        sapply(layer$features, function(feature){
          val <- feature$attributes[[key]]
          ifelse(length(val), val, NA)
        })
      })
      names(df) <- layer$keys
      sf::st_sf(df, geometry)
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
  if(all_equal(mat[,3])){
    sf::st_linestring(mat[,1:2])
  } else {
    sf::st_multilinestring(split_matrix_groups(mat))
  }
}

#FIXME: calculate winding order to detect MULTIPOLYGONS
mvt_sf_polygon <- function(mat){
  data <- split_matrix_groups(mat)
  isnew <- vapply(data, function(X){
    poly_dir(X[,1], X[,2]) == 1
  }, logical(1))
  group <- cumsum(isnew)
  if(all_equal(group)){
    sf::st_polygon(data)
  } else {
    sf::st_multipolygon(unname(split(data, group)))
  }
}

all_equal <- function(x){
  length(unique(x)) == 1L
}

split_matrix_groups <- function(mat){
  groups <- unname(split(mat[,1:2], mat[,3]))
  lapply(groups, matrix, ncol = 2)
}

x_to_lon <- function(x){
  x * 360 - 180
}

y_to_lat <- function(y){
  atan(sinh(pi - y * 2*pi)) * (180/pi)
}

MAXEXTENT <- 20037508.342789244;

scale_wsg <- function(x){
  x * (2 * MAXEXTENT) - MAXEXTENT
}

poly_dir <- function(x, y) {
  xdiff <- c(x[-1], x[1]) - x
  ysum <- c(y[-1], y[1]) + y
  sign(sum(xdiff * ysum))
}
