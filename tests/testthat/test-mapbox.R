context('MapBox Vector Tiles')

test_that("Read multipolygon", {
  # Original GeoJSON data
  campus <- sf::read_sf('../testdata/campus.geojson', quiet = TRUE, as_tibble = FALSE)

  # Converted to MVT
  mvt10 = read_mvt_sf('../testdata/tiles/10/213/388.mvt')
  mvt12 = read_mvt_sf('../testdata/tiles/12/853/1554.mvt')

  # Attributes
  style <- jsonlite::fromJSON(as.character(campus$style))
  expect_equal(style, jsonlite::fromJSON(as.character(mvt10$campus$style)))
  expect_equal(style, jsonlite::fromJSON(as.character(mvt12$campus$style)))

  # Metadata
  all.equal(attributes(mvt10$campus$geometry), attributes(campus$geometry), tol = 1e-6)
  all.equal(attributes(mvt12$campus$geometry), attributes(campus$geometry), tol = 1e-8)

  # Convert to EPSG:3857 to work with st_area() without lwgeom
  out1 <- sf::st_transform(campus, 3857)
  out2 <- sf::st_transform(mvt10$campus, 3857)
  out3 <- sf::st_transform(mvt12$campus, 3857)

  # Compare geometry data (zoom reduces accuracy)
  expect_equal(sf::st_area(out1), sf::st_area(out2), tol = 1e-2)
  expect_equal(sf::st_area(out1), sf::st_area(out3), tol = 1e-2)
})
