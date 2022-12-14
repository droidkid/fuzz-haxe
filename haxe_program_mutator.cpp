#include <iostream>
#include <fstream>

#include "protos/haxe_proto.pb.h"
#include "libprotobuf-mutator/src/mutator.h"
#include "proto_to_haxe.h"


int main(int argc, char **argv) {
    if (argc < 3) {
        std::cout<<"Usage: haxe_program_mutator <input_proto_file> <output_proto_file> <output_haxe_src_file>"<<std::endl;
        std::cout<<"OR: haxe_program_mutator <output_proto_file> <output_haxe_src_file>"<<std::endl;
        return -1;
    }
    // TODO: Use a flag library.

    char *input_proto_file_path;
    char *mutated_proto_file_path;
    char *haxe_src_file_path;

    haxe_fuzzer::HaxeProgram program;

    if (argc == 4) {
        input_proto_file_path = argv[1];
        mutated_proto_file_path = argv[2];
        haxe_src_file_path = argv[3];

        std::ifstream haxe_proto_ifs(input_proto_file_path, std::ifstream::in);
        program.ParseFromIstream(&haxe_proto_ifs);
        haxe_proto_ifs.close();
    } else {
        mutated_proto_file_path = argv[1];
        haxe_src_file_path = argv[2];
    }

    // TODO: Maybe pass in seed as a parameter for determinism?
    srand(time(NULL));
    protobuf_mutator::Mutator mutator;
    mutator.Seed(rand());
    mutator.Mutate(&program, 100000000);

    std::ofstream mutated_haxe_proto_ofs(mutated_proto_file_path, std::ofstream::out);
    program.SerializeToOstream(&mutated_haxe_proto_ofs);
    mutated_haxe_proto_ofs.close();

    std::ofstream haxe_src_ofs(haxe_src_file_path, std::ofstream::out);
    proto_to_haxe(haxe_src_ofs, &program);
    haxe_src_ofs.close();
}
