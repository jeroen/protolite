#include "rexp.pb.h"
#include <Rcpp.h>

//using namespace Rcpp;

rexp::REXP rexp_real(Rcpp::NumericVector x){
  rexp::REXP out;
  out.set_rclass(rexp::REXP_RClass_REAL);
  for(int i = 0; i < x.length(); i++){
    Rprintf("adding double %f at index %d\n", x[i], i);
    out.add_realvalue(x[i]);
  }
  return out;
}

rexp::REXP rexp_int(Rcpp::IntegerVector x){
  rexp::REXP out;
  out.set_rclass(rexp::REXP_RClass_INTEGER);
  for(int i = 0; i < x.length(); i++)
    out.add_intvalue(x[i]);
  return out;
}

rexp::REXP rexp_bool(Rcpp::LogicalVector x){
  rexp::REXP out;
  out.set_rclass(rexp::REXP_RClass_LOGICAL);
  for(int i = 0; i < x.length(); i++){
    rexp::REXP_RBOOLEAN val = Rcpp::LogicalVector::is_na(x[i]) ? rexp::REXP_RBOOLEAN_NA :
      (x[i] ? rexp::REXP_RBOOLEAN_T : rexp::REXP_RBOOLEAN_F);
    out.add_booleanvalue(val);
  }
  return out;
}

rexp::REXP rexp_string(Rcpp::StringVector x){
  rexp::REXP out;
  out.set_rclass(rexp::REXP_RClass_STRING);
  for(int i = 0; i < x.length(); i++){
    rexp::STRING *val = out.add_stringvalue();
    if(Rcpp::StringVector::is_na(x[i])){
      val->set_isna(true);
    } else {
      //always store as utf8
      val->set_isna(false);
      Rcpp::String str = x[i];
      str.set_encoding(CE_UTF8);
      val->set_strval(str.get_cstring());
    }
  }
  return out;
}

rexp::REXP rexp_raw(Rcpp::RawVector x){
  rexp::REXP out;
  out.set_rclass(rexp::REXP_RClass_RAW);
  out.set_rawvalue(x.begin(), x.length());
  return out;
}

rexp::REXP rexp_native(Rcpp::RObject x, bool skip_native){
  rexp::REXP out;
  out.set_rclass(rexp::REXP_RClass_NATIVE);

  if(skip_native)
    return out;

  /* Solution below looks nicer but Rcpp has bug for 'call' objects
    Rcpp::Function serialize = Rcpp::Environment::namespace_env("base")["serialize"];
    Rcpp::RawVector buf = serialize(x, R_NilValue);
  */
  Rcpp::Environment env;
  env["MY_R_OBJECT"] = x;
  Rcpp::ExpressionVector expr("serialize(MY_R_OBJECT, NULL)");
  Rcpp::RawVector buf = Rcpp::Rcpp_eval(expr, env);
  out.set_nativevalue(buf.begin(), buf.length());
  return out;
}

rexp::REXP rexp_complex(Rcpp::ComplexVector x){
  rexp::REXP out;
  out.set_rclass(rexp::REXP_RClass_COMPLEX);
  for(int i = 0; i < x.length(); i++){
    rexp::CMPLX *val = out.add_complexvalue();
    val->set_real(x[i].r);
    val->set_imag(x[i].i);
  }
  return out;
}

//needed by rexp_list
rexp::REXP rexp_object(Rcpp::RObject x, bool skip_native);

rexp::REXP rexp_list(Rcpp::List x, bool skip_native){
  rexp::REXP out;
  out.set_rclass(rexp::REXP_RClass_LIST);
  for(int i = 0; i < x.length(); i++){
    rexp::REXP obj = rexp_object(x[i], skip_native);
    out.add_rexpvalue()->CopyFrom(obj);
  }
  return out;
}

rexp::REXP rexp_null(){
  rexp::REXP out;
  out.set_rclass(rexp::REXP_RClass_NULLTYPE);
  return out;
}

// see also
// http://gallery.rcpp.org/articles/rcpp-wrap-and-recurse/
// http://statr.me/rcpp-note/api/RObject.html
// can we do this with RObject instead of SEXP ?
rexp::REXP rexp_any(Rcpp::RObject x, bool skip_native){
  switch(TYPEOF(x)){
    case NILSXP: return rexp_null();
    case LGLSXP: return rexp_bool(Rcpp::as<Rcpp::LogicalVector>(x));
    case INTSXP: return rexp_int(Rcpp::as<Rcpp::IntegerVector>(x));
    case REALSXP: return rexp_real(Rcpp::as<Rcpp::NumericVector>(x));
    case CPLXSXP: return rexp_complex(Rcpp::as<Rcpp::ComplexVector>(x));
    case STRSXP: return rexp_string(Rcpp::as<Rcpp::StringVector>(x));
    case VECSXP: return rexp_list(Rcpp::as<Rcpp::List>(x), skip_native);
    case RAWSXP: return rexp_raw(Rcpp::as<Rcpp::RawVector>(x));
    default: return rexp_native(x, skip_native);
  }
}

rexp::REXP rexp_object(Rcpp::RObject x, bool skip_native){
  rexp::REXP out = rexp_any(x, skip_native);
  if(out.rclass() != rexp::REXP_RClass_NATIVE){
    std::vector< std::string > attr_names = x.attributeNames();
    int len = attr_names.size();
    for(int i = 0; i < len; i++) {
      std::string name = attr_names[i];
      rexp::REXP attr = rexp_object(x.attr(name), skip_native);
      out.add_attrname()->assign(name);
      out.add_attrvalue()->CopyFrom(attr);
    }
  }
  return out;
}

// [[Rcpp::export]]
Rcpp::RawVector cpp_serialize_pb(Rcpp::RObject x, bool skip_native){
  rexp::REXP message = rexp_object(x, skip_native);
  int size = message.ByteSize();
  Rcpp::RawVector res(size);
  if(!message.SerializeToArray(res.begin(), size))
    throw std::runtime_error("Failed to serialize into protobuf message");
  return res;
}
