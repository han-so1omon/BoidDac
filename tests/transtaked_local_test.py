import eosfactory.eosf as eosf
import eosfactory
import sys, os
import json
import argparse
import subprocess
import time
import numpy as np
import pandas as pd
pd.set_option('display.width', 500)
pd.set_option('display.max_rows',10)
pd.set_option('display.float_format', '{:.3f}'.format)
#pd.options.display.float_format = '${:,.2f}'.format

################################# Test variables #########################################

TEST_DURATION   = 8  # measured in weeks
INIT_BOIDTOKENS = 10000000  # initial boid tokens given to each account (must be less than 1/4th of max supply (1000000000 BOID))
INIT_BOIDPOWER  = 1000000.0  # 2300.5  # initial boid power given to each account
INIT_BOIDSTAKE  = 10000000.0  # initial boid tokens staked by each account (must be <= INIT_BOIDTOKENS)

############# Must also modify in boidtoken.hpp ##############
# TESTING Speeds Only
WEEK_WAIT    = 1

##############################################################

BOID_TOKEN_CONTRACT_PATH = \
     os.path.abspath(
         os.path.join(
             os.path.dirname(os.path.abspath(__file__)),
             '..'))

##########################################################################################



# @param account  The account to set/delete a permission authority for
# @param permission  The permission name to set/delete an authority for
# @param authority  NULL, public key, JSON string, or filename defining the authority
# @param parent  The permission name of this parents permission (Defaults to "active")
def setAccountPermission(account, permission, authority, parent, json=False, code=False):
    if json: json = '--json'
    else: json = ''
    if code: code = '--add-code'
    else: code = ''
    permissionCmd =\
        'cleos set account permission {0} {1} {2} {3} -p {0}@active {4}'.format(
                        account, permission, authority, parent, json)
    subprocess.call(permissionCmd, shell=True)


# parse json of account balance
def getBalance(x):
    if len(x.json['rows']) > 0:
        return float(x.json['rows'][0]['balance'].split()[0])
    else:
        return 0

# parse json of stakes table
def getStakeParams(x):
    ret = {}
    for i in range(len(x.json['rows'])):
        ret[x.json['rows'][i]['stake_account']] = \
            {
             'auto_stake': x.json['rows'][i]['auto_stake'],
             'staked': x.json['rows'][i]['staked']
            }
    return ret

# return True or False if acct has staked tokens or not
def acct_has_staked_tokens(contract, acct):
    stake_params = getStakeParams(contract.table('stakes', master))
    return str(acct) in stake_params.keys()

# return the number of staked tokens
def num_staked_tokens(contract, acct):
    stake_params = getStakeParams(contract.table('stakes', master))
    if str(acct) in stake_params.keys():
        return float(stake_params[str(acct)]['staked'].split()[0])
    return 0.0

# contract action calls
def transfer(contract, from_acct, to_acct, quantity):
    contract.push_action(
        'transfer',
        {
            'from':from_acct,
            'to':to_acct,
            'quantity':quantity,
            'memo':'memo'
        }, permission=[from_acct]
    )
def transtaked(contract, to_acct, quantity, perm):
    contract.push_action(
        'transtaked',
        {
            'to':to_acct,
            'quantity':quantity,
            'memo':'memo'
        }, permission=[perm]
    )
def issue(contract, to_acct, quantity, perm):
    contract.push_action(
            'issue',
            {
                'to': to_acct,
                'quantity': quantity,
                'memo': "memo"
            }, permission=[perm]
        )
def stake(contract, acct, quantity):
    contract.push_action(
        'stake',
        {
            '_stake_account': acct,
            '_staked': quantity
        }, permission=[acct]
    )
def unstake(contract, acct, quantity):
    contract.push_action(
        'unstake',
        {
            '_stake_account': acct,
            'quantity': quantity
        }, permission=[acct]
    )
def stakebreak(contract, on_switch, acct):
    print('\nsetting stakebreak to %s' % on_switch)
    contract.push_action(  # stakebreak - activate/deactivate staking for users
        'stakebreak',
        {
            'on_switch': on_switch,
        }, [acct])



