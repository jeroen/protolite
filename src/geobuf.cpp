#include "geobuf.pb.h"
#include <Rcpp.h>

//shothands
typedef geobuf::Data Data;
typedef Data::Feature Feature;
typedef Data::FeatureCollection FeatureCollection;
typedef Data::Value Value;
typedef Data::Geometry Geometry;
typedef Geometry::Type Type;
typedef Rcpp::List List;
typedef Rcpp::NumericVector NumericVector;
typedef Rcpp::RawVector RawVector;

static uint32_t dim = 0;
static double multiplier = 1000000;
static std::vector<std::string> keys;

Type geo(std::string type){
  std::transform(type.begin(), type.end(), type.begin(), ::toupper);
  if(!type.compare("POINT"))
    return geobuf::Data_Geometry_Type_POINT;
  if(!type.compare("MULTIPOINT"))
    return geobuf::Data_Geometry_Type_MULTIPOINT;
  if(!type.compare("LINESTRING"))
    return geobuf::Data_Geometry_Type_LINESTRING;
  if(!type.compare("MULTILINESTRING"))
    return geobuf::Data_Geometry_Type_MULTILINESTRING;
  if(!type.compare("POLYGON"))
    return geobuf::Data_Geometry_Type_POLYGON;
  if(!type.compare("MULTIPOLYGON"))
    return geobuf::Data_Geometry_Type_MULTIPOLYGON;
  if(!type.compare("GEOMETRYCOLLECTION"))
    return geobuf::Data_Geometry_Type_GEOMETRYCOLLECTION;
  throw std::runtime_error("Unsupported TYPE: " + type);
}

Value make_value(Rcpp::RObject x){
  Value out;
  if(LENGTH(x) == 1){
    if(TYPEOF(x) == LGLSXP){
      out.set_bool_value(Rcpp::LogicalVector(x).at(0));
      return out;
    } else if(TYPEOF(x) == INTSXP){
      int val = Rcpp::IntegerVector(x).at(0);
      (val < 0) ? out.set_neg_int_value(-val) : out.set_pos_int_value(val);
      return out;
    } else if(TYPEOF(x) == STRSXP){
      out.set_string_value(Rcpp::String(x).get_cstring());
      return out;
    } else if(TYPEOF(x) == REALSXP){
      out.set_double_value(Rcpp::NumericVector(x).at(0));
      return out;
    }
  }
  //default is to use JSON
  Rcpp::Function make_json = Rcpp::Environment::namespace_env("protolite")["make_json"];
  Rcpp::CharacterVector json = make_json(x);
  out.set_json_value(json.at(0));
  return out;
}

int find_key(std::string name){
  size_t pos = find(keys.begin(), keys.end(), name) - keys.begin();
  if(pos >= keys.size()) {
    pos = keys.size();
    keys.push_back(name);
  }
  return pos;
}

Geometry coords_one(List x, Geometry out){
  dim = x.size();
  for(size_t i = 0; i < dim; i++){
    Rcpp::NumericVector y = x[i];
    out.add_coords(round(y(0) * multiplier));
  }
  return out;
}

Geometry coords_two(List x, Geometry out, bool closed = false){
  int points = x.size();
  std::vector<double> vec(dim);
  for(int i = 0; i < std::max(0, points-closed); i++){
    List values = x[i];
    if(dim == 0){
      dim = values.size();
      vec.resize(dim);
    } else {
      if(dim != values.size())
        throw std::runtime_error("Unequal coordinate dimensions");
    }
    for(size_t j = 0; j < dim; j++){
      Rcpp::NumericVector y = values[j];
      double val = y(0) * multiplier;
      out.add_coords(round(val - vec[j]));
      vec[j] = val;
    }
  }
  return out;
}

Geometry coords_three(List x, Geometry out, bool closed = false){
  int groups = x.size();
  for(int i = 0; i < groups; i++){
    List group = x[i];
    out = coords_two(group, out, closed);
    out.add_lengths(group.size() - closed);
  }
  return out;
}

Geometry coords_four(List x, Geometry out, bool closed = false){
  int sets = x.size();
  out.add_lengths(sets);
  for(int i = 0; i < sets; i++){
    List set = x[i];
    out.add_lengths(set.size());
    out = coords_three(set, out, closed);
  }
  return out;
}

