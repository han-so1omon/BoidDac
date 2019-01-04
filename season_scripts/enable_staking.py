import subprocess
from config import *

cmd = \
    'cleos --url ' + URL + \
    ' push action ' + OWNER + \
    ' stakeperiod \'{"on_switch":"1"}\' -p ' + OWNER
print(cmd)
#subprocess.call(cmd, shell=True)
