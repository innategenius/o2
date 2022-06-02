import subprocess
import o2 as implementation


standard_path = 'implementation0.cpp'
transpiled_path = 'source.cpp'

executor = 'g++'


def execute(path_to_wy):

    with open(path_to_wy, 'r') as f:
        source = f.read()

    with open(standard_path, 'r') as f:
        standard = f.read()

    parsed_definitions, parsed_context = implementation.parse(source)
    standard_source_lines = standard.splitlines()

    print(source, '\n->')
    print(parsed_definitions)
    print(parsed_context)

    definition_destination_line = standard_source_lines.index('// definitions go here')

    parsed_definitions.splitlines()

    transpiled_source = '\n'.join(standard_source_lines[:definition_destination_line]) + parsed_definitions + '\n'.join(standard_source_lines[definition_destination_line:-1]) + parsed_context + '\n\n' + standard_source_lines[-1]

    with open(transpiled_path, 'w') as f:
        f.write(transpiled_source)

    subprocess.call([executor, transpiled_path])
    subprocess.call(['a.exe'])


execute('source.wy')
