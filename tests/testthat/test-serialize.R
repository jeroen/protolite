context("serializing and unserializing")

test_that("Objects get serialized and unserialized correctly",{
  x <- list(foo=cars, bar=Titanic)
  expect_equal(x, unserialize_pb(serialize_pb(x)))
  expect_equal(x, unserialize_pb(serialize_pb(x, skip_native = TRUE)))

  expect_equal(cars, unserialize_pb(serialize_pb(cars)))
  expect_equal(cars, unserialize_pb(serialize_pb(cars, skip_native = TRUE)))

  expect_equal(iris, unserialize_pb(serialize_pb(iris)))
  expect_equal(iris, unserialize_pb(serialize_pb(iris, skip_native = TRUE)))

  #a bit of everything, copied from jsonlite package
  set.seed('123')
  myobject <- list(
    mynull = NULL,
    mycomplex = lapply(eigen(matrix(-rnorm(9),3)), round, 3),
    mymatrix = round(matrix(rnorm(9), 3),3),
    myint = as.integer(c(1,2,3)),
    mydf = cars,
    mylist = list(foo='bar', 123, NA, NULL, list('test')),
    mylogical = c(TRUE,FALSE,NA),
    mychar = c('foo', NA, 'bar'),
    somemissings = c(1,2,NA,NaN,5, Inf, 7 -Inf, 9, NA),
    myrawvec = charToRaw('This is a test')
  );

  expect_equal(myobject, unserialize_pb(serialize_pb(myobject)))
  expect_equal(myobject, unserialize_pb(serialize_pb(myobject, skip_native = TRUE)))
})

test_that("Native objects get serialized correctly", {
  # Examples from ?glm
  glm_obj <- glm(cyl ~ mpg + factor(carb), data = mtcars, family = poisson())
  anova_obj <- anova(glm_obj)
  summary_obj <- summary(glm_obj)

  # Serialize bunch of stuff
  expect_equal(glm_obj, unserialize_pb(serialize_pb(glm_obj)))
  expect_equal(anova_obj, unserialize_pb(serialize_pb(anova_obj)))
  expect_equal(summary_obj, unserialize_pb(serialize_pb(summary_obj)))
})

