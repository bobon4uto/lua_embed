#ifdef    MONO_BUILD
#define METAM_IMPLEMENTATION
#endif // MONO_BUILD
#ifndef    _METAM_H_
#define    _METAM_H_
// metam interface

#define NOU_MONO_BUILD
#include "../thirdparty/no.util/core.h"


// :macro 
#define LOG(...) printf(__VA_ARGS__); printf("\n"); fflush(stdout)
#define LOGI(...) printf("Info: ");printf(__VA_ARGS__); printf("\n")
#define LOGSV(sv) printf("%.*s\n", SV_FMT(sv) )
#define LOGSB(sb) printf("%.*s\n", SB_FMT(sb) )
#define LOGE(...) printf("Error: ");printf(__VA_ARGS__); printf("\n")
#define LOGW(...) printf("Warning: ");printf(__VA_ARGS__); printf("\n")

#define TERM_COMMENT               sv_from_str("//")
#define TERM_DOC_COMMENT           sv_from_str("///")
#define TERM_MACRO                 sv_from_str("#")
#define TERM_NEWLINE               sv_from_str("\n")
#define TERM_INTERFACE             sv_from_str("// :f")
#define TERM_IMPLEMENTATION        sv_from_str("// :i")


#define TERM_SPACE                 sv_from_str(" ")
#define TERM_TAB                   sv_from_str("\t")
#define TERM_ESCAPE_ONE            sv_from_str("\\")
#define TERM_STRING_BORDER         sv_from_str("\"")
#define TERM_START_MODULE          sv_from_str("{")
#define TERM_END_MODULE            sv_from_str("}")
#define TERM_CITIZEN_DEF           sv_from_str(":")
#define TERM_ASSIGN                sv_from_str("=")
#define TERM_START_TYPE            sv_from_str("{")
#define TERM_END_TYPE              sv_from_str("}")
#define TERM_START_FUNCTION        sv_from_str("{")
#define TERM_END_FUNCTION          sv_from_str("}")
#define TERM_START_CLUSTER         sv_from_str("(")
#define TERM_END_CLUSTER           sv_from_str(")")
#define TERM_SEPARATOR_IN_CLUSTER  sv_from_str(",")
#define TERM_END_STMT              sv_from_str(";")
#define TERM_POINTER               sv_from_str("*")
#define TERM_RETURN_TYPE_DEF       sv_from_str("->")

#define KEYWORD_MODULE sv_from_str("mod")
#define KEYWORD_TYPE_DEFINITION   sv_from_str("typedef")
#define KEYWORD_DYNAMIC_ARRAY sv_from_str("DA_")
#define KEYWORD_TYPE   sv_from_str("type")
#define KEYWORD_STRUCT sv_from_str("struct")
#define KEYWORD_ENUM   sv_from_str("enum")
#define KEYWORD_UNION  sv_from_str("union")
#define KEYWORD_NOU sv_from_str("NOU")
#define KEYWORD_FUNCTIONS_INTERFACE sv_from_str("unc")


// :type



typedef struct sMetaStringView {
  StringView sv;
  u x;
  u y;
  u num;
  StringView comment;
} MetaStringView;
NOU(MetaStringView);


typedef struct sVariable Variable;
NOU(Variable);
typedef struct sFunction Function;
NOU(Function);

typedef enum {
  WRAPPER_NONE,
  WRAPPER_DA,
} Wrapper;

typedef struct sType {
  MetaStringView name;
  bool needs_clone;
  StringView  push_lua;
  u push_lua_ptr;
  StringView check_lua;
  u check_lua_ptr;
  DA_(Variable) attributes;
  DA_(Function) methods;
  DA_(Function) static_methods;
} Type;
NOU(Type);


typedef struct sTypeInstance {
  StringView name;
  u pointer_num;
  Wrapper wrapper;
} TypeInstance;
NOU(TypeInstance);


struct sVariable {
  MetaStringView name;
  TypeInstance type;
};

struct sFunction {
  MetaStringView name;
  TypeInstance return_type;
  DA_(Variable) args;
};


typedef struct sInfoBank {
  StringView module_name;
  DA_(Type)         types;
  DA_(Function) functions;
  StringBuilder tmp_sb;
} InfoBank;

typedef char* char_ptr;
NOU(char_ptr);
// :fMetam
StringBuilder metam_full_sweep( DA_(char_ptr) sources, StringView module_name );
void metam_push_type_into_sb    (StringBuilder* info, Type type);
void metam_push_function_into_sb(StringBuilder* info, Function function);
void metam_push_variable_into_sb(StringBuilder* info, Variable variable);


// :fInfoBank

InfoBank ib_init(StringView source, StringView source_name);
void ib_compile_modules_to_c(InfoBank ib, StringBuilder* compiled);
StringBuilder ib_show_modules(InfoBank ib);
Type* ib_find_type(InfoBank* ib, StringView type_name);
void ib_compile_lua_interface(InfoBank* ib, StringBuilder* compiled);



// :fMetaStringView
u meta_sv_match_from(DA_(MetaStringView) msvs, StringView open_sv, StringView close_sv, u i);


#ifdef      METAM_IMPLEMENTATION
// metam implementation
u metam_great_split(StringView left);

u wordsplit(StringView left) {
  char* delims[] = {
    "// :f", // so lame but whateva
    "// :i",
    " ", "\n", "\t", "\r",
    ":", ";", "|", "||", "&&", "?", "!",
    "\\", "//", "///", "%",
    "\"", "'", "`", "~", ",",
    ".", "->",
    "{", "}", "(", ")", "[", "]",
    "=", "*", "+", "-", "#",
    NULL
  };
  u len  = 0;
  for (u i = 0; delims[i]!=NULL; ++i) {
    if (sv_starts_with_str(left, delims[i])) {
      u slen = strlen(delims[i]);
      if (slen>len) {
        len = slen;
      }
    }
  }
  return len;
}


