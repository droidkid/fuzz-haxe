#ifndef PROTO_TO_HAXE_HPP
#define PROTO_TO_HAXE_HPP

#include "protos/haxe_proto.pb.h"
#include <ostream>

void proto_to_haxe(std::ostream&, haxe_fuzzer::HaxeProgram*);

#endif