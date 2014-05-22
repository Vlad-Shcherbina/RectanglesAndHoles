import sys
import multiprocessing

import runner
import run_db


def worker(task):
    return runner.run_solution(*task)


def main():
    runner.compile()

    with open('by_size.txt') as fin:
        by_size = eval(fin.read())

    sizes = range(100, 105) + range(500, 505) + range(996, 1001)

    tasks = [('./a.out', by_size[n][i]) for i in range(3) for n in sizes]

    map = multiprocessing.Pool(5).imap

    with run_db.RunRecorder() as run:
        for result in map(worker, tasks):
            print result['n'], result['log_score']
            run.add_result(result)
        run.save()


if __name__ == '__main__':
    main()