u sv_match_once_with_escape(DA_(StringView) svs, StringView open_sv, StringView close_sv, u i, StringView escape) {
  u matched_i = i;

  da_bump_start(&svs, matched_i);
  u parity = 0;
  if (!sv_is_equal( da_first(&svs), open_sv) ) {
    return 0;
  }
  if (sv_is_equal(open_sv, close_sv)) {
    da_bump_start(&svs, 1);
    matched_i++;
  }
  foreach( StringView as sv in svs ) {
    if ( sv_is_equal(sv, escape) ) {current++; matched_i++;} // skip one
    if ( sv_is_equal(sv, close_sv)) return matched_i;
    matched_i++;
  }
  return matched_i;
}


u try_match_complex_delim(StringView left, StringView open_sv, StringView close_sv, StringView escape) {
  DA_(StringView) words = sv_splitl(left, wordsplit);
  u matched = sv_match_once_with_escape(words, open_sv, close_sv, 0, escape);
  if (matched == 0) {
    return 0;
  }
  u offset = 0;
  foreach (StringView as word in words) {
    offset += word.count;
    if (0 == matched) break;
    matched--;
  }
  da_free(&words);
  return offset;
}

u metam_great_split(StringView left) {
  u len = 0;
  u ret = 0;
  len = wordsplit(left); if (len>ret) ret=len;
  len = try_match_complex_delim(left, TERM_COMMENT, TERM_NEWLINE, TERM_ESCAPE_ONE); if (len>ret) ret=len;
  len = try_match_complex_delim(left, TERM_DOC_COMMENT, TERM_NEWLINE, TERM_ESCAPE_ONE); if (len>ret) ret=len;
  len = try_match_complex_delim(left, TERM_MACRO, TERM_NEWLINE, TERM_ESCAPE_ONE); if (len>ret) ret=len;
  len = try_match_complex_delim(left, TERM_STRING_BORDER, TERM_STRING_BORDER, TERM_ESCAPE_ONE); if (len>ret) ret=len;
  return ret;
}

// iInfoBank
DA_(MetaStringView) metam_words_to_meta_words(DA_(StringView) words) {
  DA_(MetaStringView) meta_words = {0};
  StringView current_doc_comment = {0};
  u x = 1;
  u next_x = 1;
  u y = 1;
  u current_num = 0;
  u next_num = 0;
  foreach(StringView as word in words) {
    current_num = next_num;
    next_num += word.count;
    x = next_x;
    next_x += word.count;
    if ( sv_is_equal(word, TERM_IMPLEMENTATION) ) {
      break;
    }

    if (
        sv_ends_with_sv(word, TERM_NEWLINE)
        ) {
      y++;
      x=1;
      next_x=1;
      continue;
    }



    if (
        sv_starts_with_sv(word, TERM_DOC_COMMENT)
        ) {
      if  (sv_is_equal(current_doc_comment, (StringView){0})) {
        current_doc_comment = word;
      } else {
        current_doc_comment = sv_from_sv_range(current_doc_comment, word);
      }
      continue;
    }

    if ( !sv_is_equal(word, TERM_INTERFACE) ) {
      if (
          sv_starts_with_sv(word, TERM_COMMENT)
          || sv_is_equal(word, TERM_SPACE)
          || sv_is_equal(word, TERM_TAB)
          || sv_is_equal(word, TERM_NEWLINE)
          ) {
        continue;
      }
    }
    MetaStringView meta_word = {0};
    meta_word.sv = word;
    meta_word.x = x;
    meta_word.y = y;
    meta_word.num = current_num;
    meta_word.comment = current_doc_comment;
    current_doc_comment = (StringView){0};

    da_push(&meta_words, meta_word);
  }

  da_free(&words);
  return meta_words;
}

typedef struct sIterInfo {
  MetaStringView* current;
  MetaStringView* cur;
  MetaStringView* end;
  StringBuilder log_sb;
  StringView source_name;
  Type* active_type;

} IterInfo;
bool expect_non_end_current(IterInfo* ii) {
  if (ii->log_sb.count != 0) {
    return false;
  }
  if ( ii->cur == ii->end) {
    ii->cur -= 1;
    ii->log_sb.count = 0;
    sb_push_sv(&ii->log_sb, ii->source_name);
    sb_push_fmt(&ii->log_sb, ":%zu:%zu: ", ii->cur->y, ii->cur->x);
    sb_push_str(&ii->log_sb, "ERROR: ");
    sb_push_str(&ii->log_sb, "found premature end of input\n");
    return false;
  }
  return true;
}
bool do_error(IterInfo* ii) {
  sb_push_fmt(&ii->log_sb, "ERROR");
  return false;
}
void ii_point(IterInfo* ii) {
  printf("%.*s:%zu:%zu: ", ii->source_name, ii->cur->y, ii->cur->x);
}
bool expect_non_end(IterInfo* ii) {
  if (ii->log_sb.count != 0) {
    printf( "%.*s", SB_FMT(ii->log_sb) );
    exit(1);
    return false;
  }
  ii->cur += 1;
  return expect_non_end_current(ii);
}
bool expect(IterInfo* ii, StringView expected) {
    if (ii->log_sb.count != 0) {
      printf( "%.*s", SB_FMT(ii->log_sb) );
      exit(1);
      return false;
    }
    bool ok = expect_non_end(ii);
    if (!ok) {
      return ok;
    }
    if ( !sv_is_equal(ii->cur->sv, expected)) {
      ii->log_sb.count = 0;
      sb_push_sv(&ii->log_sb, ii->source_name);
      sb_push_fmt(&ii->log_sb, ":%zu:%zu: ", ii->cur->y, ii->cur->x);
      sb_push_str(&ii->log_sb, "ERROR: expected `");
      sb_push_sv(&ii->log_sb, expected);
      sb_push_str(&ii->log_sb, "`, but found `");
      sb_push_sv(&ii->log_sb, ii->cur->sv);
      sb_push_str(&ii->log_sb, "`\n");
      return false;
    }
    return true;
}

