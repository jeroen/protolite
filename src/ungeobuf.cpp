#include "geobuf.pb.h"
#include <Rcpp.h>

//shothands
typedef geobuf::Data Data;
typedef Data::Feature Feature;
typedef Data::FeatureCollection FeatureCollection;
typedef Data::Value Value;
typedef Data::Geometry Geometry;
typedef Rcpp::List List;
typedef Rcpp::NumericVector NumericVector;

static uint32_t dim = 2;
static double multiplier = 1000000;
static std::vector<std::string> keys;

NumericVector build_one(Geometry x){
  NumericVector out;
  for (int i = 0; i < x.coords_size(); i++){
    out.push_back(x.coords(i) / multiplier);
  }
  return out;
}

List build_two(Geometry x){
  Rcpp::List coordinates;
  std::vector<double> vec(dim);
  for (size_t i = 0; i < x.coords_size() / dim; i++){
    for (size_t j = 0; j < dim; j++){
      vec[j] += x.coords(i * dim + j) / multiplier;
    }
    coordinates.push_back(NumericVector(Rcpp::wrap(vec)));
  }

  //Polygon must be closed
  if(x.type() == geobuf::Data_Geometry_Type_POLYGON){
    for (size_t j = 0; j < dim; j++){
      //note: = instead of +=
      vec[j] = x.coords(j) / multiplier;
    }
    coordinates.push_back(NumericVector(Rcpp::wrap(vec)));
  }
  return coordinates;
}

List build_three(Geometry x){
  List out;
  if(!x.lengths_size()){
    out.push_back(build_two(x));
  } else {
    size_t groups = x.lengths_size();
    size_t offset = 0;
    for (size_t i = 0; i < groups; i++){
      size_t groupsize = x.lengths(i);
      Rcpp::List group;
      std::vector<double> vec(dim);
      for (size_t j = 0; j < groupsize; j++){
        for (size_t k = 0; k < dim; k++){
          vec[k] += x.coords((offset + j) * dim + k) / multiplier;
        }
        group.push_back(NumericVector(Rcpp::wrap(vec)));
      }
      //Polygon must be closed
      if(x.type() == geobuf::Data_Geometry_Type_POLYGON){
        for (size_t k = 0; k < dim; k++){
          //note: = instead of +=
          vec[k] = x.coords(offset * dim + k) / multiplier;
        }
        group.push_back(NumericVector(Rcpp::wrap(vec)));
      }
      offset += groupsize;
      out.push_back(group);
    }
  }
  return out;
}

List build_four(Geometry x){
  List out;
  if(!x.lengths_size()){
    out.push_back(build_two(x));
  } else {
    int cursor = 0; //lengths position
    int offset = 0; //coords position
    int sets = x.lengths(0);
    for(int s = 0; s < sets; s++){
      List coordinates;
      size_t groups = x.lengths(++cursor);
      for (size_t i = 0; i < groups; i++){
        R_xlen_t groupsize = x.lengths(++cursor);
        Rcpp::List group;
        std::vector<double> vec(dim);
        for (R_xlen_t j = 0; j < groupsize; j++){
          for (size_t k = 0; k < dim; k++){
            vec[k] += x.coords((offset + j) * dim + k) / multiplier;
          }
          group.push_back(NumericVector(Rcpp::wrap(vec)));
        }
        //Polygon must be closed
        if(x.type() == geobuf::Data_Geometry_Type_MULTIPOLYGON){
          for (size_t k = 0; k < dim; k++){
            vec[k] = x.coords(offset * dim + k) / multiplier;
          }
          group.push_back(NumericVector(Rcpp::wrap(vec)));
        }
        offset += groupsize;
        coordinates.push_back(group);
      }
      out.push_back(coordinates);
    }
  }
  return out;
}

std::string ungeo(Geometry::Type x){
  switch(x){
  case geobuf::Data_Geometry_Type_GEOMETRYCOLLECTION: return "GeometryCollection";
  case geobuf::Data_Geometry_Type_LINESTRING: return "LineString";
  case geobuf::Data_Geometry_Type_MULTILINESTRING: return "MultiLineString";
  case geobuf::Data_Geometry_Type_MULTIPOINT: return "MultiPoint";
  case geobuf::Data_Geometry_Type_MULTIPOLYGON: return "MultiPolygon";
  case geobuf::Data_Geometry_Type_POINT: return "Point";
  case geobuf::Data_Geometry_Type_POLYGON: return "Polygon";
  }
  throw std::runtime_error("switch fall through");
}

