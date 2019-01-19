import subprocess
import sys
from config import *


cmd = 'cleos wallet open -n ' + WALLET_NAME
print(cmd)
subprocess.call(cmd, shell=True)

f = open(WALLET_PASSWORD, 'r')
wallet_password = f.readlines()[0][:-1]

cmd = \
    'cleos wallet unlock -n ' + WALLET_NAME + \
    ' --password ' + wallet_password
print(cmd)
subprocess.call(cmd, shell=True)

