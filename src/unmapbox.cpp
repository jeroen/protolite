#include "mapbox.pb.h"
#include <Rcpp.h>

//shothands
typedef vector_tile::Tile Tile;
typedef Tile::Value Value;
typedef Tile::GeomType GeomType;
typedef Tile::Feature Feature;
typedef Tile::Layer Layer;
typedef Rcpp::List List;
typedef Rcpp::NumericVector NumericVector;

#define MoveTo 1
#define LineTo 2
#define ClosePath 7

#define cmd_command(CommandInteger) (CommandInteger & 0x7)
#define cmd_count(CommandInteger) (CommandInteger >> 3)

static Rcpp::IntegerMatrix decode_geometry(std::vector<int> geom){
  int x = 0;
  int y = 0;
  int g = 0;
  int x0 = 0;
  int y0 = 0;
  std::vector<int> xvec;
  std::vector<int> yvec;
  std::vector<int> gvec;
  for(int i = 0; i < geom.size(); i++){
    int cmd = cmd_command(geom.at(i));
    int count = cmd_count(geom.at(i));
    REprintf("Command: %d with count %d\n", cmd, count);
    if(cmd == LineTo || cmd == MoveTo){
      for(int j = 0; j < count; j++){
        x = x + geom.at(++i);
        y = y + geom.at(++i);
        xvec.push_back(x);
        yvec.push_back(y);
        if(cmd == MoveTo){
          g++;
          x0 = x;
          y0 = y;
        }
        gvec.push_back(g);
      }
    } else if(cmd == ClosePath){
      xvec.push_back(x0);
      yvec.push_back(y0);
      gvec.push_back(g);
    }
  }
  int len = xvec.size();
  Rcpp::IntegerMatrix mat(len, 3);
  for(int i = 0; i < len; i++){
    mat.at(i, 0) = xvec.at(i);
    mat.at(i, 1) = yvec.at(i);
    mat.at(i, 2) = gvec.at(i);
  }
  return mat;
}

List unmapbox(Feature feature, Rcpp::CharacterVector all_keys, Rcpp::List all_values){
  List out;
  out["id"] = feature.id();
  out["type"] = int(feature.type());
  int n_attrib = feature.tags_size() / 2;
  Rcpp::CharacterVector names(n_attrib);
  Rcpp::List attributes(n_attrib);
  for(int i = 0; i < n_attrib; i++){
    int ikey = feature.tags(i * 2);
    int ival = feature.tags(i * 2 + 1);
    names.at(i) = all_keys.at(ikey);
    attributes.at(i) = all_values.at(ival);
  }
  attributes.attr("names") = names;
  out["attributes"] = attributes;

  int n_geometry = feature.geometry_size();
  std::vector<int> geometry(n_geometry);
  for(int i = 0; i < n_geometry; i++){
    geometry[i] = feature.geometry(i);
  }
  out["geometry"] = decode_geometry(geometry);
  return out;
}

List unmapbox(Layer layer){
  List out;
  out["version"] = layer.version();
  out["name"] = layer.name();
  out["extent"] = layer.extent();

  // Keys (strings)
  int n_keys = layer.keys_size();
  Rcpp::CharacterVector keys(n_keys);
  for(int i = 0; i < n_keys; i++){
    keys.at(i) = layer.keys(i);
  }

  // Values (objects)
  int n_values = layer.values_size();
  Rcpp::List values(n_values);
  for(int i = 0; i < n_values; i++){
    Value val = layer.values(i);
    if(val.has_bool_value()){
      values.at(i) = val.bool_value();
    } else if(val.has_double_value()){
      values.at(i) = val.double_value();
    } else if(val.has_float_value()){
      values.at(i) = val.float_value();
    } else if(val.has_int_value()){
      values.at(i) = val.int_value();
    } else if(val.has_sint_value()){
      values.at(i) = val.sint_value();
    } else if(val.has_string_value()){
      values.at(i) = val.string_value();
    } else if(val.has_uint_value()){
      values.at(i) = val.uint_value();
    }
  }

  // Features (objects)
  int n_features = layer.features_size();
  Rcpp::List features(n_features);
  for(int i = 0; i < n_features; i++){
    features.at(i) = unmapbox(layer.features(i), keys, values);
  }
  out["features"] = features;
  return out;
}

// [[Rcpp::export]]
Rcpp::List cpp_unserialize_mvt(Rcpp::RawVector x){
  vector_tile::Tile message;
  if(!message.ParseFromArray(x.begin(), x.size()))
    throw std::runtime_error("Failed to parse geobuf proto message");
  int n = message.layers_size();
  Rcpp::List out(n);
  for(int i = 0; i < n; i++){
    out[i] = unmapbox(message.layers(i));
  }
  return out;
}