TypeInstance ib_process_type_instance(InfoBank* ib, IterInfo* ii) {
  TypeInstance ti = {0};
  if ( sv_is_equal( ii->cur->sv, KEYWORD_DYNAMIC_ARRAY ) ) {
    expect(ii, TERM_START_CLUSTER);
    expect_non_end(ii);
    ti.name = ii->cur->sv;
    ti.wrapper = WRAPPER_DA;
    expect(ii, TERM_END_CLUSTER);
  } else {
    ti.name = ii->cur->sv;
  }
  expect_non_end(ii);
  while ( sv_is_equal(ii->cur->sv, TERM_POINTER) ) {
    ti.pointer_num += 1;
    expect_non_end(ii);
  }
  return ti;
}

void ib_process_struct_internals(InfoBank* ib,  IterInfo* ii, Type* type) {
  expect_non_end(ii);
  while ( !sv_is_equal(ii->cur->sv, TERM_END_TYPE) ) {
    Variable attribute = {0};
    attribute.name.comment = ii->cur->comment;

    attribute.type = ib_process_type_instance(ib, ii);
    attribute.name.sv = ii->cur->sv;
    expect(ii, TERM_END_STMT);
    expect_non_end(ii);

    da_push(&type->attributes, attribute);
  }

}
MetaStringView struct_type_to_type(MetaStringView st) {
  MetaStringView type_name = st;
  type_name.sv = sv_bump_start(type_name.sv, 1);
  return type_name;
}
Type* ib_process_struct(InfoBank* ib, IterInfo* ii) {
  expect_non_end(ii);
  MetaStringView type_name = struct_type_to_type(*ii->cur);
  Type* new_type = ib_find_type(ib, type_name.sv);
  expect_non_end(ii);
  if ( sv_is_equal(ii->cur->sv, TERM_START_TYPE) ) {
    // typedef struct sName {`internals`} Name;
    ib_process_struct_internals(ib, ii, new_type);
  } else {
    // typedef struct sName Name;
    // ...
    // struct sName {`internals`};
    ii->cur -= 1;
  }
  return new_type;
}
void ib_process_type_definition(InfoBank* ib, IterInfo* ii) {
  MetaStringView typedef_msv = *ii->cur;
  expect_non_end(ii);
  if ( sv_is_equal( ii->cur->sv, KEYWORD_STRUCT)  ) {
    Type* new_type = ib_process_struct(ib, ii);
    new_type->name.comment = typedef_msv.comment;
    expect(ii, new_type->name.sv);
    expect(ii, TERM_END_STMT);
  } else {
    // TODO: enum and union
  }
}

bool sv_compare_with_sb_and_free(StringView sv, StringBuilder sb) {
  bool cmp = sv_is_equal(sv, sv_from_sb(sb));
  sb_free(&sb);
  return cmp;
}
u sv_starts_in_short(StringView sv, StringView full) {
  bool expecting_match = true;
  u count = 0;
  foreach (char as c in full) {
    if (sv_first(sv)=='_') return count;
    if (c == '_') {
      expecting_match = true;
      continue;
    }
    if (expecting_match) {
      expecting_match = false;
      char short_c = sv_first(sv);
      sv = sv_bump_start(sv, 1);
      if (short_c=='_') return count;
      count +=1;
      if (short_c!=c) return 0;
    }
  }
  return 0;
}
bool is_upper(char c) {
  return c >= 'A' && c <= 'Z';
}
bool is_lower(char c) {
  return c >= 'a' && c <= 'z';
}
char to_upper(char c) {
  if ( !is_lower(c) ) {
    return c;
  }
  return c - 'a' + 'A';
}
char to_lower(char c) {
  if ( !is_upper(c) ) {
    return c;
  }
  return c - 'A' + 'a';
}

StringBuilder camel_to_camel(StringView sv) {
  return sb_from_sv(sv);
}

StringBuilder camel_to_snake(StringView sv) {
  StringBuilder ret = {0};
  sb_push_c(&ret, to_lower( sv_first(sv) ) );
  sv = sv_bump_start(sv,1);
  foreach (char as c in sv) {
    if ( is_upper(c) ) {
      sb_push_c(&ret, '_');
    }
    sb_push_c(&ret, to_lower(c) );
  }
  return ret;
}
StringBuilder camel_to_screaming_snake(StringView sv) {
  StringBuilder ret = camel_to_snake(sv);
  foreach (char as c in ret) {
    *current = to_upper(c);
  }
  return ret;
}
StringBuilder camel_to_meta_table(StringView sv) {
  StringBuilder ret = camel_to_screaming_snake(sv);
  sb_push_str(&ret, "_META_TABLE");
  return ret;
}
StringBuilder function_name_shortened(StringView sv, StringView type_name) {
  StringBuilder type_name_snake_case_sb = camel_to_snake(type_name);
  StringView name_snake_case = sv_from_sb(type_name_snake_case_sb);
  StringBuilder ret = sb_from_sv(sv);
  if ( sv_starts_with_sv(sv, name_snake_case) ) {
    sb_free(&ret);
    ret = sb_from_sv( sv_bump_start( sv, name_snake_case.count + 1 ) );
  } else if ( sv_starts_in_short(sv, name_snake_case) > 0 ) {
    sb_free(&ret);
    ret = sb_from_sv( sv_bump_start( sv, sv_starts_in_short(sv, name_snake_case) + 1 ) );
  }
  sb_free(&type_name_snake_case_sb);
  return ret;
}
void ib_process_function(InfoBank* ib, IterInfo* ii) {
  Function function = {0};
  function.name.comment = ii->cur->comment;
  function.return_type = ib_process_type_instance(ib, ii);
  function.name.sv= ii->cur->sv;
  expect(ii, TERM_START_CLUSTER);

  expect_non_end(ii);
  while ( !sv_is_equal( ii->cur->sv, TERM_END_CLUSTER)  ) {
    Variable arg = {0};
    arg.type = ib_process_type_instance(ib, ii);
    arg.name.sv= ii->cur->sv;
    expect_non_end(ii);
    if ( sv_is_equal(ii->cur->sv, TERM_SEPARATOR_IN_CLUSTER) ) {
      expect_non_end(ii);
    } else {
      ii->cur -= 1;
      expect(ii, TERM_END_CLUSTER);
    }
    da_push(&function.args, arg);
  }
  expect(ii, TERM_END_STMT);
  if (ii->active_type) {
    bool is_static = false;
    if (function.args.count == 0) {
      is_static = true;
    } else {
      if ( sv_is_equal( function.args.items[0].type.name, ii->active_type->name.sv)  ) {
        is_static = false;
      } else {
        is_static = true;
      }
    }
    if (is_static) {

      if ( sv_compare_with_sb_and_free(sv_from_str("check_lua"), function_name_shortened(function.name.sv, ii->active_type->name.sv) )  ) {
        ii->active_type->check_lua_ptr = function.return_type.pointer_num;
        ii->active_type->check_lua = function.name.sv;
      } else if ( sv_compare_with_sb_and_free(sv_from_str("push_lua"), function_name_shortened(function.name.sv, ii->active_type->name.sv) )  ) {
        ii->active_type->push_lua_ptr = function.args.items[1].type.pointer_num;
        ii->active_type->push_lua = function.name.sv;
      } else {
        da_push(&ii->active_type->static_methods, function);
      }
    } else {


      da_push(&ii->active_type->methods, function);
    }
  } else {
    da_push(&ib->functions, function);
  }
}
void ib_process_nou(InfoBank* ib,IterInfo* ii) {
  expect(ii, TERM_START_CLUSTER);
  expect_non_end(ii);
  expect(ii, TERM_END_CLUSTER);
  expect(ii, TERM_END_STMT);
}



