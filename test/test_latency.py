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
                            stdout=subprocess.DEVNULL,
                            stderr=subprocess.PIPE,
                            cwd=dir_path)
    proc.stdin.write(input_bytes)
    proc.stdin.flush()
    return proc


try:
    procs = []
    stderrs = []

    input_bytes = b'register niba%d doctorniba\nlogin niba%d doctorniba\ncreate nibadan%d m 1 1 2 1\nsend nibadan helloworld\nlearnmagic 1\nlearnmagic 2\nfusemagic 2 1\nreordermagic 2\nfight 1\nfight 1\nfight 1\nexit\n'
    # for j in range(100):
    #     input_bytes += b'fight 1\n'

    start = time.time()

    niba0 = run_client(b'register niba doctorniba\nlogin niba doctorniba\ncreate nibadan m 1 1 2 1\ntimeout 10\nexit\n')

    for i in range(clients):
        proc = run_client(input_bytes % (i, i, i))
        procs.append(proc)

    for proc in procs:
        _, stderr = proc.communicate()
        # stderr are exclusively response timings
        stderrs.append(stderr)

    end = time.time()

    total = 0
    for stderr in stderrs:
        # print(stderr)
        time = 0
        requests = 0
        try:
            for latency in stderr.split(b'\n'):
                if latency == b'':
                    continue
                time += float(latency)
                requests += 1
            avg = time / requests
            # print(avg, 'ms')
            total += avg
        except Exception as e:
            print(stderr)
            raise e

    print('average wait per request', total / clients, 'ms')
    print('overall', end - start, 's')

    niba0.terminate()

except Exception as e:
    print(e)

finally:
    server.kill()
