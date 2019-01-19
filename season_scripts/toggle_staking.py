import subprocess
import argparse
from config import *
import sys

parser = argparse.ArgumentParser()
parser.add_argument(
    "-s",
    dest="on_switch",
    help="1/0 turn on/off stakebreak")
args = parser.parse_args()

try:
    on_switch = int(args.on_switch)
except:
    print('on_switch must be an int: currently it is a %s' % type(args.on_switch))
    print('correct example: python toggle_staking.py -s 1')
    sys.exit()

cmd = \
    'cleos --url ' + URL + \
    ' push action ' + OWNER + \
    ' stakebreak \'{"on_switch":"%d"}\' -p ' % on_switch + OWNER
print(cmd)
subprocess.call(cmd, shell=True)