char* base_types[] = {
  "int", "void", "char", "StringBuilder",
  NULL
};



void ib_register_base_types(InfoBank* ib) {
  for (u i = 0; base_types[i]!=NULL; ++i) {
    Type new_type = {0};
    new_type.name.sv = sv_from_str(base_types[i]);
    new_type.name.comment = sv_from_str("/// This type is base");
    da_push(&ib->types, new_type);
  }
}
bool type_has_method(Type type, char* method_name) {
  foreach (Function as method in type.methods) {
   if ( sv_compare_with_sb_and_free(sv_from_str(method_name), function_name_shortened(method.name.sv, type.name.sv) ) ) {
     return true;
   }
  }
  return false;
}
StringView sv_on_tmp_sb(StringBuilder *sb, cstr fmt, ...) {

    nou_va_list args;

    nou_va_start(args, fmt);
    int n = nou_vsnprintf(NULL, 0, fmt, args);
    nou_va_end(args);

    da_ensure(sb, sb->count + n + 1);
    char *dest = sb->items + sb->count;
    nou_va_start(args, fmt);
    nou_vsnprintf(dest, n+1, fmt, args);
    nou_va_end(args);


    StringView sv = {0};
    sv.items = sb->items + sb->count;
    sv.count = n;

    sb->count += n;

    return sv;
}
void ib_fix_clones(InfoBank* ib) {
  foreach (Type as type in ib->types) {
    Type* t = current;
    bool has_clone = type_has_method(type, "clone");
    bool any_attr_has_clone = false;
    foreach (Variable as attribute in type.attributes) {
      Type attr_type = *ib_find_type(ib, attribute.type.name);
      if ( type_has_method(attr_type,"clone") ) {
        any_attr_has_clone = true;
      }

    }

    if (!has_clone && any_attr_has_clone) {
      t->needs_clone = true;
      Function clone_fn = {0};
      clone_fn.name = (MetaStringView){0};

      StringBuilder type_name_snake_case_sb = camel_to_snake(type.name.sv);
      StringView name_snake_case = sv_from_sb(type_name_snake_case_sb);
      clone_fn.name.sv = sv_on_tmp_sb(&ib->tmp_sb, "%.*s_clone", SV_FMT(name_snake_case) );
      sb_free(&type_name_snake_case_sb);
      clone_fn.return_type.name = type.name.sv;
      Variable arg = {0};
      arg.name.sv = sv_from_str("self");
      arg.type.name = type.name.sv;
      arg.type.pointer_num = 1;
      da_push(&clone_fn.args, arg);

      da_push(&t->methods, clone_fn);
    }
  }
}
void ib_push(InfoBank* ib, StringView source, StringView source_name) {
  DA_(StringView) words = sv_splitl(source, metam_great_split);
  DA_(MetaStringView) meta_words = metam_words_to_meta_words(words);



  u i = 0;
  u count = 0;

  u to_jump = 0; IterInfo ii0 = {0};
  IterInfo* ii = &ii0;
  ii->source_name = source_name;
  ii->active_type = NULL;


  //ib_register_base_types(ib);
  foreach(MetaStringView as meta_word in meta_words) {
    ii->current = current;
    ii->cur = current;
    ii->end = end;

    if ( sv_is_equal(ii->cur->sv, KEYWORD_STRUCT) ) {
      ib_process_struct(ib, ii);
    } else if ( sv_is_equal(ii->cur->sv, KEYWORD_NOU) ) {
      ib_process_nou(ib,ii);
      // TODO: register type as NOU compatable
    } else if ( sv_is_equal(ii->cur->sv, KEYWORD_TYPE_DEFINITION) ) {
      ib_process_type_definition(ib, ii);
    } else if ( sv_is_equal(meta_word.sv, TERM_INTERFACE) ) {
      expect_non_end(ii);
      if ( sv_is_equal( ii->cur->sv, KEYWORD_FUNCTIONS_INTERFACE )  ) {
        ii->active_type = NULL;
      } else {
        ii->active_type = ib_find_type(ib, ii->cur->sv);
      }
    } else if ( sv_is_equal(meta_word.sv, KEYWORD_DYNAMIC_ARRAY) ) {
      ib_process_function(ib, ii);
    } else {
      foreach (Type as type in ib->types) {
        if ( sv_is_equal(ii->cur->sv, type.name.sv) ) {
          ib_process_function(ib, ii);
        }
      }
    }
    if (ii->log_sb.count != 0) {
      break;
    }
    current = ii->cur;
    continue;
  }
  da_free(&meta_words);
}
InfoBank ib_init(StringView source, StringView source_name) {
  InfoBank ib0 = {0};
  InfoBank* ib = &ib0;

  ib_push(ib, source, source_name);

  return ib0;
}
StringBuilder metam_full_sweep( DA_(char_ptr) sources, StringView module_name ) {
  StringBuilder compiled = {0};

  DA_(StringBuilder) source_files = {0};
  foreach (char_ptr as source in sources) {
    da_push(&source_files, sb_from_file(source));
  }

  InfoBank ib = {0};
  da_reserve(&ib.tmp_sb, 1024);
  ib.module_name = module_name;
  for ( u i=0; i < sources.count; ++i ) {
    ib_push(&ib, sv_from_sb(source_files.items[i]), sv_from_str(sources.items[i]));
  }

  ib_compile_lua_interface(&ib, &compiled);

  foreach (StringBuilder as source_file in source_files) {
    (void)source_file;
    sb_free(current);
  }

  return compiled;
}


