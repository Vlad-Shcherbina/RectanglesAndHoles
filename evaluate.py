import sys
import multiprocessing

import runner
import run_db


def worker(task):
    return runner.run_solution(*task)


def main():
    runner.compile(release=True)

    with open('by_size.txt') as fin:
        by_size = eval(fin.read())

    sizes = range(100, 1001, 100)

    tasks = [('./a.out', by_size[n][i]) for i in range(5) for n in sizes]

    map = multiprocessing.Pool(15).imap

    with run_db.RunRecorder() as run:
        for result in map(worker, tasks):
            print result['n'], result['log_score']
            run.add_result(result)
        run.save()


if __name__ == '__main__':
    main()
