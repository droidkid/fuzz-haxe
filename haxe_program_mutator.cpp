#include <iostream>

#include "haxe_proto.pb.h"
#include "libprotobuf-mutator/src/mutator.h"

int main() {
    protobuf_mutator::Mutator mutator;

    haxe_fuzzer::haxe_program program;
    mutator.Seed(1234);
    mutator.Mutate(&program, 10000);
    std::cout<<program.DebugString()<<std::endl;
    
}