// :iInfoBank

Type* ib_find_type(InfoBank* ib, StringView type_name) {
  foreach (Type as type in ib->types) {
    if (sv_is_equal( type.name.sv, type_name)) {
      return current;
    }
  }
  Type new_type = {0};
  new_type.name.sv = type_name;
  da_push(&ib->types, new_type);
  return da_last_ptr(&ib->types);
}

// :iMetaStringView
u meta_sv_match_from(DA_(MetaStringView) msvs, StringView open_sv, StringView close_sv, u i) {
  u matched_i = i;
  da_bump_start(&msvs, matched_i);
  u parity = 0;
  foreach( MetaStringView as msv in msvs ) {
    StringView sv = msv.sv;
    if (sv_starts_with_sv(sv, open_sv)) {sv.items++;break;}
  }
  foreach( MetaStringView as msv in msvs ) {
    StringView sv = msv.sv;
    if (sv_starts_with_sv(sv, open_sv)) parity++;
    if (sv_starts_with_sv(sv, close_sv)) parity--;
    if (parity == 0) break;
    matched_i++;
  }
  return matched_i;
}


void sb_push_and_free_sb(StringBuilder* sb, StringBuilder to_push) {
  sb_push_sv(sb, sv_from_sb(to_push));
  sb_free(&to_push);
}


