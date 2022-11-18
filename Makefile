clean:
	rm *.pb.cc
	rm *.pb.h
	rm *_pb2.py
	rm haxe_program_mutator

gen-proto: haxe_proto.proto
	protoc -I=. haxe_proto.proto --python_out=. --cpp_out=.

haxe_program_mutator: haxe_program_mutator.cpp gen-proto
	g++ haxe_program_mutator.cpp haxe_proto.pb.cc -I/usr/local/include -I/usr/local/include/libprotobuf-mutator -lprotobuf -lprotobuf-mutator -o haxe_program_mutator
