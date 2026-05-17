#include "mycmp.h"

int smaller(int lhs, int rhs) { return lhs < rhs; }
int bigger(int lhs, int rhs)  { return lhs > rhs; }
int smaller_or_eq(int lhs, int rhs) { return lhs <= rhs; }
int bigger_or_eq(int lhs, int rhs)  { return lhs >= rhs; }
int equal(int lhs, int rhs)   { return lhs == rhs; }
int not_equal(int lhs, int rhs)   { return lhs != rhs; }
