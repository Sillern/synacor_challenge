from __future__ import print_function
import sys
import os
import subprocess
import shutil


def try_compile(build_type, root, generator):
    '''Generate, make, and package the adas-component
    Raise subprocess.CalledProcessError on cmake error or IOError on IO
    failures
    Return True on when working as expecting
    '''

    component = 'synacor_machine'
    output_file = os.path.join('bin', component)
    cmake = os.environ.get('CMAKE', 'cmake')

    relative_path_to_root = os.path.relpath(root, os.getcwd())
    output_path = os.path.join(relative_path_to_root, 'build', component)

    command = [cmake,
               '-G', generator,
               '-DCMAKE_BUILD_TYPE=' + build_type,
               relative_path_to_root]

    subprocess.check_call(command)

    command = [cmake, '--build', '.',
               '--', '-v']

    subprocess.check_call(command)

    # Copy generated package to the defined output path
    shutil.copyfile(output_file, output_path)
    shutil.copystat(output_file, output_path)
    return True


def build(build_type, root, generator):
    '''Create a build directory and trigger a build
    Return False on error and True on success'''

    # Check build_type input
    if not(build_type == 'Debug' or build_type == 'Release'):
        print('Warning: Input build_type must be Debug or Release')
        print('Setting build_type to Release')
        build_type = 'Release'

    os.environ['CC'] = '/usr/bin/clang'
    os.environ['CXX'] = '/usr/bin/clang++'

    return_value = True
    builddir = os.path.join(root, 'build', 'target', generator)

    try:
        os.makedirs(builddir)
    except OSError:
        print('Build folder already exists')

    os.chdir(builddir)
    if not try_compile(build_type, root, generator):
        return_value = False
    os.chdir(root)

    return return_value


ROOT_DIR = os.path.abspath(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), '.'))


def main(root_dir=ROOT_DIR, generator='Ninja'):
    build_type = 'Release'
    if len(sys.argv) > 1:
        if sys.argv[1] == 'Debug':
            build_type = 'Debug'

    return 0 if build(build_type, root_dir, generator) else 1

if __name__ == '__main__':
    sys.exit(main())
