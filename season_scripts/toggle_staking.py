import subprocess
from config import *

cmd = \
    'cleos --url ' + URL + \
    ' push action ' + OWNER + \
    ' stakeperiod \'{"on_switch":"0"}\' -p ' + OWNER
subprocess.call(cmd, shell=True)
