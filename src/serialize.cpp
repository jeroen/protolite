#include "rexp.pb.h"
#include <Rcpp.h>

//using namespace Rcpp;

rexp::REXP rexp_real(Rcpp::NumericVector x){
  rexp::REXP out;
  out.set_rclass(rexp::REXP_RClass_REAL);
  for(int i = 0; i < x.length(); i++)
    out.add_realvalue(x[i]);
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

rexp::REXP rexp_native(Rcpp::RObject x){
  rexp::REXP out;
  out.set_rclass(rexp::REXP_RClass_NATIVE);

  /* This looks nicer but it doesn't work for 'call' objects
   Rcpp::Function serialize = Rcpp::Environment::namespace_env("base")["serialize"];
   Rcpp::RawVector buf = serialize(x, R_NilValue);
   */
  Rcpp::Environment env;
  env["MY_R_OBJECT"] = x;
  Rcpp::ExpressionVector expr("serialize(MY_R_OBJECT, NULL)");
  Rcpp::RawVector buf = Rcpp::Rcpp_eval(expr, env);
  out.set_rawvalue(buf.begin(), buf.length());
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
rexp::REXP rexp_object(Rcpp::RObject x);

rexp::REXP rexp_list(Rcpp::List x){
  rexp::REXP out;
  out.set_rclass(rexp::REXP_RClass_LIST);
  for(int i = 0; i < x.length(); i++){
    rexp::REXP *val = out.add_rexpvalue();
    rexp::REXP obj = rexp_object(x[i]);
    val->CopyFrom(obj);
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
rexp::REXP rexp_any(Rcpp::RObject x){
  switch(TYPEOF(x)){
    case NILSXP: return rexp_null();
    case LGLSXP: return rexp_bool(Rcpp::as<Rcpp::LogicalVector>(x));
    case INTSXP: return rexp_int(Rcpp::as<Rcpp::IntegerVector>(x));
    case REALSXP: return rexp_real(Rcpp::as<Rcpp::NumericVector>(x));
    case CPLXSXP: return rexp_complex(Rcpp::as<Rcpp::ComplexVector>(x));
    case STRSXP: return rexp_string(Rcpp::as<Rcpp::StringVector>(x));
    case VECSXP: return rexp_list(Rcpp::as<Rcpp::List>(x));
    case RAWSXP: return rexp_raw(Rcpp::as<Rcpp::RawVector>(x));
    default: return rexp_native(x);
  }
}

rexp::REXP rexp_object(Rcpp::RObject x){
  rexp::REXP out = rexp_any(x);
  if(out.rclass() != rexp::REXP_RClass_NATIVE){
    std::vector< std::string > attr_names = x.attributeNames();
    int len = attr_names.size();
    for(int i = 0; i < len; i++) {
      std::string *name = out.add_attrname();
      name->assign(attr_names[i]);
      rexp::REXP *val = out.add_attrvalue();
      val->CopyFrom(rexp_object(x.attr(*name)));
    }
  }
  return out;
}

// [[Rcpp::export]]
Rcpp::RawVector cpp_serialize_pb(Rcpp::RObject x){
  rexp::REXP message = rexp_object(x);
  int size = message.ByteSize();
  Rcpp::RawVector res(size);
  if(!message.SerializeToArray(res.begin(), size))
    throw std::runtime_error("Failed to serialize into protobuf message");
  return res;
}