Geometry parse_geometry(List x){
  Geometry out;
  if(!x.containsElementNamed("type"))
    throw std::runtime_error("Geometry does not have a type");
  out.set_type(geo(x["type"]));
  Rcpp::CharacterVector names = x.names();
  for(int i = 0; i < x.length(); i++){
    std::string key(names.at(i));
    if(!key.compare("type") || !key.compare("coordinates") || !key.compare("geometries"))
      continue;
    out.add_custom_properties(find_key(key));
    out.add_custom_properties(i);
    out.add_values()->CopyFrom(make_value(x[i]));
  }
  if(out.type() == geobuf::Data_Geometry_Type_GEOMETRYCOLLECTION){
    if(!x.containsElementNamed("geometries"))
      throw std::runtime_error("GeometryCollection does not contain geometries");
    List geometries = x["geometries"];
    for(int i = 0; i < geometries.length(); i++){
      out.add_geometries()->CopyFrom(parse_geometry(geometries[i]));
    }
    return out;
  } else {
    List coords = x["coordinates"];
    switch(out.type()){
    case geobuf::Data_Geometry_Type_POINT: return coords_one(coords, out);
    case geobuf::Data_Geometry_Type_MULTIPOINT: return coords_two(coords, out);
    case geobuf::Data_Geometry_Type_LINESTRING: return coords_two(coords, out);
    case geobuf::Data_Geometry_Type_MULTILINESTRING: return coords_three(coords, out);
    case geobuf::Data_Geometry_Type_POLYGON: return coords_three(coords, out, true);
    case geobuf::Data_Geometry_Type_MULTIPOLYGON: return coords_four(coords, out, true);
    case geobuf::Data_Geometry_Type_GEOMETRYCOLLECTION: throw std::runtime_error("switch fall through");
    }
  }
  throw std::runtime_error("switch fall through");
}

Feature parse_feature(List x){
  Feature out;
  if(!x.containsElementNamed("geometry"))
    throw std::runtime_error("feature does not contain geometry");
  out.mutable_geometry()->CopyFrom(parse_geometry(x["geometry"]));
  if(x.containsElementNamed("properties")){
    List properties = x["properties"];
    Rcpp::CharacterVector names = properties.names();
    for(int i = 0; i < properties.size(); i++){
      out.add_properties(find_key(std::string(names.at(i))));
      out.add_properties(i); //not sure why
      out.add_values()->CopyFrom(make_value(properties[i]));
    }
  }
  if(x.containsElementNamed("id")){
    if(TYPEOF(x["id"]) == STRSXP){
      Rcpp::CharacterVector str = x["id"];
      out.set_id(str.at(0));
    } else if(TYPEOF(x["id"]) == INTSXP) {
      Rcpp::IntegerVector num = x["id"];
      out.set_int_id(num.at(0));
    } else if(TYPEOF(x["id"]) == REALSXP){
      Rcpp::NumericVector num = x["id"];
      double val = num.at(0);
      if(val == round(val)){
        out.set_int_id(round(val));
      } else {
        throw std::runtime_error("ID has non-integer number");
      }
    } else {
      throw std::runtime_error("ID field must be string o rnumber");
    }
  }
  Rcpp::CharacterVector names = x.names();
  for(int i = 0; i < x.length(); i++){
    std::string key(names.at(i));
    if(!key.compare("geometry") || !key.compare("type") || !key.compare("properties") || !key.compare("id"))
      continue;
    out.add_custom_properties(find_key(key));
    out.add_custom_properties(i);
    out.add_values()->CopyFrom(make_value(x[i]));
  }
  return out;
}

FeatureCollection parse_collection(List x){
  FeatureCollection out;
  if(x.containsElementNamed("features")){
    List features = x["features"];
    for(int i = 0; i < features.length(); i++)
      out.add_features()->CopyFrom(parse_feature(features[i]));
  }
  Rcpp::CharacterVector names = x.names();
  for(int i = 0; i < x.length(); i++){
    std::string key(names.at(i));
    if(!key.compare("features") || !key.compare("type"))
      continue;
    out.add_custom_properties(find_key(key));
    out.add_custom_properties(i);
    out.add_values()->CopyFrom(make_value(x[i]));
  }
  return out;
}

// [[Rcpp::export]]
RawVector cpp_serialize_geobuf(List x, int decimals){
  keys.clear();
  Data message;
  message.set_precision(decimals);
  dim = 0;
  multiplier = pow(10.0, decimals);
  if(!x.containsElementNamed("type"))
    throw std::runtime_error("Data does not have 'type' element");
  std::string type = x["type"];
  std::transform(type.begin(), type.end(), type.begin(), ::toupper);
  if(!type.compare("FEATURECOLLECTION")){
    message.mutable_feature_collection()->CopyFrom(parse_collection(x));
  } else if(!type.compare("FEATURE")){
    message.mutable_feature()->CopyFrom(parse_feature(x));
  } else if(!type.compare("GEOMETRY")){
    message.mutable_geometry()->CopyFrom(parse_geometry(x));
  } else {
    throw std::runtime_error("Unsupported type:" + type);
  }
  message.set_dimensions(dim);
  for(size_t i = 0; i < keys.size(); i++){
    message.add_keys(keys.at(i));
  }
  int size = message.ByteSizeLong();
  RawVector res(size);
  if(!message.SerializeToArray(res.begin(), size))
    throw std::runtime_error("Failed to serialize into geobuf message");
  return res;
}
