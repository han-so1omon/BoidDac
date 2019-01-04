import subprocess
from config import *

# get stake periods
MONTH = 
QUARTER = 

# get all staked accounts
cmd = \
    'cleos --url' + URL + \
    ' get table ' + OWNER + \
    ' ' + user_account accounts

for acct in staked_accts:
    stake_period = 
    if stake_period == MONTH:
        cmd = \
            'cleos --url ' + URL + \
            ' push action ' + OWNER + \
            ' unstake \'{"_stake_account":"' + acct + \
            '"}\' -p ' + OWNER
        print(cmd)
        #subprocess.call(cmd, shell=True)
