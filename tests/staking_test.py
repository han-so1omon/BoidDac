import eosfactory.eosf as eosf
import sys
import os
import time
import json
import argparse
import subprocess
import pandas as pd
pd.set_option('precision',9)
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import rcParams
rcParams.update({'figure.autolayout': True})


# TODO: find a better way to reference the eos/contracts/eosio.token
eosBuild = os.getenv('EOS_SRC')
if eosBuild == '' or eosBuild == None:
    raise ValueError(
            'EOS_BUILD environment variable must be set')
EOS_TOKEN_CONTRACT_PATH = os.path.join(eosBuild,'contracts','eosio.token')

BOID_STAKE_CONTRACT_PATH = \
    os.path.abspath(
        os.path.join(
            os.path.dirname(os.path.abspath(__file__)),
            '..'))

assetDict = {'BOID': 0, 'EOS': 1}
getAssetQuantity = lambda x: float(x.split()[0])
getAssetType = lambda x: assetDict(x.split()[1])

################################# Test variables #########################################

TEST_DURATION = 4  # measured in weeks
INIT_BOIDTOKENS = 1000000  # initial boid tokens given to each account (must be less than 1/4th of max supply (1000000000 BOID))
INIT_BOIDPOWER = 1000  # initial boid power given to each account
INIT_BOIDSTAKED = 1000000  # initial boid tokens staked by each account (must be <= INIT_BOIDTOKENS)

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

##########################################################################################



# @param account  The account to set/delete a permission authority for
# @param permission  The permission name to set/delete an authority for
# @param authority  NULL, public key, JSON string, or filename defining the authority
# @param parent  The permission name of this parents permission (Defaults to "active")
def setAccountPermission(account, permission, authority, parent, json=False):
    if json: json = '--json'
    else: json = ''
    permissionCmd = \
            'cleos set account permission {0} {1} {2} {3} -p {0}@owner {4}'.format(
                        account, permission, authority, parent, json)
    print('\naccount permissionCmd')
    print(permissionCmd)
    print()
    subprocess.call(permissionCmd, shell=True)

# @param account  The account to set/delete a permission authority for
# @param contract  The account that owns the code for the action
# @param actionName  The type of the action
# @param permissionName  The permission name required for executing the given action 
def setActionPermission(account, contract, actionName, permissionName):
    permissionCmd = \
           'cleos set action permission {0} {1} {2} {3} -p {0}@owner'.format(
                        account, contract, actionName, permissionName)
    print('\naction permissionCmd')
    print(permissionCmd)
    print()
    subprocess.call(permissionCmd, shell=True)

transferPermission = lambda key, actor: \
    '\'{' + \
        '"threshold": 1, ' + \
        '"keys": ' + ('[{"key":"%s", "weight":1}], '%key if key != '' else '[], ') + \
        '"accounts": [' + \
            '{' + \
                '"permission": {"actor": "%s", "permission": "eosio.code"}, ' % actor + \
                '"weight": 1' + \
            '}' + \
        ']' + \
    '}\''


def getBalance(x):
    if len(x.json['rows']) > 0:
        return float(x.json['rows'][0]['balance'].split()[0])
    else:
        return 0

def getStakeParams(x):
    ret = {}
    for i in range(len(x.json['rows'])):
        ret[x.json['rows'][i]['stake_account']] = \
            {'stake_period': x.json['rows'][i]['stake_period'],
             'staked': x.json['rows'][i]['staked'],
             'escrow': x.json['rows'][i]['escrow']}
    return ret

def getBoidpowers(x):
    ret = {}
    for i in range(len(x.json['rows'])):
        ret[x.json['rows'][i]['acct']] = x.json['rows'][i]['quantity']
    return ret

#TODO
def getStakeType(acct):
    '''
  acct1_type="$(cleos_local_test push action boid.stake printstake \
    '[ "acct1" ]' -p boid.stake | sed -n 2p | sed 's/[^0-9]*//g')"
    '''
    pass

# def setAccountPermission(account, permission, authority, parent, json=False):
#         if json: json = '--json'
#         else: json = ''
#         permissionCmd = \
#                 'cleos set account permission {0} {1} {2} {3} -p {0}@owner {4}'.format(
#                             account, permission, authority, parent, json)
#         print('account permission')
#         print(permissionCmd)
#         subprocess.call(permissionCmd, shell=True)



def get_percentage_change(dfs):
    for df in dfs:
        df['stake_ROI'] = \
            100 * (df['staked_boid_tokens'] / df['staked_boid_tokens'][0] - 1.0)
    return dfs

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

def print_acct_dfs(dfs):

    stake_periods = ['Month', 'Quarter']
    for account_num, df in enumerate(dfs):
        print('------------------------ acct%d --- 1 %s stake -----------------------' % ((account_num + 1), stake_periods[account_num]))
        print(df)
        print('---------------------------------------------------------------------------')