if __name__ == '__main__':

    # determine if we want to
    # save the test data to a csv
    # build the contracts
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-s","--save",
        action="store_true",
        help="save test data to csv file in ./results")
    parser.add_argument(
        "-b","--build",
        action="store_true",
        help="build new contract ABIs")
    args = parser.parse_args()

    # start single-node local testnet
    eosf.reset()

    # create master account from which
    # other account can be created
    # accessed via global variable: master
    w = eosf.wallet.Wallet()
    eosf.create_master_account('master')

    # Create contract owner account: boid_token
    eosf.create_account('boid_token', master, account_name='boid.token')

    # acct1 does monthly stakes and acct2 does quarterly stakes
    eosf.create_account('acct1',      master, account_name='account1')
    eosf.create_account('acct2',      master, account_name='account2')
    accts = [acct1, acct2]

    # make build directory if it does not exist
    build_dir = os.path.join(BOID_TOKEN_CONTRACT_PATH, 'build')
    if not os.path.exists(build_dir):
        os.mkdir(build_dir)

    # create reference to the token staking contract
    # build and deploy the contracts on the testnet
    contract = eosf.Contract(boid_token, BOID_TOKEN_CONTRACT_PATH)
    if args.build:
        contract.build()
    contract.deploy()


    ############# now we can call actions ##############
    ################# from the contract ################
    print('\n')

    print('\nCreate tokens and set boid_token account as issuer of BOID')
    contract.push_action(
        'create',
        {
            'issuer': boid_token,
            'maximum_supply': '1000000000.0000 BOID'
        }, [boid_token])
    contract.push_action(
        'initstats',
        '{}', [boid_token])

    print('\nissue tokens to accts')
    contract.push_action('issue',
        {'to': boid_token, 'quantity': '%.4f BOID' % INIT_BOIDTOKENS, 'memo': 'memo'},
        [boid_token])
    contract.push_action('issue',
        {'to': acct1, 'quantity': '2000.0000 BOID', 'memo': 'memo'},
        [boid_token])
    contract.push_action('issue',
        {'to': acct2, 'quantity': '2000000.0000 BOID', 'memo': 'memo'},
        [boid_token])


    stakebreak(contract, "1", boid_token)
    stake(contract, boid_token, '5000000.0000 BOID')
    stakebreak(contract, "0", boid_token)

    print('\n---------------- Transtaked Tests: -----------------\n')

    print('Try to transtaked with someone other than issuer')
    try:
        transtaked(
            contract,
            acct1,
            '100000.0000 BOID',
            acct2)
        print('Test Failed. This action should have failed.')
        eosf.stop()
        sys.exit()
    except eosfactory.core.errors.Error as e:
        print(e)
        print('Test Passed. This action failed, as it should.\n')

    print('Try to transtaked more than issuer has in balance')
    try:
        transtaked(
            contract,
            acct1,
            '%.4f BOID' % (2 * INIT_BOIDTOKENS),
            boid_token)
        print('Test Failed. This action should have failed.')
        eosf.stop()
        sys.exit()
    except eosfactory.core.errors.Error as e:
        print(e)
        print('Test Passed. This action failed, as it should.\n')

    print('Try to transtaked more than issuer has in unstaked balance')
    try:
        transtaked(
            contract,
            acct1,
            '7500000.0000 BOID',
            boid_token)
        print('Test Failed. This action should have failed.')
        eosf.stop()
        sys.exit()
    except eosfactory.core.errors.Error as e:
        print(e)
        print('Test Passed. This action failed, as it should.\n')

    print('Try to transtaked less than min_stake')
    try:
        transtaked(
            contract,
            acct1,
            '1000.0000 BOID',
            boid_token)
        print('Test Failed. This action should have failed.')
        eosf.stop()
        sys.exit()
    except eosfactory.core.errors.Error as e:
        print(e)
        print('Test Passed. This action failed, as it should.\n')

    print('Try a valid transtaked.')
    try:
        balance = getBalance(contract.table("accounts", boid_token))
        print('boid_token balance: %s' % balance)

        balance = getBalance(contract.table("accounts", acct1))
        print('acct1 balance: %s' % balance)

        stake_params = getStakeParams(contract.table('stakes', boid_token))
        print('stake_params: %s' % stake_params)

        transtaked(
            contract,
            acct1,
            '100000.0000 BOID',
            boid_token)

        balance = getBalance(contract.table("accounts", boid_token))
        print('boid_token balance: %s' % balance)

        balance = getBalance(contract.table("accounts", acct1))
        print('acct1 balance: %s' % balance)

        stake_params = getStakeParams(contract.table('stakes', boid_token))
        print('stake_params: %s' % stake_params)
    
        print('Test Passed.\n')
    except eosfactory.core.errors.Error as e:
        print(e)
        print('Test Failed. This action should not have failed.\n')
        eosf.stop()
        sys.exit()


    print('\n-------------- Transtaked Tests Complete ----------------------\n')

    # stop the testnet and exit python
    eosf.stop()
    sys.exit()
