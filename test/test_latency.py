import subprocess
import os
import time

dir_path = os.path.abspath(os.path.join(os.path.dirname( __file__ ), '..', 'build'))

# TODO: make it argparse
# looks like the runtime/system is restricting to ~512 open handles per process on my wsl + arch + python3.7 setup
CLIENTS = 2000

server = subprocess.Popen([dir_path + '/niba-server'], cwd=dir_path)
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
    input_bytes = b'register niba%d doctorniba\nlogin niba%d doctorniba\ncreate nibadan%d 0 1 1 2 1\nfight 1\nexit\n'
    # for j in range(100):
    #     input_bytes += b'fight 1\n'

    start = time.time()

    for i in range(CLIENTS):
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
        for latency in stderr.split(b'\n'):
            if latency == b'':
                continue
            time += float(latency)
            requests += 1
        avg = time / requests
        # print(avg, 'ms')
        total += avg

    print('average wait per request', total / CLIENTS, 'ms')
    print('overall', end - start, 's')

except Exception as e:
    print(e)

finally:
    server.kill()
