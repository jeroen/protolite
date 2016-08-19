context("roundtrip geobuf to json")

test_that("roundtrip geobuf data",{
  buf <- readBin("test.pb", raw(), file.info("test.pb")$size)
  data <- read_geobuf(buf, as_data_frame = FALSE)
  out <- serialize_geobuf(data, decimals = attr(data, "precision"))
  data2 <- read_geobuf(out, as_data_frame = FALSE)
  expect_equal(data, data2)
})

test_that("geobuf2json output is identical to NPM",{
  geobuf <- fromJSON(geobuf2json("test.pb"), simplifyVector = FALSE)
  geojson <- fromJSON("test.json", simplifyVector = FALSE)
  expect_equal(geobuf, geojson)
})