List append_prop(List x, uint32_t key, Value val){
  if(key > keys.size())
    throw std::runtime_error("Propety index out of bounds");
  std::string prop(keys.at(key));
  if(val.has_string_value()){
    x[prop] = val.string_value();
  } else if(val.has_double_value()){
    x[prop] = val.double_value();
  } else if(val.has_pos_int_value()){
    int64_t num = val.pos_int_value();
    if(num < pow(2.0, 31)){
      x[prop] = int(num);
    } else {
      x[prop] = double(num);
    }
  } else if(val.has_neg_int_value()){
    int64_t num = val.neg_int_value();
    if(num < pow(2.0, 31)){
      x[prop] = -1 * int(num);
    } else {
      x[prop] = -1 * double(num);
    }
  } else if(val.has_bool_value()){
    x[prop] = (double) val.bool_value();
  } else if(val.has_json_value()){
    Rcpp::Function parse_json = Rcpp::Environment::namespace_env("protolite")["parse_json"];
    x[prop] = parse_json(Rcpp::CharacterVector(val.json_value()));
  } else {
    throw std::runtime_error("Empty property value");
  }
  return x;
}

List ungeo(Geometry x){
  List out;
  out["type"] = ungeo(x.type());
  for(int i = 0; i < x.custom_properties_size() / 2; i++){
    out = append_prop(out, x.custom_properties(i * 2), x.values(i));
  }
  if(x.geometries_size()){
    List geometries;
    for(int i = 0; i < x.geometries_size(); i++){
      geometries.push_back(ungeo(x.geometries(i)));
    }
    out["geometries"] = geometries;
  }
  if(x.coords_size()){
    switch(x.type()){
    case geobuf::Data_Geometry_Type_POINT: out["coordinates"] = build_one(x); break;
    case geobuf::Data_Geometry_Type_LINESTRING: out["coordinates"] = build_two(x); break;
    case geobuf::Data_Geometry_Type_MULTIPOINT: out["coordinates"] = build_two(x); break;
    case geobuf::Data_Geometry_Type_POLYGON: out["coordinates"] = build_three(x); break;
    case geobuf::Data_Geometry_Type_MULTILINESTRING: out["coordinates"] = build_three(x); break;
    case geobuf::Data_Geometry_Type_MULTIPOLYGON: out["coordinates"] = build_four(x); break;
    case geobuf::Data_Geometry_Type_GEOMETRYCOLLECTION: break;
    }
  }
  return out;
}

List ungeo(Feature x){
  List out;
  out["type"] = "Feature";
  if(x.has_geometry())
    out["geometry"] = ungeo(x.geometry());
  if(x.has_id()){
    out["id"] = x.id();
  } else if(x.has_int_id()){
    int64_t val = x.int_id();
    out["id"] = val < pow(2.0, 31) ? int(val) : double(val);
  }
  if(x.properties_size()){
    List props;
    for(int i = 0; i < x.properties_size()/2; i++)
      props = append_prop(props, x.properties(i * 2), x.values(i));
    out["properties"] = props;
  }
  for(int i = 0; i < x.custom_properties_size()/2; i++){
    out = append_prop(out, x.custom_properties(i * 2), x.values(x.properties_size()/2 + i));
  }
  return out;
}

List ungeo(FeatureCollection x){
  List out;
  List features;

  for(int i = 0; i < x.features_size(); i++){
    features.push_back(ungeo(x.features(i)));
  }
  out["type"] = "FeatureCollection";
  out["features"] = features;
  for(int i = 0; i < x.custom_properties_size() / 2; i++){
    out = append_prop(out, x.custom_properties(i * 2), x.values(i));
  }
  return out;
}

// [[Rcpp::export]]
List cpp_unserialize_geobuf(Rcpp::RawVector x){
  geobuf::Data message;
  if(!message.ParseFromArray(x.begin(), x.size()))
    throw std::runtime_error("Failed to parse geobuf proto message");
  dim = message.dimensions();
  multiplier = pow(10.0, message.precision());
  keys.clear();
  for(int i = 0; i < message.keys_size(); i++){
    keys.push_back(message.keys(i));
  }
  List out;
  if(message.has_feature_collection()){
    out = ungeo(message.feature_collection());
  } else if(message.has_feature()){
    out = ungeo(message.feature());
  } else if(message.has_geometry()){
    out = ungeo(message.geometry());
  } else {
    throw std::runtime_error("No 'data_type' field set");
  }
  out.attr("precision") = message.precision();
  return out;
}

