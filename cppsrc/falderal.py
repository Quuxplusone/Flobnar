#!/usr/bin/env python

import subprocess
import sys

readme = sys.argv[1]
with open(readme) as f:
    current_program = []
    for line in f.readlines():
        if line.startswith('    |'):
            current_program += [line[5:].rstrip()]
        elif line.startswith('    ='):
            expected_output = line[5:].strip()
            # Left-justify the input program
            while all(line.startswith(' ') for line in current_program):
                current_program = [line[1:] for line in current_program]
            # Dump it to a file
            with open('example.flobnar', 'w') as ex:
                ex.write('\n'.join(current_program))
            # Execute that file
            output = subprocess.check_output(['./flobnar', 'example.flobnar']).strip()
            if output != expected_output:
                print 'Executing the following test case:'
                for line in current_program:
                    print '    | %s' % line
                print '    = %s' % expected_output
                print 'Actual->', output
            else:
                print 'Success! Output was->', expected_output
        else:
            current_program = []
