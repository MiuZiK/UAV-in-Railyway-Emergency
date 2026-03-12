#!/usr/bin/env python3

import subprocess
import time

#
# run_real.sh, yuntai.sh, odom.sh, offboard.sh
#
def run():
    # for i in range(30):
    #     print(f'{i}s sleep')
    #     time.sleep(1)
    scripts = ["odom.sh", "run_real.sh", "yuntai.sh", "offboard.sh"]
    terminals = []
    for script in scripts:
        subprocess.Popen([
            "gnome-terminal", "--", "bash", "-c",
            f"./{script}; exec bash"
        ])

        # subprocess.Popen(["x-terminal-emulator", "-e", f"bash -c '{script}; exec bash'"])
        # subprocess.Popen(["x-terminal-emulator", "-e", f"bash -c './{script}; exec bash'"])
        
        print(f'script:{script} opened')
        time.sleep(2)

    try:
        while True:
            time.sleep(10)
            print('start_sh.py alive...')
    except KeyboardInterrupt:
        pass

if __name__ == '__main__':
    run()