void sb_push_type_instance(StringBuilder* compiled, TypeInstance type) {
  if (type.wrapper == WRAPPER_DA) {
    sb_push_str(compiled, "DA_(");
    sb_push_sv (compiled, type.name);
    sb_push_str(compiled, ")");
  } else {
    sb_push_sv (compiled, type.name);
  }
  for (u i =0; i < type.pointer_num; ++i) {
    sb_push_str(compiled, "*");
  }
}
StringBuilder type_instance_to_meta_table(TypeInstance type) {
  return camel_to_meta_table(type.name);
  // TODO: make pointer and DA work
}
bool type_instance_is_base_type(TypeInstance type) {
  for (u i = 0; base_types[i]!=NULL; ++i) {
    if ( sv_is_equal_str(type.name, base_types[i]) ) {
      return true;
    }
  }
  return false;
}
bool sb_push_check_base_type_inctance(StringBuilder* compiled, TypeInstance type, int index) {
  if ( sv_is_equal_str(type.name, "char") ) {
    if (type.pointer_num >= 2) {
      return false;
    }

    if (type.pointer_num == 0) {
      sb_push_str(compiled, "*");
    }
    // discard const
    // its bad but idk
    sb_push_fmt(compiled, "(char*)luaL_checkstring(L, %d)", index);

  } else if ( sv_is_equal_str(type.name, "int") ) {
    if (type.pointer_num >= 1) {
      return false;
    }

    sb_push_fmt(compiled, "luaL_checkinteger(L, %d)", index);
  } else if ( sv_is_equal_str(type.name, "StringBuilder") ) {
    if (type.pointer_num >= 1) {
      return false;
    }
    sb_push_fmt(compiled, "sb_from_str(luaL_checkstring(L, %d))", index);
  } else if ( sv_is_equal_str(type.name, "StringView") ) {
    sb_push_fmt(compiled, "sv_from_str(luaL_checkstring(L, %d))", index); // Im not siure if its safe tbh
  }


  return true;

}
StringView type_fetch_clone_method(Type type) {
  foreach (Function as method in type.methods) {
    if ( sv_compare_with_sb_and_free(sv_from_str("clone"), function_name_shortened( method.name.sv, type.name.sv ) ) ) {
      return method.name.sv;
    }
  }

  return (StringView){0};
}
void sb_push_check_type_inctance_opt(StringBuilder* compiled, InfoBank* ib, TypeInstance type, bool try_base, int index) {
  Type t = *ib_find_type(ib, type.name);
  StringView clone_sv = type_fetch_clone_method(t);
  if (try_base) {
    if (t.check_lua.count > 0) {
      if (t.check_lua_ptr == type.pointer_num) {
        // the type has custom check function ( see base_types.h for example )
        sb_push_sv (compiled, t.check_lua);
        sb_push_str(compiled, "(L, ");
        sb_push_fmt(compiled, "%d", index);
        sb_push_str(compiled, ")");
        return;
      }
    }


    //if ( type_instance_is_base_type(type) ) {
    //  if (sb_push_check_base_type_inctance(compiled, type, index)) {
    //    return;
    //  }
    //}
  }
  if (clone_sv.count > 0 && type.pointer_num == 0) {
    sb_push_sv (compiled, clone_sv);
    sb_push_str(compiled, "(");
  } else if (type.pointer_num == 0) {
    sb_push_str(compiled, "*");
  }

  sb_push_str(compiled, "(");
  sb_push_type_instance(compiled, type);
  if (type.pointer_num == 0) {
    sb_push_str(compiled, "*");
  }
  sb_push_fmt(compiled, ")luaL_checkudata(L, %d, ", index);
  sb_push_and_free_sb(compiled, type_instance_to_meta_table(type) );
  sb_push_str(compiled, ")");

  if (clone_sv.count > 0 && type.pointer_num == 0) {
    sb_push_str(compiled, ")");
  }

}
void sb_push_check_type_inctance(StringBuilder* compiled, InfoBank* ib, TypeInstance type, int index) {
  sb_push_check_type_inctance_opt(compiled, ib, type, true, index);
}
void sb_push_check_type(StringBuilder* compiled, InfoBank* ib, Type type, int index) {
  TypeInstance ti = {0};
  ti.name = type.name.sv;
  ti.pointer_num = 1;
  sb_push_check_type_inctance_opt(compiled, ib, ti, false, index);
}
bool sb_push_base_type_data(StringBuilder* compiled, Variable data, bool is_on_self) {
  if ( sv_is_equal_str(data.type.name, "int") ) {
    if (data.type.pointer_num > 0) {
      return false;
    }
    sb_push_str(compiled, "lua_pushinteger(L, (lua_Integer)");
    if (is_on_self) {
      sb_push_str(compiled, "self->");
    }
    sb_push_sv (compiled, data.name.sv);
    sb_push_str(compiled, ");");
  }
  if ( sv_is_equal_str(data.type.name, "char") ) {
    if (data.type.pointer_num != 1) {
      return false;
    }
    sb_push_str(compiled, "lua_pushlstring(L, ");
    sb_push_sv (compiled, data.name.sv);
    sb_push_str(compiled, ", strlen(");
    sb_push_sv (compiled, data.name.sv);
    sb_push_str(compiled, ") );");
  }
  if ( sv_is_equal_str(data.type.name, "StringView") ) {
    if (data.type.pointer_num != 1) {
      return false;
    }
    sb_push_str(compiled, "lua_pushlstring(L, ");
    sb_push_sv (compiled, data.name.sv);
    sb_push_str(compiled, ".items, ");
    sb_push_sv (compiled, data.name.sv);
    sb_push_str(compiled, ".count );");
  }

  return true;
}
void sb_push_new_data(StringBuilder* compiled, InfoBank* ib, Variable data, bool is_on_self) {
  Type t = *ib_find_type(ib, data.type.name);
  if (t.push_lua.count > 0) {
    if ( t.push_lua_ptr == data.type.pointer_num) {
      // the type has custom push function ( see base_types.h for example )
      sb_push_sv (compiled, t.push_lua);
      sb_push_str(compiled, "(L, ");
      if (is_on_self) {
        sb_push_str(compiled, "self->");
      }
      sb_push_sv (compiled, data.name.sv);
      sb_push_str(compiled, ")");
      return;
    }
  }
  // if ( type_instance_is_base_type(data.type) ) {
  //   if (sb_push_base_type_data(compiled, data, is_on_self)) {
  //     return;
  //   }
  // }
  sb_push_str(compiled, "*( (");
  sb_push_type_instance(compiled, data.type);
  sb_push_str(compiled, "*)lua_newuserdatauv(L, sizeof(");
  sb_push_type_instance(compiled, data.type);
  sb_push_str(compiled, "), 0) ) = ");
  if (is_on_self) {
    sb_push_str(compiled, "self->");
  }
  sb_push_sv (compiled, data.name.sv);
  sb_push_str(compiled, "; luaL_setmetatable(L, ");
  sb_push_and_free_sb(compiled, type_instance_to_meta_table(data.type) );
  sb_push_str(compiled, ")");
}
bool function_ret_type_is_non_void(Function function) {
  return !sv_is_equal_str(function.return_type.name, "void");
}

void ib_compile_lua_function(InfoBank* ib, Function function, StringBuilder* compiled) {
  sb_push_str(compiled, "int ");
  sb_push_sv (compiled, function.name.sv);
  sb_push_str(compiled, "_lua(lua_State* L) {\n");
  u i = 0;
  foreach (Variable as arg in function.args) {
    sb_push_str(compiled, "  ");
    sb_push_type_instance(compiled, arg.type);
    sb_push_str(compiled, " ");
    sb_push_sv (compiled, arg.name.sv);
    sb_push_str(compiled, " = ");
    sb_push_check_type_inctance(compiled, ib, arg.type, ++i);
    sb_push_str(compiled, ";\n");
  }

  sb_push_str(compiled, "  ");
  if ( function_ret_type_is_non_void(function) ) {
    sb_push_type_instance(compiled, function.return_type);
    sb_push_str(compiled, " ret = ");
  }
  sb_push_sv (compiled, function.name.sv);
  sb_push_str(compiled, "(");
  foreach (Variable as arg in function.args) {
    sb_push_sv (compiled, arg.name.sv);
    sb_push_str(compiled, ", ");
  }
  if (function.args.count > 0) {
    compiled->count -= 2;
  }
  sb_push_str(compiled, ");\n  ");


  if ( function_ret_type_is_non_void(function) ) {
    Variable ret = {0};
    ret.name.sv = sv_from_str("ret");
    ret.type = function.return_type;
    sb_push_new_data(compiled, ib, ret, false);
    sb_push_str(compiled, ";\n  return 1;\n");
  } else {
    sb_push_str(compiled, "\n  return 0;\n");
  }
  sb_push_str(compiled, "}\n");


}

