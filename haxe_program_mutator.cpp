#include <iostream>
#include <fstream>

#include "haxe_proto.pb.h"
#include "libprotobuf-mutator/src/mutator.h"

void proto_to_haxe(std::ostream &out, haxe_fuzzer::haxe_program *program) {
    out << "class HaxeFuzzTest { " << std::endl;
    out << "    static public function main() { " << std::endl;
    out << "        trace(\""<< program->seed() <<"\"); " << std::endl;
    out << "    }" << std::endl;
    out << "}" << std::endl;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        std::cout<<"Usage: haxe_program_mutator <input_proto_file> <output_proto_file> <output_haxe_src_file>"<<std::endl;
        return -1;
    }
    // TODO: Use a flag library.
    char *input_proto_file_path = argv[1];
    char *mutated_proto_file_path = argv[2];
    char *haxe_src_file_path = argv[3];

    // TODO: Maybe pass in seed as a parameter for determinism?
    srand(time(NULL));
    
    protobuf_mutator::Mutator mutator;
    mutator.Seed(rand());

    haxe_fuzzer::haxe_program program;

    std::ifstream haxe_proto_ifs(input_proto_file_path, std::ifstream::in);
    program.ParseFromIstream(&haxe_proto_ifs);
    haxe_proto_ifs.close();

    // TODO: Take proto_size_hint as a flag, or a config?
    mutator.Mutate(&program, 10000 /* PROTO_SIZE_HINT */);

    std::ofstream mutated_haxe_proto_ofs(mutated_proto_file_path, std::ofstream::out);
    program.SerializeToOstream(&mutated_haxe_proto_ofs);
    mutated_haxe_proto_ofs.close();

    std::ofstream haxe_src_ofs(haxe_src_file_path, std::ofstream::out);
    proto_to_haxe(haxe_src_ofs, &program);
    haxe_src_ofs.close();
}