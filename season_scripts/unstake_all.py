import subprocess
import sys
from config import *
from accounts import *


# # get all staked accounts
# cmd = \
#     'cleos --url' + URL + \
#     ' get table ' + OWNER + \
#     ' ' + user_account accounts

def get_accts_to_unstake():

    ['/usr/local/bin/cleos', '--url', 'http://127.0.0.1:8888', 'get', 'table', 'boid.token', 'boid.token', 'stakes', '--limit', '10']
    limit = 10000
    cmd = 'cleos --url ' + URL + ' get table ' + OWNER + ' ' + OWNER + ' stakes --limit %d' % limit
    print('cmd')
    print(cmd)
    subprocess.run(cmd, shell=True)

    return []

    # find accounts with autostake = false
    accts_to_unstake = []
    for acct in staked_accts:
        auto_stake = False
        if not auto_stake:
            accts_to_unstake.append(acct)


# get all staked accounts
staked_accts = get_accounts(temp_filename=STAKED_ACCTS_FILE)
print('staked_accts:')
for acct in staked_accts:
    print(acct)

accts_to_unstake = get_accts_to_unstake()
sys.exit()


# 
for acct in accts_to_unstake:
    cmd = \
        'cleos --url ' + URL + \
        ' push action ' + OWNER + \
        ' unstake \'{"_stake_account":"' + acct + \
        '"}\' -p ' + OWNER
    print(cmd)
    #subprocess.call(cmd, shell=True)

