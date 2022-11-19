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
std::ostream &operator<<(std::ostream &os, const BinaryOp &x);
std::ostream &operator<<(std::ostream &os, const StatementSeq &x);

std::ostream &operator<<(std::ostream &os, const Const &x) {
  return os << "(" << x.val() << ")";
}
std::ostream &operator<<(std::ostream &os, const VarRef &x) {
  return os << "a[" << (static_cast<uint32_t>(x.varnum()) % 100) << "]";
}
std::ostream &operator<<(std::ostream &os, const Lvalue &x) {
  return os << x.varref();
}
std::ostream &operator<<(std::ostream &os, const Rvalue &x) {
    if (x.has_varref()) return os << x.varref();
    if (x.has_cons())   return os << x.cons();
    if (x.has_binop())  return os << x.binop();
    return os << "1";
}
std::ostream &operator<<(std::ostream &os, const BinaryOp &x) {
  os << "(" << x.left();
  switch (x.op()) {
    case BinaryOp::PLUS: os << "+"; break;
    case BinaryOp::MINUS: os << "-"; break;
    case BinaryOp::MUL: os << "*"; break;
    case BinaryOp::DIV: os << "/"; break;
    case BinaryOp::MOD: os << "%"; break;
    case BinaryOp::XOR: os << "^"; break;
    case BinaryOp::AND: os << "&"; break;
    case BinaryOp::OR: os << "|"; break;
    case BinaryOp::EQ: os << "=="; break;
    case BinaryOp::NE: os << "!="; break;
    case BinaryOp::LE: os << "<="; break;
    case BinaryOp::GE: os << ">="; break;
    case BinaryOp::LT: os << "<"; break;
    case BinaryOp::GT: os << ">"; break;
  }
  return os << x.right() << ")";
}

std::ostream &operator<<(std::ostream &os, const AssignmentStatement &x) {
  return os << x.lvalue() << "=" << x.rvalue() << ";\n";
}

std::ostream &operator<<(std::ostream &os, const IfElse &x) {
  return os << "if (" << x.cond() << " != 0){\n"
            << x.if_body() << "} else { \n"
            << x.else_body() << "}\n";
}

std::ostream &operator<<(std::ostream &os, const While &x) {
  return os << "while (" << x.cond() << " != 0){\n" << x.body() << "}\n";
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
    out << "class HaxeFuzzTest { " << std::endl;
    out << "static public function main() { " << std::endl;
    out << "var a:Array<Int> = [for (i in  0...100) i];" << std::endl;
    out << program->main_body() << std::endl;
    out << "}" << std::endl;
    out << "}" << std::endl;
}