def plot_stake_reward(dfs, save):

    colors = ['red', 'green', 'blue', 'yellow']
    dot_size = 1.625
    plots = []
    df_column_to_plot = 'stake_ROI'

    for i, df in enumerate(dfs):
        y = df[df_column_to_plot].tolist()
        x = range(len(y))
        plots.append(
            plt.scatter(
                x, y,
                c=colors[i],
                s=dot_size,
                label=STAKE_PERIOD_STRINGS[i]))

    all_pts = []
    for df in dfs:
        all_pts += df[df_column_to_plot].tolist()
    _min , _max = min(all_pts), max(all_pts)
    diff = _max - _min
    margin = 0.75
    plt.ylim(_min - margin*diff, _max + margin*diff)
    plt.title('Stake Reward')
    plt.xlabel('Days')
    plt.ylabel('ROI % of Staked BOID Tokens')
    plt.ticklabel_format(useOffset=False)
    plt.legend(tuple(plots),
        (
            '1 Month Stake: MONTH_MULTIPLIERX100 = %s' % MONTH_MULTIPLIERX100,
            '1 Quarter Stake: QUARTER_MULTIPLIERX100 = %s' % QUARTER_MULTIPLIERX100
        ), loc='upper left')
    if save:
        plot_file = os.path.join(os.getcwd(), save, 'staking_reward_plot.png')
        plt.savefig(plot_file)
    plt.show()



