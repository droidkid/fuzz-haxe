//==-- proto_to_haxe.cpp - Protobuf-Haxe conversion --------------------------==//
//
//===----------------------------------------------------------------------===//
//
// Implements functions for converting between protobufs and Haxe.
// Forked from https://github.com/llvm-mirror/clang/blob/master/tools/clang-fuzzer/proto-to-cxx/proto_to_cxx.cpp
//
//===----------------------------------------------------------------------===//
#include "protos/haxe_proto.pb.h"
#include <ostream>

using namespace haxe_fuzzer;

// Forward decls.
std::ostream &operator<<(std::ostream &os, const StatementSeq &x);
std::ostream &operator<<(std::ostream &os, const VarRef &x);
std::ostream &operator<<(std::ostream &os, const Lvalue &x);
std::ostream &operator<<(std::ostream &os, const Const &x);
std::ostream &operator<<(std::ostream &os, const BoolConst &x);
std::ostream &operator<<(std::ostream &os, const ArithExp &x);
std::ostream &operator<<(std::ostream &os, const CompareExp &x);
std::ostream &operator<<(std::ostream &os, const LogicalExp &x);
std::ostream &operator<<(std::ostream &os, const Condition &x);
std::ostream &operator<<(std::ostream &os, const Rvalue &x);
std::ostream &operator<<(std::ostream &os, const AssignmentStatement &x);


std::ostream &operator<<(std::ostream &os, const VarRef &x) {
  return os << "a[" << (static_cast<uint32_t>(x.varnum()) % 100) << "]";
}

std::ostream &operator<<(std::ostream &os, const Lvalue &x) {
  return os << x.varref();
}

std::ostream &operator<<(std::ostream &os, const Const &x) {
  return os << "(" << x.val() << ")";
}

std::ostream &operator<<(std::ostream &os, const BoolConst &x) {
  switch (x.value()) {
    case BoolConst::FALSE: os<<"false"; break;
    case BoolConst::TRUE: os<<"true"; break;
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const ArithExp &x) {
  os << "(" << x.left();
  switch (x.op()) {
    case ArithExp::PLUS: os << "+"; break;
    case ArithExp::MINUS: os << "-"; break;
    case ArithExp::MUL: os << "*"; break;
    case ArithExp::DIV: os << "/"; break;
    case ArithExp::MOD: os << "%"; break;
    case ArithExp::XOR: os << "^"; break;
  }
  return os << x.right() << ")";
}

std::ostream &operator<<(std::ostream &os, const CompareExp &x) {
  os << "(" << x.left();
  switch (x.op()) {
    case CompareExp::EQ: os << "=="; break;
    case CompareExp::NE: os << "!="; break;
    case CompareExp::LE: os << "<="; break;
    case CompareExp::GE: os << ">="; break;
    case CompareExp::LT: os << "<"; break;
    case CompareExp::GT: os << ">"; break;
  }
  return os << x.right() << ")";
}

std::ostream &operator<<(std::ostream &os, const LogicalExp &x) {
  os << "(" << x.left();
  switch (x.op()) {
    case LogicalExp::AND: os << "&&"; break;
    case LogicalExp::OR: os << "||"; break;
  }
  return os << x.right() << ")";
}

std::ostream &operator<<(std::ostream &os, const Condition &x) {
  if (x.has_bool_const()) return os << x.bool_const();
  if (x.has_logical_exp()) return os << x.logical_exp();
  if (x.has_compare_exp()) return os << x.compare_exp();
  return os << "false";
}


std::ostream &operator<<(std::ostream &os, const Rvalue &x) {
    if (x.has_varref()) return os << x.varref();
    if (x.has_cons())   return os << x.cons();
    if (x.has_arith_exp())  return os << x.arith_exp();
    return os << "1";
}

std::ostream &operator<<(std::ostream &os, const AssignmentStatement &x) {
  return os << x.lvalue() << "=" << x.rvalue() << ";\n";
}

std::ostream &operator<<(std::ostream &os, const IfElse &x) {
  return os << "if (" << x.cond() << "){\n"
            << x.if_body() << "} else { \n"
            << x.else_body() << "}\n";
}

std::ostream &operator<<(std::ostream &os, const While &x) {
  return os << "while (" << x.cond() << "){\n" << x.body() << "}\n";
}

std::ostream &operator<<(std::ostream &os, const Statement &x) {
  if (x.has_assignment()) return os << x.assignment();
  if (x.has_ifelse())     return os << x.ifelse();
  if (x.has_while_loop()) return os << x.while_loop();
  return os;
}

std::ostream &operator<<(std::ostream &os, const StatementSeq &x) {
  for (auto &st : x.statements()) os << st;
  return os;
}

void proto_to_haxe(std::ostream &out, HaxeProgram *program) {
    out << "using Lambda;" << std::endl;
    out << "class HaxeFuzzTest { " << std::endl;
    out << "static public function main() { " << std::endl;
    out << "var a:Array<Int> = [for (i in  0...100) i];" << std::endl;

    out << program->main_body() << std::endl;

    // TODO: Come up with a better hash function
    out << "var hash_fn = function(num, h) return h ^= num;\n";;
    out << "trace(a.fold(hash_fn, 0));\n";
    out << "trace(a.fold(Math.max, a[0]));\n";
    out << "trace(a.fold(Math.min, a[0]));\n";
    out << "}" << std::endl;
    out << "}" << std::endl;
}
