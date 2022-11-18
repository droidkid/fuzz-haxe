# Fuzzing Haxe

TODO: Write a make file
```
$ protoc -I=. haxe_proto.proto --python_out=. --cpp_out=.

```

## Dependencies

- absl-py (pip install absl-py)
- Hashlink (Install hashlink and make sure hl is on path)
- hxcpp, hxjvm
- protocol buffers (protoc)
- libprotobuf-mutator