if __name__ == '__main__':

    # determine if we want to build the contracts
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-s","--save",
        dest="save",
        help="save test data to csv file location SAVE (default SAVE = results)")
    parser.add_argument(
        "-u","--use_csv",
        action="store_true",
        help="use data from csv file")
    parser.add_argument(
        "-b","--build",
        action="store_true",
        help="build new contract ABIs (-u must not be flagged)")
    args = parser.parse_args()

    # verify valid save location
    if args.save:
        save_loc = os.path.join(
            BOID_STAKE_CONTRACT_PATH,
            'tests',
            args.save)
        if not os.path.exists(save_loc):
            print('invalid save location: %s' % save_loc)
            sys.exit()

    # get csv data if prompted
    if args.use_csv:
        csv_loc = os.path.join(BOID_STAKE_CONTRACT_PATH, 'tests', 'results')
        dfs = []
        for i in [1,2,3,4]:
            csv_file = os.path.join(csv_loc, 'df%d.csv'%i)
            # check if csv exists
            if os.path.exists(csv_file):
                try:
                    dfs.append(pd.read_csv(csv_file))
                except:
                    print('failed to use all csv files')
                    sys.exit()
            else:
                print('csv files do not exist')
                sys.exit()
    else:

        # start single-node local testnet
        eosf.reset()

        eosf.create_wallet()
        eosf.get_wallet().open_unlock()
        eosf.get_wallet().keys()

        # create master account from which
        # other account can be created
        # accessed via global variable: master
        eosf.create_master_account('master')

        # Create accounts: eosio_token, eos, boid, boid_stake, boid_power, acct1, acct2
        eosf.create_account('eosio_token', master, account_name='eosio.token')
        eosf.create_account('eos',         master, account_name='eos')
        eosf.create_account('boid',        master, account_name='boid')
        eosf.create_account('boid_power',  master, account_name='boid.power')
        eosf.create_account('boid_stake',  master, account_name='boid.stake')
        # setAccountPermission(
        #     boid_stake.name, 'xfer',
        #     transferPermission(
        #         '', #boid_stake.active_key.key_public,
        #         boid_stake.name), 'active')

        # print(222222222)
        # print(boid_stake.name)
        # print(333333333)
        # EOS7nLRAheR7tYMiTiCA2ry2vW2Yh4LKoMUsxiE6YcxSCh2nS8tq4
        # key = boid_stake.active_key.key_public
        # aaa = 'cleos set account permission %s active \'{"threshold":1,"keys":[{"key":"%s","weight":1}],"accounts":[{"permission":{"actor":"%s","permission":"eosio.code"},"weight":1}]}\' owner -p %s@owner' % (boid_stake.name, key, boid_stake.name, boid_stake.name)
        # aaa = 'cleos set account permission %s active \'{"threshold":1,"keys":[{"key":"%s","weight":1}],"accounts":[{"permission":{"actor":"eosio","permission":"eosio.code"},"weight":1},{"permission":{"actor":"%s","permission":"eosio.code"},"weight":1}]}\' owner -p %s@owner' % (boid_stake.name, key, boid_stake.name, boid_stake.name)


        # print('aaa')
        # print(aaa)
        # subprocess.call(aaa, shell=True)

        # setActionPermission(boid_stake.name, eosio_token.name, 'transfer', 'xfer')

        # resign = '{"threshold" : 1, "keys" : [], "accounts": [{"permission":{"actor":"boid.token", "permission":"active"}, "weight":1}]}'#, "waits": []}'
        # setAccountPermission(boid_stake.name, 'active', resign, 'owner')
        # setAccountPermission(boid_stake.name, 'owner',  resign, '\'\'')

        # acct1 does monthly stakes and acct2 does quarterly stakes
        eosf.create_account('acct1',       master, account_name='account1')
        eosf.create_account('acct2',       master, account_name='account2')
        accts = [acct1, acct2]

        eosf.get_wallet().keys()

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
        # (so eosfactory sets up the eos contract proberly)
        build_dir = os.path.join(BOID_STAKE_CONTRACT_PATH, 'build')
        if not os.path.exists(build_dir):
            os.mkdir(build_dir)

        # create reference to the token staking contract
        # build and deploy the contracts on the testnet
        eosioToken_c = eosf.Contract(eosio_token, EOS_TOKEN_CONTRACT_PATH)
        boidStake_c  = eosf.Contract(boid_stake, BOID_STAKE_CONTRACT_PATH)
        if args.build:
            eosioToken_c.build()
            boidStake_c.build()
        eosioToken_c.deploy()
        boidStake_c.deploy()


        ############# now we can call functions ##############
        ########## (aka actions) from the contract! ##########


        # Set up eos account as issuer of EOS
        # and boid account as issuer of BOID
        eosioToken_c.push_action(
            'create',  # action name
            {  # action_arguments_in_json
                'issuer': eos,
                'maximum_supply': '1000000000.0000 EOS'
            }, [eosio_token])  # list_of_accounts_whose_permission_is_needed)
        eosioToken_c.push_action(
            'create',
            {
                'issuer': boid,
                'maximum_supply': '1000000000.0000 BOID'
            }, [eosio_token])
        boidStake_c.push_action(
            'create',
            {
                'issuer': boid,
                'maximum_supply': '1000000000.0000 BOID'
            }, [boid_stake])
        boidStake_c.push_action(
            'running',  # running sets payouts to on
            {
                'on_switch': '1',
            }, [boid_stake])
        boidStake_c.push_action(
            'initstats',  # initstats resets/sets up configuration of contract
            {}, [boid_stake])


        print('aaaaaaaaaaaaaaaaaaa')
        subprocess.call('cleos get scope eosio.token -t accounts', shell=True)
        print('bbbbbbbbbbbbbbbbbbb')
        # Distribute initial quantities of EOS & BOID
        for acct in accts:
            eosioToken_c.push_action(
                'issue',
                {
                    'to': acct,
                    'quantity': '1000000.0000 EOS',
                    'memo': 'memo'
                }, [eos])
            eosioToken_c.push_action(
                'issue',
                {
                    'to': acct,
                    'quantity': '%.4f BOID' % INIT_BOIDTOKENS,
                    'memo': 'memo'
                }, [boid])
            boidStake_c.push_action(
                'issue',
                {
                    'to': acct,
                    'quantity': '%.4f BOID' % INIT_BOIDTOKENS,
                    'memo': 'memo'
                }, [boid])
        # print(boidStake_c.table("accounts", acct1))
        # print(boidStake_c.table("accounts", acct2))

        print('cccccccccccccccccc')
        subprocess.call('cleos get scope eosio.token -t accounts', shell=True)
        print('dddddddddddddddddd')

        for acct in accts:
            # cmd = 'cleos get table %s %s accounts' % (eosio_token.name, acct.name)
            cmd = 'cleos get currency balance %s %s' % (eosio_token.name, acct.name)
            print('hii' + cmd)
            subprocess.call(cmd, shell=True)

        # give accounts boidpower
        for acct in accts:
            boidStake_c.push_action(
                'setnewbp',
                {
                    'acct': acct,
                    'boidpower': str(INIT_BOIDPOWER)
                }, [boid_stake, acct])
        # print(getBoidpowers(boidStake_c.table('boidpowers', boid_stake)))
        
        boid_stake.info()    
        # each account stakes their BOID tokens for
        # a day, week, month, quarter, respectively
        for stake_period, acct in zip(STAKE_PERIODS, accts):
            boidStake_c.push_action(
                'stake',
                {
                    '_stake_account': acct,
                    '_stake_period': str(stake_period),
                    '_staked': '%.4f BOID' % INIT_BOIDSTAKED
                }, [acct])
                # permission=[
                #     (acct),
                #     (boid_stake, '@active'),
                #     (boid_stake, '@xfer')
                # ])

        # each account claims its stake reward each day
        print('week 0')
        dfs = get_state(boidStake_c, boid_stake, accts, dfs)
        for t in range(TEST_DURATION):
            time.sleep(WEEK_WAIT)
            print('week %d' % (t+1))
            for i, acct in enumerate(accts):
                print('acct %d' % (i+1))
                boidStake_c.push_action(
                    'claim',
                    {'_stake_account': acct},
                    [acct])
            dfs = get_state(boidStake_c, boid_stake, accts, dfs)
        dfs = get_percentage_change(dfs)

        # save the data
        if args.save:
            csv_loc = os.path.join(os.getcwd(), args.save)
            for i, df in enumerate(dfs):
                df.to_csv(os.path.join(csv_loc, 'df%d.csv'%(i+1)))

    print_acct_dfs(dfs)
    plot_stake_reward(dfs, args.save)


    # stop the testnet and exit python
    eosf.stop()
    sys.exit()

