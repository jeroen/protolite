/* Generate tiles from GeoJSON data.
 * Sample data from leflet: https://leafletjs.com/examples/geojson/example.html
 * npm install geojson2mvt
 */

var fs = require('fs');
var geojson2mvt = require('geojson2mvt');

const filePath = './boundary.geojson';

var options = {
  rootDir: 'boundary',
  bbox : [39.73,-105.02,39.75,-105.00],
  zoom : {
    min : 8,
    max : 14,
  },
  layerName: 'boundary',
};

var geoJson = JSON.parse(fs.readFileSync(filePath, "utf8"));

// build the static tile pyramid
geojson2mvt(geoJson, options);
