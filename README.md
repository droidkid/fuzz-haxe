# Fuzzing Haxe

```
$ make haxe_program_mutator
$ python fuzz_haxe.py --campaign_dir=test_campaign

```

This approach to fuzzing Haxe is inspired by the following talk:
[Structure aware fuzzing - Réka Kovács - Meeting C++ 2018](https://www.youtube.com/watch?v=wTWNmOSKfD4)

WARNING: This is a work-in-progress.

## Dependencies

- absl-py (pip install absl-py)
- Hashlink (Install hashlink and make sure hl is on path)
- hxcpp, hxjvm
- protocol buffers (protoc)
- libprotobuf-mutator
