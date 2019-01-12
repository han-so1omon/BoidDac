import eosfactory.eosf as eosf
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
MONTH_WAIT   = 1 * 30
QUARTER_WAIT = 1 * 30 * 4
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
        }, permission=[acct]
    )

def claim(acct):
    boidToken_c.push_action(
        'claim',
        {
            '_stake_account': acct
        }, permission = [boid_token] #, forceUnique=1)
    )

def unstake(acct):
    boidToken_c.push_action(
        'unstake',
        {
            '_stake_account': acct,
        }, permission=[boid_token]
    )

def initStaking():
    # initstats - reset/setup configuration of contract
    boidToken_c.push_action(
        'initstats',
        '{}', [boid_token])
    boidToken_c.push_action(  # running - sets payouts to on
        'running',
        {
            'on_switch': '1',
        }, [boid_token])
    stakebreak('1')

def stakebreak(on_switch):
    boidToken_c.push_action(  # stakebreak - activate/deactivate staking for users
        'stakebreak',
        {
            'on_switch': on_switch,
        }, [boid_token])

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
        acct_bp = float(bps[account]) if account in bps.keys() else 0.0
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

def get_stake_roi(dfs):
    for df in dfs:
        stake_revenue = df['unstaked_boid_tokens'] - df['unstaked_boid_tokens'][0]
        df['stake_ROI'] = \
            100 * (stake_revenue / df['staked_boid_tokens'][0])
    return dfs

def get_total_roi(dfs):
    for df in dfs:
        df['total_ROI'] = \
            100 * (df['total_boid_tokens'] / df['total_boid_tokens'][0] - 1.0)
    return dfs

def print_acct_dfs(dfs):

    for i, (df, stake_period) in enumerate(zip(dfs, STAKE_PERIOD_STRINGS)):
        df.index.name = 'week'
        print('------------------------------------ acct%d ---- 1 %s stake -----------------------------------' % ((i + 1), stake_period))
        print(df)
        print('---------------------------------------------------------------------------------------------------')



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

    # data frames to hold account state
    acct_df_columns = [
        'boid_power',
        'staked_boid_tokens',
        'unstaked_boid_tokens',
        'total_boid_tokens']
    dfs = [
        pd.DataFrame(columns=acct_df_columns),
        pd.DataFrame(columns=acct_df_columns)]

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


    # Set up boid_token account as issuer of BOID
    boidToken_c.push_action(
        'create',
        {
            'issuer': boid_token,
            'maximum_supply': '1000000000.0000 BOID'
        }, [boid_token])

    # issue initial quantities of BOID
    for acct in accts:
        boidToken_c.push_action(
            'issue',
            {
                'to': acct,
                'quantity': '%.4f BOID' % INIT_BOIDTOKENS,
                'memo': 'memo'
            }, [boid_token])

    boidToken_c.push_action(
            'transfer',
            {
                'from': acct1,
                'to': acct2,
                'quantity': '1000.0000 BOID',
                'memo': 'memo',
                'a': '0'  # 0: from->payer
            }, [acct1])
    boidToken_c.push_action(
            'transfer',
            {
                'from': acct2,
                'to': acct1,
                'quantity': '500.0000 BOID',
                'memo': 'memo',
                'a': '1'  # 1: to->payer
            }, [acct2])
    eosf.stop()
    sys.exit()












    for acct in accts:  # set bp for accounts
        setBoidpower(acct, INIT_BOIDPOWER)
    initStaking()  # setup
#    # test setters
#    boidToken_c.push_action('setmonth', {'month_stake_roi':'1.2'}, [boid_token])
#    boidToken_c.push_action('setquarter', {'quarter_stake_roi':'1.5'}, [boid_token])
#    boidToken_c.push_action('setbpratio', {'bp_bonus_ratio':'0.0002'}, [boid_token])
#    boidToken_c.push_action('setbpmult', {'bp_bonus_multiplier':'0.000002'}, [boid_token])
#    boidToken_c.push_action('setbpmax', {'bp_bonus_max':'55000.0'}, [boid_token])
#    boidToken_c.push_action('setminstake', {'min_stake':'5000.0'}, [boid_token])
    for stake_period, acct in zip(STAKE_PERIODS, accts):  # stake boid tokens
        stake(acct, '%.4f BOID' % INIT_BOIDSTAKE, str(stake_period))
    stakebreak('0')  # disable staking, stakebreak is over

    # run test over time
    dfs = get_state(boidToken_c, boid_token, accts, dfs)
    for t in range(TEST_DURATION):
#        if t+1 > 1:  # testing exit
#            eosf.stop()
#            sys.exit()
        time.sleep(WEEK_WAIT)
        print('\n/-------------------- week %d --------------------------------\\' % (t+1))
        for i, acct in enumerate(accts):
            print('acct %d' % (i+1))
            claim(acct)
        dfs = get_state(boidToken_c, boid_token, accts, dfs)
        print('\\--------------------- week %d ---------------------------------/' % (t+1))
    dfs = get_stake_roi(dfs)
    dfs = get_total_roi(dfs)

    # unstake the staked tokens of each account
    for acct in accts:
        unstake(acct)

    # output test results, and save them to csv files if prompted
    print_acct_dfs(dfs)
    if args.save:
        save_loc = os.path.join(
            BOID_TOKEN_CONTRACT_PATH,
            'tests',
            'results')
        for acct, df in zip(accts, dfs):
            file_loc = os.path.join(save_loc, acct.name + '_df')
            df.to_csv(file_loc)

    # stop the testnet and exit python
    eosf.stop()
    sys.exit()
