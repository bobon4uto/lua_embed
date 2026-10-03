#ifdef    MONO_BUILD
#define EXAMPLE_IMPLEMENTATION
#endif // MONO_BUILD
#ifndef    _EXAMPLE_H_
#define    _EXAMPLE_H_
// example interface

#include "../thirdparty/no.util/core.h"

typedef struct sSomeType SomeType;
// metam recognizes my NOU macro and creates bindings for dynamic array
NOU(SomeType);


struct sSomeType {
  StringBuilder name;
};

typedef struct sNewType {
  SomeType some;
  int i;
} NewType;


typedef struct sNeedClone {
  StringBuilder name;
} NeedClone ;


/// define functions here
// :func
int int_add(int a, int b);

// :fSomeType

/// if first argument is NOT type itself
/// becomes static method
SomeType some_type_new(char* a);
/// __gc is like a destructor
/// LIKE A WHO????
/// LIKE A DRAGON
//void some_type_free(SomeType* self);

/// pointer to type itself counts too
void some_type_change(SomeType* self);

char* some_type___tostring(SomeType* self);



// this technically works but i use fType because i can search it fast and with space inbetween its not cormftable
// :f NewType

/// short names can also be removed
NewType nt_new(char* a, int i);
/// But anything else is kept as is
void newt_change(NewType* self);

// special: converts to __add (so that nt1+nt2 works)
NewType ntm_add(NewType a, NewType b);

// all `///` comments are getting appended to next item and appear in static instance of a class, so
// lib.NewType.help.new will give you `short names can also be removed`


// :fNeedClone
NeedClone need_clone_new();
//void need_clone_free(NeedClone* self);

// :fStringBuilder
void sb_change(StringBuilder* self);

/// metam does not interact with implementation in any way.
#ifdef      EXAMPLE_IMPLEMENTATION
// example implementation
// :impl
int int_add(int a, int b) {
  return a+b;
}

// :iSomeType

SomeType some_type_new(char* a) {
  SomeType self = {0};
  self.name = sb_from_str(a);

  return self;
}
//void some_type_free(SomeType* self) {
//  sb_free(&self->name);
//}
void some_type_change(SomeType* self) {
  self->name.count = 0;
  sb_push_str(&self->name, "CHANGED");
}
char* some_type___tostring(SomeType* self) {
  return sb_to_str(&self->name);
}



// :iNewType
NewType nt_new(char* a, int i) {
  NewType self = {0};
  self.some = some_type_new(a);
  self.i = i;
  return self;
}
void newt_change(NewType* self) {
  some_type_change(&self->some);
}
NewType ntm_add(NewType a, NewType b) {
  NewType ret = {0};
  ret.some.name = sb_from_sv( sv_from_sb(a.some.name) );
  sb_push_sv(&ret.some.name, sv_from_sb(b.some.name) );
  ret.i = a.i+b.i;
  return ret;
}


// :iNeedClone
NeedClone need_clone_new() {
  NeedClone self = {0};
  self.name = sb_from_str("DONT FORGET TO FREE THIS");
  return self;
}
//void need_clone_free(NeedClone* self) {
//
//  sb_free(&self->name);
//}

// :iStringBuilder
void sb_change(StringBuilder* self) {
  self->count = 3;
}


#endif   // EXAMPLE_IMPLEMENTATION
#endif   //_EXAMPLE_H_

