import subprocess
import sys
from config import *
from accounts import *

# get all staked accounts
staked_accts = get_accounts(temp_filename=STAKED_ACCTS_FILE)
print('staked_accts:')
for acct in staked_accts:
    print(acct)
#    if ...:
#        staked_accts.append(acct)
sys.exit()

# claim rewards for all staked accounts
for acct in staked_accts():
    cmd = \
        'cleos --url ' + URL + \
        ' push action ' + OWNER + \
        ' claim \'{"_stake_account":"' + acct + \
        '"}\' -p ' + OWNER
    print(cmd)
    #subprocess.call(cmd, shell=True)


