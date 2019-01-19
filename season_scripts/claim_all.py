import subprocess
import sys
from config import *
from accounts import *

# get all staked accounts
accts = get_accounts()
staked_accts = []
print(len(accts))
sys.exit()
#for acct in accts:
#    if ...:
#        staked_accts.append(acct)

# claim rewards for all staked accounts
for acct in staked_accts():
    cmd = \
        'cleos --url ' + URL + \
        ' push action ' + OWNER + \
        ' claim \'{"_stake_account":"' + acct + \
        '"}\' -p ' + OWNER
    print(cmd)
    #subprocess.call(cmd, shell=True)


