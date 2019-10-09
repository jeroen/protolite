context('MapBox Vector Tiles')

test_that("Read multipolygon", {
  # Original GeoJSON data
  campus <- sf::read_sf('../testdata/campus.geojson', quiet = TRUE)

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

  # Compare geometry data (zoom reduces accuracy)
  expect_equal(sf::st_area(campus), st_area(mvt10$campus), tol = 1e-2)
  expect_equal(sf::st_area(campus), st_area(mvt12$campus), tol = 1e-2)
})
