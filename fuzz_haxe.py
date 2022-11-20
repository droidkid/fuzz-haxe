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
figured out how to instrument the haxe compiler. 
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



def prep_next_execution(exec_num, campaign_execution_dir):
    exec_dir = os.path.abspath(os.path.join(
        campaign_execution_dir, "%d" % exec_num))
    os.mkdir(exec_dir)
    mutator = ["./haxe_program_mutator"]

    if exec_num > 0:
        rand_execution_num = random.randrange(exec_num)
        input_proto_path = os.path.join(
            campaign_execution_dir, str(rand_execution_num), "haxe_program.pb")
        mutator.append(input_proto_path)

    mutated_proto_path = os.path.join(exec_dir, "haxe_program.pb")
    haxe_src_path = os.path.join(exec_dir, "HaxeFuzzTest.hx")
    mutator = mutator + [mutated_proto_path, haxe_src_path]

    subprocess.run(mutator)

    return exec_dir 

# ==================== BUILD DEFINITIONS START =================== #
# All the build_<target>_methods assume the current working directory
# has a file called HaxeFuzzTest.hx.


def build_target(args):
    build_result = {}
    try:
        build_result['output'] = subprocess.check_output(args, timeout=30)
        build_result['status'] = 'OK'
    except subprocess.CalledProcessError as e:
        build_result['status'] = 'BUILD_NZ_ERROR'
        build_result['output'] = str(e)
    except subprocess.TimeoutExpired as e:
        build_result['status'] = 'BUILD_TIME_OUT'
        build_result['output'] = str(e)
    return build_result


def build_nodejs_target():
    return build_target(["haxe", "-lib", "hxnodejs", "--main", "HaxeFuzzTest",
                         "--js", "HaxeFuzzTest.js"])


def build_cpp_target():
    return build_target(["haxe", "--main", "HaxeFuzzTest",
                         "--cpp", "HaxeFuzzTestCpp"])


def build_hashlink_target():
    return build_target(["haxe", "--main", "HaxeFuzzTest",
                         "--hl", "HaxeFuzzTest.hl"])

# ==================== BUILD DEFINITIONS END =================== #

# ==================== RUN DEFINITIONS START =================== #


def run_target(args):
    target_exec_result = {}
    try:
        output = subprocess.check_output(args, timeout=1)
        target_exec_result['status'] = 'OK'
        target_exec_result['output'] = output
    except subprocess.TimeoutExpired as e:
        target_exec_result['status'] = 'TIME_OUT'
        target_exec_result['output'] = str(e)
    except subprocess.CalledProcessError as e:
        target_exec_result['status'] = 'NZ_RETURN'
        target_exec_result['output'] = str(e)
    return target_exec_result


def run_nodejs_target():
    return run_target(["node", "HaxeFuzzTest.js"])


def run_hashlink_target():
    return run_target(["hl", "HaxeFuzzTest.hl"])


def run_cpp_target():
    return run_target(["./HaxeFuzzTestCpp/HaxeFuzzTest"])

# ==================== RUN DEFINITIONS END =================== #


TARGETS = {
    "nodejs": {
        "build_fn": build_nodejs_target,
        "run_fn": run_nodejs_target
    },
    "hashlink_build": {
        "build_fn": build_hashlink_target,
        "run_fn": run_hashlink_target,
    }
}


def analyze_result(exec_result):
    analysis = []

    # if any of the build targets fail, it is interesting.
    # This most likely means our mutator is not generating valid haxe code.
    # Or there's a bug in the haxe transpiler...
    for target in TARGETS:
        if exec_result[target]['build']['status'] != 'OK':
            analysis.append(target + ' compilation failed!')

    # For the builds that passed, check that they have all the same status
    run_status_list = [exec_result[target]['run']['status']
                       for target in TARGETS if exec_result[target]['build']['status'] == 'OK']
    deduped_run_status = list(dict.fromkeys(run_status_list))
    if (len(deduped_run_status) > 1):
        analysis.append('Different targets had different run status!')

    # If all the runs had a OK status, check that their output is same
    if len(deduped_run_status) == 1 and deduped_run_status[0] == 'OK':
        run_output_list = [exec_result[target]['run']['output']
                           for target in TARGETS
                           if exec_result[target]['build']['status'] == 'OK' and
                           exec_result[target]['run']['status'] == 'OK']
        deduped_run_output = list(dict.fromkeys(run_output_list))
        if len(deduped_run_output) > 1:
            analysis.append('Different targets had different outputs!')

    return analysis
        


def run_exec(exec_dir, capture_dir):
    cur_dir = os.getcwd()
    os.chdir(exec_dir)

    exec_result = {}

    for target in TARGETS:
        exec_result[target] = {}
        exec_result[target]['build'] = TARGETS[target]['build_fn']()
        if exec_result[target]['build']['status'] == 'OK':
            exec_result[target]['run'] = TARGETS[target]['run_fn']()

    exec_result['analysis'] = analyze_result(exec_result)

    os.chdir(cur_dir)

    if (len(exec_result['analysis']) > 0):
        subprocess.call(["cp", "-r", exec_dir, capture_dir])
    return exec_result


def main(argv):
    CAMPAIGN_DIR = os.path.abspath(FLAGS.campaign_dir)
    CAMPAIGN_EXECUTIONS_DIR = os.path.join(FLAGS.campaign_dir, "executions")
    CAPTURED_EXECUTIONS_DIR = os.path.join(FLAGS.campaign_dir, "captured")

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
    os.mkdir(CAPTURED_EXECUTIONS_DIR)

    LOG = open(os.path.join(CAMPAIGN_DIR, "LOG.txt"), 'w+', encoding='utf-8')

    exec_num = 0
    exec_results = {}
    while (exec_num < FLAGS.executions):
        exec_dir = prep_next_execution(exec_num, CAMPAIGN_EXECUTIONS_DIR)
        exec_results[exec_num] = run_exec(exec_dir, CAPTURED_EXECUTIONS_DIR)
        LOG.write(str(exec_num) + ": " + str(exec_results[exec_num]) + "\n")
        LOG.flush()
        exec_num += 1

    LOG.close()


if __name__ == '__main__':
    app.run(main)
