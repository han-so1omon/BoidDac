import subprocess
from config import *

# get all staked accounts
cmd = \
    'cleos --url' + URL + \
    ' get table ' + OWNER + \
    ' ' + user_account accounts

for acct in staked_accts:
    cmd = \
        'cleos --url ' + URL + \
        ' push action ' + OWNER + \
        ' unstake \'{"_stake_account":"' + acct + \
        '"}\' -p ' + OWNER
    print(cmd)
    #subprocess.call(cmd, shell=True)