void ib_compile_lua_type(InfoBank* ib, Type type, StringBuilder* compiled) {

  StringBuilder type_name_snake_case_sb           = camel_to_snake(type.name.sv);
  StringBuilder type_name_camel_case_sb           = camel_to_camel(type.name.sv);
  StringBuilder type_name_screaming_snake_case_sb = camel_to_screaming_snake(type.name.sv);
  StringBuilder type_meta_table_sb                = camel_to_meta_table(type.name.sv);


  StringView name_snake_case = sv_from_sb(type_name_snake_case_sb);
  StringView name_camel_case = sv_from_sb(type_name_camel_case_sb);
  StringView name_screaming_snake_case = sv_from_sb(type_name_screaming_snake_case_sb);
  StringView type_meta_table                = sv_from_sb(type_meta_table_sb);

  sb_push_str(compiled,"// type ");
  sb_push_sv (compiled,type.name.sv);
  sb_push_str(compiled,"\n");
  sb_push_str(compiled,"#define ");
  sb_push_sv (compiled, type_meta_table );
  sb_push_str(compiled," \"");
  sb_push_sv (compiled,name_snake_case);
  sb_push_str(compiled,"\"\n");

  sb_push_str(compiled,"\n");

// index
  sb_push_str(compiled,"static int ");
  sb_push_sv (compiled, name_snake_case);
  sb_push_str(compiled,"_index(lua_State* L) {\n");
  sb_push_str(compiled,"  ");
  sb_push_sv (compiled, type.name.sv);
  sb_push_str(compiled,"* self = ");
  sb_push_check_type(compiled, ib, type, 1);
  sb_push_str(compiled,";\n");
  sb_push_str(compiled,"  const char *key = luaL_checkstring(L, 2);\n  (void)self; (void)key;\n");

  sb_push_str(compiled,"\n");
  foreach (Variable as attribute in type.attributes) {
    sb_push_str(compiled,"  if (strcmp(key, \"");
    sb_push_sv (compiled, attribute.name.sv);
    sb_push_str(compiled,"\") == 0) {\n");
    sb_push_str(compiled,"    ");
    sb_push_new_data(compiled, ib, attribute, true);
    sb_push_str(compiled,";\n");

    sb_push_str(compiled,"    return 1;\n");
    sb_push_str(compiled,"  }\n");
  }
  sb_push_str(compiled,"\n");

  sb_push_str(compiled, "  luaL_getmetatable(L, ");
  sb_push_sv (compiled, type_meta_table );
  sb_push_str(compiled, ");\n");
  sb_push_str(compiled, "  lua_pushvalue(L, 2);\n");
  sb_push_str(compiled, "  lua_rawget(L, -2);\n");
  sb_push_str(compiled, "  return 1;\n");
  sb_push_str(compiled, "}\n");
  sb_push_str(compiled,"\n");


// newindex
  sb_push_str(compiled,"static int ");
  sb_push_sv (compiled, name_snake_case);
  sb_push_str(compiled,"_newindex(lua_State* L) {\n");
  sb_push_str(compiled,"  ");
  sb_push_sv (compiled, type.name.sv);
  sb_push_str(compiled,"* self = ");
  sb_push_check_type(compiled, ib, type, 1);
  sb_push_str(compiled,";\n");
  sb_push_str(compiled,"  const char *key = luaL_checkstring(L, 2);\n  (void)self; (void)key;\n");

  sb_push_str(compiled,"\n");
  foreach (Variable as attribute in type.attributes) {
    sb_push_str(compiled,"  if (strcmp(key, \"");
    sb_push_sv (compiled, attribute.name.sv);
    sb_push_str(compiled,"\") == 0) {\n");

    sb_push_str(compiled,"    self->");
    sb_push_sv (compiled, attribute.name.sv);
    sb_push_str(compiled," = ");
    sb_push_check_type_inctance(compiled, ib, attribute.type, 3);
    sb_push_str(compiled,";\n");

    sb_push_str(compiled,"    return 0;\n");
    sb_push_str(compiled,"  }\n");
  }
  sb_push_str(compiled,"\n");

  sb_push_str(compiled, "  return luaL_error(L, \"unknown `");
  sb_push_sv (compiled, type.name.sv);
  sb_push_str(compiled, "` field '%s'\", key);\n");
  sb_push_str(compiled,"}\n");


  sb_push_str(compiled,"\n");
  sb_push_str(compiled,"// static methods\n");
  sb_push_str(compiled,"\n");
  foreach (Function as method in type.static_methods) {
    ib_compile_lua_function(ib, method, compiled);
  }


  sb_push_str(compiled,"\n");
  sb_push_str(compiled,"// methods\n");
  sb_push_str(compiled,"\n");
  foreach (Function as method in type.methods) {
    ib_compile_lua_function(ib, method, compiled);
  }


  sb_push_str(compiled, "static const luaL_Reg ");
  sb_push_sv (compiled, name_snake_case);
  sb_push_str(compiled, "_methods[] = {\n");
  foreach (Function as method in type.methods) {
    sb_push_str(compiled, "  {\"");
    if ( sv_compare_with_sb_and_free( sv_from_str("free"), function_name_shortened(method.name.sv, name_snake_case ) ) ) {
      // special mapping free -> __gc
      sb_push_str(compiled, "__gc");
    } else {
      sb_push_and_free_sb(compiled, function_name_shortened(method.name.sv, name_snake_case ) );
    }
    sb_push_str(compiled, "\",  ");
    sb_push_sv (compiled, method.name.sv );
    sb_push_str(compiled, "_lua},\n");
  }
  sb_push_str(compiled, "  {NULL, NULL}\n");
  sb_push_str(compiled, "};\n");


  sb_push_str(compiled, "static const luaL_Reg ");
  sb_push_sv (compiled, name_snake_case);
  sb_push_str(compiled, "_static_methods[] = {\n");
  foreach (Function as method in type.static_methods) {
    sb_push_str(compiled, "  {\"");
    sb_push_and_free_sb(compiled, function_name_shortened(method.name.sv, name_snake_case) );
    sb_push_str(compiled, "\",  ");
    sb_push_sv (compiled, method.name.sv );
    sb_push_str(compiled, "_lua},\n");
  }
  sb_push_str(compiled, "  {NULL, NULL}\n");
  sb_push_str(compiled, "};\n");

/*
static const luaL_Reg point_methods[] = {
    {"move",  point_move},
    {"get_x", point_get_x},
    {"get_y", point_get_y},
    {"__gc",  point_gc},
    {NULL, NULL}
};
*/




  sb_push_str(compiled,"\n");
  sb_free( &type_name_snake_case_sb);
  sb_free( &type_name_camel_case_sb);
  sb_free( &type_name_screaming_snake_case_sb);
  sb_free( &type_meta_table_sb);



}
void ib_compile_lua_interface(InfoBank* ib, StringBuilder* compiled) {
  ib_fix_clones(ib);
  //sb_push_file(compiled, "base_types_interface.h");

  sb_push_str(compiled, "#include <lua.h>\n");
  sb_push_str(compiled, "#include <lauxlib.h>\n");
  sb_push_str(compiled, "#include <stdlib.h>\n");
  sb_push_str(compiled, "#include <stdio.h>\n");
  sb_push_str(compiled, "\n");


  // need to add clone if missing
  // First we define the function so the ordering does not matter
  foreach (Type as type in ib->types) {
    // this means we registered clone as an existing method but it was not implemented yet.
    if (type.needs_clone) {
      sb_push_sv (compiled, type.name.sv);
      sb_push_str(compiled, " ");
      sb_push_and_free_sb(compiled, camel_to_snake(type.name.sv) );
      sb_push_str(compiled, "_clone(");
      sb_push_sv (compiled, type.name.sv);
      sb_push_str(compiled, "* self);\n");
    }
  }
  sb_push_str(compiled, "\n");
  foreach (Type as type in ib->types) {
    if (type.needs_clone) {
      sb_push_sv (compiled, type.name.sv);
      sb_push_str(compiled, " ");
      sb_push_and_free_sb(compiled, camel_to_snake(type.name.sv) );
      sb_push_str(compiled, "_clone(");
      sb_push_sv (compiled, type.name.sv);
      sb_push_str(compiled, "* self) {\n");
      sb_push_str(compiled, "  ");
      sb_push_sv (compiled, type.name.sv);
      sb_push_str(compiled, " ");
      sb_push_str(compiled, "cloned = *self;");
      foreach (Variable as attribute in type.attributes) {
        StringView clone_method = type_fetch_clone_method( *ib_find_type(ib, attribute.type.name) );
        if (clone_method.count > 0) {
          sb_push_str(compiled, "  cloned.");
          sb_push_sv (compiled, attribute.name.sv);
          sb_push_str(compiled, " = ");
          sb_push_sv (compiled, clone_method);
          sb_push_str(compiled, "( &self->");
          sb_push_sv (compiled, attribute.name.sv);
          sb_push_str(compiled, " );\n");

        }
      }

      sb_push_str(compiled, "  return cloned;\n");
      sb_push_str(compiled, "}\n");
    }
  }

  foreach (Type as type in ib->types) {
    ib_compile_lua_type(ib, type, compiled);
  }
  foreach (Function as function in ib->functions) {
    ib_compile_lua_function(ib, function, compiled);
  }

  sb_push_str(compiled, "static const luaL_Reg ");
  sb_push_str(compiled, "module_functions[] = {\n");
  foreach (Function as function in ib->functions) {
    sb_push_str(compiled, "  {\"");
    sb_push_sv (compiled, function.name.sv );
    sb_push_str(compiled, "\",  ");
    sb_push_sv (compiled, function.name.sv );
    sb_push_str(compiled, "_lua},\n");
  }
  sb_push_str(compiled, "  {NULL, NULL}\n");
  sb_push_str(compiled, "};\n");



  sb_push_str(compiled, "int luaopen_");
  sb_push_sv (compiled, ib->module_name);
  sb_push_str(compiled, "_lib(lua_State* L) {\n");
  foreach (Type as type in ib->types) {
    sb_push_str(compiled, "  luaL_newmetatable(L, ");
    sb_push_and_free_sb(compiled, camel_to_meta_table(type.name.sv) );
    sb_push_str(compiled, ");\n");
    sb_push_str(compiled, "\n");
    sb_push_str(compiled, "  lua_pushcfunction(L, ");
    sb_push_and_free_sb(compiled, camel_to_snake(type.name.sv) );
    sb_push_str(compiled, "_index);\n");
    sb_push_str(compiled, "  lua_setfield(L, -2, \"__index\");\n");
    sb_push_str(compiled, "\n");
    sb_push_str(compiled, "  lua_pushcfunction(L, ");
    sb_push_and_free_sb(compiled, camel_to_snake(type.name.sv) );
    sb_push_str(compiled, "_newindex);\n");
    sb_push_str(compiled, "  lua_setfield(L, -2, \"__newindex\");\n");

    sb_push_str(compiled, "  luaL_setfuncs(L, ");
    sb_push_and_free_sb(compiled, camel_to_snake(type.name.sv) );
    sb_push_str(compiled, "_methods, 0);\n");
    sb_push_str(compiled, "  lua_pop(L, 1);\n");
    sb_push_str(compiled, "\n");
    sb_push_str(compiled, "\n");
  }

  sb_push_str(compiled, "\n");
  sb_push_str(compiled, "  luaL_newlib(L, module_functions);\n");
  sb_push_str(compiled, "\n");

  foreach (Type as type in ib->types) {

    sb_push_str(compiled, "  lua_newtable(L);\n");
    sb_push_str(compiled, "luaL_setfuncs(L, ");
    sb_push_and_free_sb(compiled, camel_to_snake(type.name.sv) );
    sb_push_str(compiled, "_static_methods, 0);");
    sb_push_str(compiled, "lua_setfield(L, -2, \"");
    sb_push_sv (compiled, type.name.sv);
    sb_push_str(compiled, "\");");
    sb_push_str(compiled, "\n");
    sb_push_str(compiled, "\n");
  }


  sb_push_str(compiled, "  return 1;\n");
  sb_push_str(compiled, "}\n");










}

#endif   // METAM_IMPLEMENTATION
#endif   //_METAM_H_

