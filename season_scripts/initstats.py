from config import *
import os, sys
import subprocess


# initialize the stats table
cmd = \
    'cleos --url ' + URL + \
    ' push action ' + OWNER + ' initstats \'{}\' -p ' + OWNER
print('initstats cmd:')
print(cmd)
subprocess.call(cmd, shell=True)


