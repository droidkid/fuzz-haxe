'''
A barebones test harness to fuzz the haxe transpiler.

The fuzzer will run for some executions, where each execution will
- Pick a random seed to mutate
- Fetch and mutate a random seed.
- - Mutation is done using libprotobuf-mutator
- Test the proto by
- - Converting the proto to a haxe program
- - Compiling the haxe program to different targets (C++, Javascript, HashLink..)
- - Running the different targets, and check if they exhibit the same output
- - If they don't, store the seed - this is a potential bug
- Add the mutated program to the list of seeds

TODO
- Need to figure out a way to be smart about seeds to save.

Why not use AFL/libfuzzer...? 
- AFL/libfuzzer requires coverage to guide its mutation. I haven't
out how to instrument the haxe compiler. 
- I'm not sure how to do differential fuzzing in AFL/libfuzzer 

This fuzzer is written as a script, some parts could definitely be wrapped
in classes. Maybe later, the scripts is a manageable size for now.
'''

# TODO: Implement writing out haxe_lang.proto to haxe program.
# TODO: Implement differential testing - outputs should match across all targets
# TODO: Build seeds from a directory of haxe programs.
# TODO: Add flags to control which targets to execute. (Cpp is slow to build)

from absl import app
from absl import flags
import random
import os
import subprocess

FLAGS = flags.FLAGS

flags.DEFINE_string('campaign_dir', None,
                    'Directory under which to run fuzzing campaign.')
flags.DEFINE_integer(
    'executions', 10, 'Number of haxe executions to perform.', lower_bound=1)

flags.mark_flag_as_required('campaign_dir')


def pick_next_seed():
    return random.getrandbits(32)


def write_haxe_program(seed):
    haxe_src_path = os.path.join(os.getcwd(), "HaxeFuzzTest.hx")
    haxe_program = [
        "class HaxeFuzzTest { ", 
        "   static public function main() {",
        "       trace(\"" + str(seed) + "!\");",
        "   }",
        "}"
        ];
    f = open(haxe_src_path, "w")
    f.write('\n'.join(haxe_program))
    f.close()
    return haxe_src_path;

# ==================== BUILD DEFINITIONS START =================== #
# All the build_<target>_methods assume the current working directory
# has a file called HaxeFuzzTest.hx.


def build_js_target():
    subprocess.run(["haxe", "--main", "HaxeFuzzTest", "--js", "HaxeFuzzTest.js" ])
    # TODO: Handle failures here

def build_cpp_target():
    subprocess.run(["haxe", "--main", "HaxeFuzzTest", "--cpp", "HaxeFuzzTestCpp" ])
    # TODO: Handle failures here

def build_hashlink_target():
    subprocess.run(["haxe", "--main", "HaxeFuzzTest", "--hl", "HaxeFuzzTest.hl" ])

# ==================== BUILD DEFINITIONS END =================== #

# ==================== RUN DEFINITIONS START =================== #
def run_js_target():
    subprocess.run(["node", "HaxeFuzzTest.js"])
    # TODO: Capture output. Handle failure here

def run_hashlink_target():
    subprocess.run(["hl", "HaxeFuzzTest.hl"])

def run_cpp_target():
    subprocess.run(["./HaxeFuzzTestCpp/HaxeFuzzTest" ])
    # TODO: Handle failures here


# ==================== RUN DEFINITIONS END =================== #

def test_execution(seed):
    write_haxe_program(seed);

    build_js_target();
    build_cpp_target();
    build_hashlink_target();

    run_js_target();
    run_cpp_target();
    run_hashlink_target();


def main(argv):
    CAMPAIGN_DIR = os.path.abspath(FLAGS.campaign_dir)
    CAMPAIGN_EXECUTIONS_DIR = os.path.join(FLAGS.campaign_dir, "executions")

    if os.path.exists(CAMPAIGN_DIR):
        print("The campaign directory already exists!")
        print("Use a new directory or delete it!")
        # TODO: Add a flag that lets you force continue.
        # TODO: Add a prompt to clear the directories and try again.
        # TODO: Add a way to continue a campaign.
        print("%s", os.path.abspath(FLAGS.campaign_dir))
        exit()

    print("Starting campaign in %s", CAMPAIGN_DIR)
    os.mkdir(CAMPAIGN_DIR)
    os.mkdir(CAMPAIGN_EXECUTIONS_DIR)

    execution_num = 0
    while (execution_num < FLAGS.executions):
        next_seed_to_mutate = pick_next_seed()

        EXECUTION_DIR = os.path.abspath(os.path.join(
            CAMPAIGN_EXECUTIONS_DIR, "%d" % execution_num))

        cur_dir = os.getcwd()
        os.mkdir(EXECUTION_DIR)
        os.chdir(EXECUTION_DIR)
        test_execution(next_seed_to_mutate)
        os.chdir(cur_dir)

        execution_num += 1


if __name__ == '__main__':
    app.run(main)
