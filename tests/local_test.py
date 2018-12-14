import eosfactory.eosf as eosf
import sys
import os
import json
import argparse
import subprocess
import time
import numpy as np
import pandas as pd

BOID_TOKEN_CONTRACT_PATH = \
    os.path.abspath(
        os.path.join(
            os.path.dirname(os.path.abspath(__file__)),
            '..'))

STAKE_MONTHLY = '1'
STAKE_QUARTERLY = '2'

WEEK_WAIT = 1 * 7
MONTH_WAIT = 1 * 7 * 30
QUARTER_WAIT = 1 * 7 * 30 * 4

# @param account  The account to set/delete a permission authority for
# @param permission  The permission name to set/delete an authority for
# @param authority  NULL, public key, JSON string, or filename defining the authority
# @param parent  The permission name of this parents permission (Defaults to "active")
def setAccountPermission(account, permission, authority, parent,
        json=False, code=False):
    if json: json = '--json'
    else: json = ''
    if code: code = '--add-code'
    else: code = ''
    permissionCmd =\
        'cleos set account permission {0} {1} {2} {3} -p {0}@active {4}'.format(
                        account, permission, authority, parent, json)
    subprocess.call(permissionCmd, shell=True)

# @param account  The account to set/delete a permission authority for
# @param contract  The account that owns the code for the action
# @param actionName  The type of the action
# @param permissionName  The permission name required for executing the given action 
def setActionPermission(
        account, contract, actionName, permissionName):
    permissionCmd = \
            'cleos set action permission {0} {1} {2} {3} -p {0}@active'.format(
                        account, contract, actionName, permissionName)
    subprocess.call(permissionCmd, shell=True)

transferPermission = lambda x,y:\
   '\'{{\
        "threshold": 1,\
        "keys": [\
            {{\
                "key" : "{0}",\
                "weight" : 1\
            }}\
        ],\
        "accounts": [\
            {{\
                "permission": {{"actor": "{1}", "permission": "eosio.code"}},\
                "weight" : 1\
            }}\
        ]\
    }}\''.format(x,y)

def stake(acct, amount, stake_period):
    boidToken_c.push_action(
        'stake',
        {
            '_stake_account': acct,
            '_stake_period': stake_period,
            '_staked': amount
        }, permission=[acct, boid_token]
    )

def claim(acct):
    boidToken_c.push_action(
        'claim',
        {
            '_stake_account': acct
        }, permission = [acct]
    )

def unstake(acct):
    boidToken_c.push_action(
        'unstake',
        {
            '_stake_account': acct,
        }, permission=[acct, boid_token]
    )

def initStaking():
    boidToken_c.push_action(  # running - sets payouts to on
        'running',
        {
            'on_switch': '1',
        }, [boid_token])
    # initstats - reset/setup configuration of contract
    boidToken_c.push_action(
        'initstats',   
        {}, [boid_token])

def setBoidpower(acct, bp):
    boidToken_c.push_action(
        'setnewbp',
        {
            'acct': acct,
            'boidpower': bp
        }, [boid_token, acct])


def getBalance(x):
    if len(x.json['rows']) > 0:
        return float(x.json['rows'][0]['balance'].split()[0])
    else:
        return 0

def getStakeParams(x):
    ret = {}
    for i in range(len(x.json['rows'])):
        ret[x.json['rows'][i]['stake_account']] = \
            {
             'stake_period': x.json['rows'][i]['stake_period'],
             'staked': x.json['rows'][i]['staked']
            }
    return ret

def getBoidpowers(x):
    ret = {}
    for i in range(len(x.json['rows'])):
        ret[x.json['rows'][i]['acct']] = x.json['rows'][i]['quantity']
    return ret

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument("-b","--build", action="store_true",
                        help="build new contract abis")
    args = parser.parse_args()

    # start single-node local testnet
    eosf.reset()

    # create master account from which
    # other account can be created
    # accessed via global variable: master
    w = eosf.wallet.Wallet()
    eosf.create_master_account('master')

    # Create 7 accounts: eosio_token, eos, boid, boid_stake, boid_power, acct1, acct2
    eosf.create_account(
        'boid_token', master, account_name='boid.token')
    eosf.create_account(
        'boid', master, account_name='boid')
    eosf.create_account(
        'boid_power', master, account_name='boid.power')
    eosf.create_account(
        'acct1', master, account_name='account1')
    eosf.create_account(
        'acct2', master, account_name='account2')

    # make build directory if it does not exist
    build_dir = os.path.join(BOID_TOKEN_CONTRACT_PATH, 'build')
    if not os.path.exists(build_dir):
        os.mkdir(build_dir)

    # create reference to the token staking contract
    boidToken_c = eosf.Contract(
        boid_token, BOID_TOKEN_CONTRACT_PATH)

    # build the token staking contract
    if args.build:
        boidToken_c.build()

    # deploy the token staking contract on the testnet
    boidToken_c.deploy()

    # Allow boid.stake to transfer BOiD tokens from eosio.token contract
    '''
    xferKey = eosf.wallet.cleos.CreateKey('xfer')
    w.import_key(xferKey.key_private)
    setAccountPermission(
            boid_stake.name, 'xfer',
            #xferKey.key_public, 'active')
            #transferPermission(xferKey.key_public,eosio_token.name),
            transferPermission(xferKey.key_public,boid_stake.name),
            'active')
    setActionPermission(boid_stake.name, eosio_token.name, 'transfer', 'xfer')


    setAccountPermission(
            acct1.name, 'active',
            transferPermission(acct1.active_key.key_public, boid_stake.name),
            'owner')
    '''

    ############# now we can call functions ##############
    ########## (aka actions) from the contract! ##########

    # Set up master as issuer of EOS and boid as issuer of BOID
    # account.push_action(
    #		action_name,
    #		action_arguments_in_json,
    #		account_whose_permission_is_needed)
    boidToken_c.push_action(
        'create',
        {
            'issuer': boid,
            'maximum_supply': '1000000000.0000 BOID'
        }, [boid_token])

    # Distribute initial quantities of EOS & BOID
    boidToken_c.push_action(
        'issue',
        {
            'to': acct1,
            'quantity': '1000000.0000 BOID',
            'memo': 'memo'
        }, [boid])
    boidToken_c.push_action(
        'issue',
        {
            'to': acct2,
            'quantity': '2000000.0000 BOID',
            'memo': 'memo'
        }, [boid])

    initStaking()

    stakeColumns = ['Week', 'Balance', 'Staked', 'StakePeriod', 'Boidpower']
    stakeDf = pd.DataFrame(columns = stakeColumns)

    setBoidpower(acct1, 10)
    setBoidpower(acct2, 10000)

    # Run staking tests with acct1 and acct2
    stake(acct1, '100000.0000 BOID', STAKE_MONTHLY)
    stake(acct2, '100000.0000 BOID', STAKE_QUARTERLY)
    print(getBalance(boidToken_c.table("accounts", acct1)))
    print(getBoidpowers(boidToken_c.table('boidpowers', boid_token)))

    numWeeks = 1
    for i in range(numWeeks):
        time.sleep(WEEK_WAIT)

        claim(acct1)
        claim(acct2)
        print(getBalance(boidToken_c.table("accounts", acct1)))
        print(getStakeParams(boidToken_c.table('stakes',boid_token)))

    unstake(acct1)
    unstake(acct2)
    print(getBalance(boidToken_c.table("accounts", acct1)))


    # stop the testnet and exit python
    eosf.stop()
    sys.exit()
