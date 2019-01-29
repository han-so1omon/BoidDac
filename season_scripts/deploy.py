from config import *
import os, sys
import subprocess

par_dir = \
    os.path.split(
        os.getcwd())[1]
correct_dir = 'season_scripts'
if par_dir != correct_dir:
    print('the deploy.py script must be run from the directory the script is in: ' + correct_dir)
    print('current directory is: ' + par_dir)
    sys.exit()
contract_dir = os.path.join('..', 'build')  # path containing contract's .wasm and .abi files


# undeploy the old contract
cmd = \
    'cleos --url ' + URL + \
    ' set contract ' + OWNER + ' -c'
print('un-deploy cmd:')
print(cmd)
subprocess.call(cmd, shell=True)

# deploy contract
cmd = \
    'cleos --url ' + URL + \
    ' set contract ' + OWNER + ' ' + contract_dir + \
    ' boidtoken.wasm boidtoken.abi -p ' + OWNER +'@active'
print('deploy cmd:')
print(cmd)
subprocess.call(cmd, shell=True)


