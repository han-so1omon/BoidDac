import eosfactory.eosf as eosf
import sys
import os
import json
import argparse
import subprocess
import time
import numpy as np
import pandas as pd
pd.set_option('display.width', 500)


################################# Test variables #########################################

TEST_DURATION   = 52  # measured in weeks
INIT_BOIDTOKENS = 10  # initial boid tokens given to each account (must be less than 1/4th of max supply (1000000000 BOID))
INIT_BOIDPOWER  = 30  # initial boid power given to each account
INIT_BOIDSTAKE  = 2.0230  # initial boid tokens staked by each account (must be <= INIT_BOIDTOKENS)

############# Must also modify in boidtoken.hpp ##############
# TESTING Speeds Only
WEEK_WAIT    = 1
MONTH_WAIT   = 1 * 30
QUARTER_WAIT = 1 * 30 * 4
MONTH_MULTIPLIERX100   = 150  # Reward for staking monthly
QUARTER_MULTIPLIERX100 = 200  # Reward for staking quarterly
MONTHLY   = 1
QUARTERLY = 2
##############################################################
STAKE_PERIODS = [MONTHLY, QUARTERLY]
STAKE_PERIOD_STRINGS = ['Month', 'Quarter']

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
        '{}', [boid_token])

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

def get_state(contract, contract_owner, accts, dfs, p=False):

    for account_num, acct in enumerate(accts):
        account = 'account%d' % (account_num + 1)
        acct_balance = getBalance(contract.table("accounts", acct))
        stake_params = getStakeParams(contract.table('stakes',contract_owner))
        staked_tokens = float(stake_params[account]['staked'].split()[0]) \
            if account in stake_params.keys() else 0.0
        bps = getBoidpowers(contract.table('boidpowers', contract_owner))
        acct_bp = bps[account] if account in bps.keys() else 0.0
        dfs[account_num] = dfs[account_num].append({
            'boid_power': acct_bp,
            'staked_boid_tokens': staked_tokens,
            'unstaked_boid_tokens': acct_balance,
            'total_boid_tokens': acct_balance + staked_tokens
        }, ignore_index=True)

        if p: print('%s_balance = %f' % (acct, acct_balance))
        if p: print('stake params %s' % stake_params)
        if p: print('%s_bp = %f' % (acct, acct_bp))

    return dfs

def get_percentage_change(dfs):
    for df in dfs:
        df['stake_ROI'] = \
            100 * (df['total_boid_tokens'] / df['total_boid_tokens'][0] - 1.0)
    return dfs

def print_acct_dfs(dfs):

    for i, (df, stake_period) in enumerate(zip(dfs, STAKE_PERIOD_STRINGS)):
        print('------------------------------ acct%d --- 1 %s stake -----------------------------' % ((i + 1), stake_period))
        print(df)
        print('-------------------------------------------------------------------------------------')



if __name__ == '__main__':

    # determine if we want to
    # save the test data to a csv
    # build the contracts
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-s","--save",
        dest="save",
        help="save test data to csv file location SAVE (default SAVE = results)")
    parser.add_argument(
        "-b","--build",
        action="store_true",
        help="build new contract ABIs")
    args = parser.parse_args()

    # verify valid save location
    if args.save:
        save_loc = os.path.join(
            BOID_TOKEN_CONTRACT_PATH,
            'tests',
            args.save)
        if not os.path.exists(save_loc):
            print('invalid save location:')
            print(save_loc)
            print('location does not exist')
            sys.exit()

    # start single-node local testnet
    eosf.reset()

    # create master account from which
    # other account can be created
    # accessed via global variable: master
    w = eosf.wallet.Wallet()
    eosf.create_master_account('master')

    # Create accounts: boid_token, boid
    eosf.create_account('boid_token', master, account_name='boid.token')
    eosf.create_account('boid',       master, account_name='boid')

    # acct1 does monthly stakes and acct2 does quarterly stakes
    eosf.create_account('acct1',      master, account_name='account1')
    eosf.create_account('acct2',      master, account_name='account2')
    accts = [acct1, acct2]

    # data frames to hold account state
    acct_df_columns = [
        'boid_power',
        'staked_boid_tokens',
        'unstaked_boid_tokens',
        'total_boid_tokens']
    dfs = [
        pd.DataFrame(columns=acct_df_columns),
        pd.DataFrame(columns=acct_df_columns)]

    stakeColumns = ['Week', 'Balance', 'Staked', 'StakePeriod', 'Boidpower']
    stakeDf = pd.DataFrame(columns = stakeColumns)

    # make build directory if it does not exist
    build_dir = os.path.join(BOID_TOKEN_CONTRACT_PATH, 'build')
    if not os.path.exists(build_dir):
        os.mkdir(build_dir)

    # create reference to the token staking contract
    # build and deploy the contracts on the testnet
    boidToken_c = eosf.Contract(boid_token, BOID_TOKEN_CONTRACT_PATH)
    if args.build:
        boidToken_c.build()
    boidToken_c.deploy()


    ############# now we can call functions ##############
    ########## (aka actions) from the contract! ##########

    # Set up boid account as issuer of BOID
    boidToken_c.push_action(
        'create',
        {
            'issuer': boid,
            'maximum_supply': '1000000000.0000 BOID'
        }, [boid_token])

    # Distribute initial quantities of BOID
    for acct in accts:
        boidToken_c.push_action(
            'issue',
            {
                'to': acct,
                'quantity': '%.4f BOID' % INIT_BOIDTOKENS,
                'memo': 'memo'
            }, [boid])

    # stake BOID tokens of accts
    initStaking()  # setup
    for acct in accts:  # set bp for accounts
        setBoidpower(acct, INIT_BOIDPOWER)
    for stake_period, acct in zip(STAKE_PERIODS, accts):  # stake
        stake(acct, '%.4f BOID' % INIT_BOIDSTAKE, str(stake_period))

    # run test over time
    dfs = get_state(boidToken_c, boid_token, accts, dfs)
    for t in range(TEST_DURATION):
        time.sleep(WEEK_WAIT)
        print('week %d' % (t+1))
        for i, acct in enumerate(accts):
            print('acct %d' % (i+1))
            claim(acct)
        dfs = get_state(boidToken_c, boid_token, accts, dfs)
    dfs = get_percentage_change(dfs)

    # unstake the staked tokens of each account
    for acct in accts:
        unstake(acct)

    # output test results, and save them to csv files if prompted
    print_acct_dfs(dfs)
    if args.save:
        save_loc = os.path.join(
            BOID_TOKEN_CONTRACT_PATH,
            'tests',
             args.save)
        for acct, df in zip(accts, dfs):
            file_loc = os.path.join(save_loc, acct.name + '_df')
            df.to_csv(file_loc)

    # stop the testnet and exit python
    eosf.stop()
    sys.exit()
