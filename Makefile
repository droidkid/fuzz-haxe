clean:
	rm -rf protos
	rm -f haxe_program_mutator

gen-proto: haxe_proto.proto
	mkdir -p protos
	protoc -I=. haxe_proto.proto --cpp_out=protos/

haxe_program_mutator: haxe_program_mutator.cpp gen-proto
	g++ haxe_program_mutator.cpp haxe_proto.pb.cc -I/usr/local/include -I/usr/local/include/libprotobuf-mutator -lprotobuf -lprotobuf-mutator -o haxe_program_mutator
