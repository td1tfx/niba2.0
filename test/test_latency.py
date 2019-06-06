#!/usr/bin/env python3

# Run following to clean up db
# delete from character_equipped_magic where 1=1;
# delete from player_character where 1=1;
# delete from user_id where 1=1;
# delete from player_magic where 1=1;

import subprocess
import os
import time
import sys

dir_path = os.path.abspath(os.path.join(
    os.path.dirname(__file__), '..', 'build'))

# TODO: make it argparse
clients = 100
if len(sys.argv) > 1:
    clients = int(sys.argv[1])

server = subprocess.Popen([dir_path + '/niba-server', 'config.json'], cwd=dir_path)
time.sleep(1)


def run_client(input_bytes):
    proc = subprocess.Popen([dir_path + '/niba-client'],
                            stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE,
                            universal_newlines=True,
                            cwd=dir_path)
    proc.stdin.write(input_bytes)
    proc.stdin.flush()
    return proc


try:
    procs = []
    stdouts = []
    stderrs = []

    input_bytes = 'register niba%d doctorniba\nlogin niba%d doctorniba\ncreate nibadan%d m 1 1 2 1\nsend nibadan helloworld\nlearnmagic 1\nlearnmagic 2\nfusemagic 2 1\nreordermagic 2\nfight 1\nfight 1\nfight 1\nexit\n'
    # for j in range(100):
    #     input_bytes += b'fight 1\n'

    start = time.time()

    niba0 = run_client('register niba doctorniba\nlogin niba doctorniba\ncreate nibadan m 1 1 2 1\ntimeout 10\nexit\n')

    for i in range(clients):
        proc = run_client(input_bytes % (i, i, i))
        procs.append(proc)

    for proc in procs:
        stdout, stderr = proc.communicate()
        # stderr are exclusively response timings
        stdouts.append(stdout)
        stderrs.append(stderr)

    end = time.time()

    total = 0
    for stdout, stderr in zip(stdouts, stderrs):
        # print(stderr)
        time = 0
        requests = 0
        try:
            for latency in stderr.split('\n'):
                if latency == '':
                    continue
                time += float(latency)
                requests += 1
            avg = time / requests
            # print(avg, 'ms')
            total += avg
        except Exception as e:
            print('encountered exception')
            print(stdout)
            print(stderr)
            # raise e

    print('average wait per request', total / clients, 'ms')
    print('overall', end - start, 's')
    
except Exception as e:
    print(e)

finally:
    niba0.terminate()
    server.kill()
