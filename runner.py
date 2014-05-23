import os
import re
import subprocess
from timeit import default_timer
import multiprocessing
import pprint
import math


def run_solution(command, seed):
    try:
        start = default_timer()
        p = subprocess.Popen(
            'java -jar RectanglesAndHolesVis.jar -exec "{}" '
            '-novis -seed {}'.format(command, seed),
            shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        out, err = p.communicate()
        p.wait()
        assert p.returncode == 0

        result = {}
        for line in out.splitlines():
            var, _, value = line.strip().partition(' = ')
            if value:
                result[var.strip()] = eval(value)

        result['time'] = default_timer() - start

        assert result['Score'] > 0
        result['log_score'] = math.log(result['Score'])
        assert 'n' in result

        return result
    except Exception as e:
        print e
        raise Exception('seed={}, out={}, err={}'.format(seed, out, err))


def grouping_task(seed):
    sol = run_solution('./a.out', seed)
    return seed, sol['n']


def compile(release=False):
    if release:
        subprocess.check_call(
            'g++ --std=c++0x '
            '-W -Wall -Wno-sign-compare '
            '-O2 -s -pipe -mmmx -msse -msse2 -msse3 '
            'sol.cc', shell=True)
    else:
        subprocess.check_call('g++ --std=c++0x sol.cc', shell=True)


def main():
    compile()

    pool = multiprocessing.Pool(6)
    by_problem_type = {
        n:[] for n in range(100, 1001)}
    for seed, n in pool.imap_unordered(grouping_task, range(1, 20000)):
        if len(by_problem_type[n]) < 10:
            by_problem_type[n].append(seed)
        print seed, n

    with open('by_size.txt', 'w') as fout:
        pprint.pprint(by_problem_type, width=1000, stream=fout)
    shortest = min(map(len, by_problem_type.values()))
    print 'shortest', shortest


if __name__ == '__main__':
    main()
