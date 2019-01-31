import eosfactory.eosf as eosf
from eosfactory.eosf import *
import sys, os
import json
import argparse
#import subprocess
#import time
import numpy as np
import pandas as pd



BOID_TOKEN_CONTRACT_PATH = \
     os.path.abspath(
         os.path.join(
             os.path.dirname(os.path.abspath(__file__)),
             '..'))

# default min_stake = 100000.0
_STAKE1 = 50000.0  # stake less than minimum stake amount (w/ nothing initially staked)
_STAKE2 = 100000000.0  # stake more than account balance amount (w/ nothing initially staked)
_STAKE3 = 150000.0  # stake a valid amount (w/ nothing initially staked)
_STAKE4 = 100000000.0  # stake more on top of initial stake, thats more than un-staked tokens
_STAKE5 = 50000.0  # stake more on top of initial stake, that is a valid amount
_UNSTAKE1 = 120000.0  # unstake an amount that would put the staked amount below the minimum staked threshhold
_UNSTAKE2 = 20000.0  # unstake an amount that would put the staked amount above the minimum staked threshhold
_UNSTAKE3 = 100000000.0  # unstake an amount that would put the staked amount to zero

_INIT_BALANCE = 500000.0  # INIT_BALANCE = a valid balance of unstaked tokens master account must initially have that is unstaked 

STAKE1 = '%.4f BOID' % _STAKE1
STAKE2 = '%.4f BOID' % _STAKE2
STAKE3 = '%.4f BOID' % _STAKE3
STAKE4 = '%.4f BOID' % _STAKE4
STAKE5 = '%.4f BOID' % _STAKE5
UNSTAKE1 = '%.4f BOID' % _UNSTAKE1
UNSTAKE2 = '%.4f BOID' % _UNSTAKE2
UNSTAKE3 = '%.4f BOID' % _UNSTAKE3
INIT_BALANCE = '%.4f BOID' % _INIT_BALANCE



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

    parser = argparse.ArgumentParser(description='''
        This is a unit test for the stake and unstake actions of the
        BOID Token smart contract.
    ''')

    parser.add_argument(
        "alias", nargs="?",
        help="Testnet alias")

    parser.add_argument(
        "-t", "--testnet", nargs=4,
        help="<url> <name> <owner key> <active key>")

    parser.add_argument(
        "-b","--build",
        action="store_true",
        help="build new contract ABIs")

    args = parser.parse_args()

    testnet = get_testnet(args.alias, args.testnet)
    testnet.configure()

    ################ test begins here #################

    eosf.create_master_account('master')

    # make build directory if it does not exist
    build_dir = os.path.join(BOID_TOKEN_CONTRACT_PATH, 'build')
    if not os.path.exists(build_dir):
        os.mkdir(build_dir)

    # create reference to the token staking contract
    # build and deploy the contracts on the testnet
    contract = eosf.Contract(master, BOID_TOKEN_CONTRACT_PATH)
    if args.build:
        contract.build()
    contract.deploy()

    # setup contract (1st time)
    try:
        contract.push_action(
            'create',
            {
                'issuer': master,
                'maximum_supply': '10000000000.0000 BOID'
            }, permission=[master]
        )
        contract.push_action(
            'initstats',
            '{}', permission=[master]
        )
        issue(contract, master, INIT_BALANCE, master)
    except:
        pass

    # turn on staking
    stakebreak(contract, "1", master)

    # verify that master's balance is completely unstaked
    staked = acct_has_staked_tokens(contract, master)
    if not staked:
        print('%s\'s balance is completely unstaked\n' % master)
    else:
        print('%s\'s balance is not completely unstaked ... unstaking' % master)
        while staked:
            print(staked)
            unstake(contract, master, UNSTAKE3)
            staked = acct_has_staked_tokens(contract, master)
        print('%s\'s balance is completely unstaked\n' % master)

    transfer(contract, alice, master, '1000.0000 BOID')

    # verify that master's balance = INIT_BALANCE
    balance = getBalance(contract.table("accounts", master))
    while balance != _INIT_BALANCE:
        if balance > _INIT_BALANCE:
            print('%s\'s balance is greater than %s' % (master, INIT_BALANCE))
            print('transfering the difference to %s' % alice)
            transfer(contract, master, alice, '%.4f BOID' % (balance - _INIT_BALANCE))
        elif balance < _INIT_BALANCE:
            # issue tokens to master
            print('%s\'s balance is less than %s' % (master, INIT_BALANCE))
            print('issuing the difference to %s' % master)
            issue(contract, master, '%.4f BOID' % (_INIT_BALANCE - balance), master)
        balance = getBalance(contract.table("accounts", master))
    print('%s\'s balance equals INIT_BALANCE\n' % master)


    ################### staking tests ########################################

    # stake less than minimum stake amount (w/ nothing initially staked)
    try:
        stake(contract, master, STAKE1)
        print('TEST FAILED: accounts are not supposed to be able to')
        print('stake less than minimum stake amount')
        print('(w/ nothing initially staked)\n')
        sys.exit()
    except: # Error as e:  # not sure what type of error
        print('PASSED minimum stake test\n')

    # stake more than account balance amount (w/ nothing initially staked)
    try:
        stake(contract, master, STAKE2)
        print('TEST FAILED: accounts are not supposed to be able to')
        print('stake more than the account\'s balance')
        print('(w/ nothing initially staked)\n')
        sys.exit()
    except: # Error as e:  # not sure what type of error
        print('PASSED maximum stake test 1\n')

    # stake a valid amount (w/ nothing initially staked)
    try:
        stake(contract, master, STAKE3)
        print('PASSED valid stake amount (w/ nothing initially staked) test 1\n')
    except:
        print('TEST FAILED: accounts should be able to stake valid amounts')
        print('(w/ nothing initially staked)\n')
        sys.exit()

    # stake more on top of initial stake, thats more than un-staked tokens
    try:
        stake(contract, master, STAKE4)
        print('TEST FAILED: accounts are not supposed to be able to')
        print('stake more than the account\'s unstaked balance')
        print('(on top of a previous valid stake)\n')
        sys.exit()
    except: # Error as e:  # not sure what type of error
        print('PASSED maximum stake test 2\n')


    # stake more on top of initial stake, that is a valid amount
    try:
        stake(contract, master, STAKE5)
        print('PASSED valid stake amount (on top of a previous valid stake) test 2\n')
    except:
        print('TEST FAILED: accounts should be able to stake valid amounts')
        print('(on top of a previous valid stake)\n')
        sys.exit()


    ################### unstaking tests ######################################

    # unstake an amount that would put the staked amount below the minimum staked threshhold
    unstake(contract, master, UNSTAKE1)

    # unstake an amount that would put the staked amount above the minimum staked threshhold
    unstake(contract, master, UNSTAKE2)

    # unstake an amount that would put the staked amount to zero
    unstake(contract, master, UNSTAKE3)



    # stop the testnet and exit python
    eosf.stop()
    sys.exit()


