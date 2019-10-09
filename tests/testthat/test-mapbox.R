context('MapBox Vector Tiles')

# Original GeoJSON data
campus <- sf::read_sf('../testdata/campus.geojson', quiet = TRUE, as_tibble = FALSE)

test_that("Read multipolygon", {
  # Converted to MVT
  mvt10 = read_mvt_sf('../testdata/campus/10/213/388.mvt')
  mvt12 = read_mvt_sf('../testdata/campus/12/853/1554.mvt')

  # Attributes
  style <- jsonlite::fromJSON(as.character(campus$style))
  expect_equal(style, jsonlite::fromJSON(as.character(mvt10$campus$style)))
  expect_equal(style, jsonlite::fromJSON(as.character(mvt12$campus$style)))

  # Metadata
  all.equal(attributes(mvt10$campus$geometry), attributes(campus$geometry), tol = 1e-6)
  all.equal(attributes(mvt12$campus$geometry), attributes(campus$geometry), tol = 1e-8)

  # Convert to EPSG:3857 to test st_area() without lwgeom
  wgs0 <- sf::st_transform(campus, 3857)
  wgs10 <- sf::st_transform(mvt10$campus, 3857)
  wgs12 <- sf::st_transform(mvt12$campus, 3857)

  # Compare geometry data (zoom reduces accuracy)
  expect_equal(sf::st_area(wgs0), sf::st_area(wgs10), tol = 1e-2)
  expect_equal(sf::st_area(wgs0), sf::st_area(wgs12), tol = 1e-2)
})

test_that("Read multilinestring", {
  # Original GeoJSON data
  boundary <- sf::st_boundary(campus)

  # Read from MVT
  mvt10 <- read_mvt_sf('../testdata/boundary/10/213/388.mvt')
  mvt12 <- read_mvt_sf('../testdata/boundary/12/853/1554.mvt')

  # Metadata
  expect_equal(attr(mvt10$boundary$geometry, 'bbox'), attr(boundary$geometry, 'bbox'), tol = 1e-6)
  expect_equal(attr(mvt12$boundary$geometry, 'bbox'), attr(boundary$geometry, 'bbox'), tol = 1e-8)

  # Convert to EPSG:3857 to test st_length() without lwgeom
  wgs0 <- sf::st_transform(boundary, 3857)
  wgs10 <- sf::st_transform(mvt10$boundary, 3857)
  wgs12 <- sf::st_transform(mvt12$boundary, 3857)

  # Compare geometry data (zoom reduces accuracy)
  expect_equal(sf::st_length(wgs0), sf::st_length(wgs10), tol = 1e-4)
  expect_equal(sf::st_length(wgs0), sf::st_length(wgs12), tol = 1e-5)
})
