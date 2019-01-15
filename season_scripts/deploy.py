from config import *
import os, sys

working_dir = os.path.abspath(__file__)
correct_dir = os.path.realpath(__file__)
print(working_dir)
print(correct_dir)
print(sys.path[0])
#if working_dir != correct_dir:
#    print('the deploy.py script must be run from the directory the script is in: %s' % correct_dir)
#    print('current directory is: %s' % working_dir)
#    sys.exit()

sys.exit()

# deploy contract
contract_dir = os.path.join('..', 'build')  # path containing contract's .wasm and .abi files
cmd = \
    'cleos --url ' + URL + \
    ' set contract ' + OWNER + ' ' + contract_dir + \
    ' boidtoken.wasm boidtoken.abi -p ' + OWNER +'@active'
subprocess.call(cmd, shell=True)

# initialize the stats table
cmd = \
    'cleos --url ' + URL + \
    ' push action ' + OWNER + ' initstats \'{}\' -p ' + OWNER
subprocess.call(cmd, shell=True)

# set 'running' to 'on' in the contract
cmd = \
    'cleos --url ' + URL + \
    ' push action ' + OWNER + \
    ' running \'{"onswitch":"1"}\' -p ' + OWNER
subprocess.call(cmd, shell=True)

