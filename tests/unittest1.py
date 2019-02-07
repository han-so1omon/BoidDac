import unittest, argparse, sys, time
import eosfactory.eosf as eosf
import eosfactory
import sys, os
import json
import argparse
import subprocess
import string
import time
import numpy as np
import pandas as pd
import random
pd.set_option('display.width', 500)
pd.set_option('display.max_rows',10)
pd.set_option('display.float_format', '{:.3f}'.format)
#pd.options.display.float_format = '${:,.2f}'.format

eosf.verbosity([eosf.Verbosity.INFO,
                eosf.Verbosity.OUT,
                eosf.Verbosity.TRACE])

CONTRACT_WORKSPACE = sys.path[0] + "/../"

INITIAL_RAM_KBYTES = 100
INITIAL_STAKE_NET = 3
INITIAL_STAKE_CPU = 3

############# Must also modify in boidtoken.hpp ##############
# TESTING Speeds Only
WEEK_WAIT    = 7 # seconds

################################# Test variables #########################################

#NUM_ACCOUNTS        = 100
NUM_ACCOUNTS        = 3
TEST_DURATION       = [1,8,10,20]*WEEK_WAIT
MAX_BOID_SUPPLY     = 1e12
INIT_BOIDTOKENS     = np.linspace(1,10e9,NUM_ACCOUNTS)
INIT_BOIDPOWER      = np.linspace(0,10e3,NUM_ACCOUNTS)
INIT_BOIDSTAKE      = INIT_BOIDTOKENS/10

##########################################################################################
digs = string.digits + string.ascii_letters
def eos_name_digits(x):
    if x < 0:
        sign = -1
    elif x == 0:
        return digs[0 + 1]
    else:
        sign = 1

    x *= sign
    digits = []

    while x:
        digits.append(digs[int(x % 5) + 1])
        x = int(x / 5)

    if sign < 0:
        digits.append('-')

    digits.reverse()

    return ''.join(digits)

eosAlphabet=['a','b','c','d','e','f','g','h','i','j','k',
             'l','m','n','o','p','q','r','s','t','u','v',
             'w','x','y','z','1','2','3','4','5']
eosAccountNameDict = {}
def eosNameGenerator(length=12):
    if length > 12:
        raise ValueError('length must be between 1 and 12')
    name = ''
    nameFound = False
    while not nameFound:
        for i in range(length):
            name += eosAlphabet[random.randint(0,len(eosAlphabet)-1)]
        if name not in eosAccountNameDict:
            nameFound = True
            eosAccountNameDict[name] = True
        else: name = ''
    return name

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

# @param account  The account to set/delete a permission authority for
# @param contract  The account that owns the code for the action
# @param actionName  The type of the action
# @param permissionName  The permission name required for executing the given action 
def setActionPermission(account, contract, actionName, permissionName):
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

def issue(contract_acct, issuer_acct, to_acct, amount, memo):
    contract_acct.push_action(
        'issue',
        {
            'to': to_acct,
            'quantity': '%.4f BOID' % amount,
            'memo': memo 
        }, [issuer_acct])

def stake(contract_acct, acct, amount, memo):
    contract_acct.push_action(
        'stake',
        {
            '_stake_account': acct,
            '_staked': amount,
            'memo': memo
        }, permission=[acct]
    )

def claim(contract_acct, token_acct, acct):
    contract_acct.push_action(
        'claim',
        {
            '_stake_account': acct
        }, permission = [token_acct] #, forceUnique=1)
    )

def unstake(contract_acct, acct, acct_permission, memo):
    contract_acct.push_action(
        'unstake',
        {
            '_stake_account': acct,
            'memo': memo
        }, permission=[acct_permission]
    )

def initStaking(contract_acct, token_acct):
    # initstats - reset/setup configuration of contract
    contract_acct.push_action(
        'initstats',
        '{}', [token_acct])
    stakebreak(contract_acct, token_acct, '1')

def stakebreak(contract_acct, token_acct, on_switch):
    contract_acct.push_action(  # stakebreak - activate/deactivate staking for users
        'stakebreak',
        {
            'on_switch': on_switch
        }, [token_acct])

def setBoidpower(contract_acct, token_acct, acct, bp):
    contract_acct.push_action(
        'setnewbp',
        {
            'acct': acct,
            'boidpower': bp
        }, [token_acct, acct])

def setAutostake(contract_acct, acct, on_switch):
    contract_acct.push_action(
        'setautostake',
        {
            '_stake_account': acct,
            'on_switch': on_switch 
        }, [acct])

def transfer(contract_acct, from_acct, to_acct, amount, memo):
    contract_acct.push_action(
        'transfer',
        {
            'from': from_acct,
            'to': to_acct,
            'quantity': amount,
            'memo': memo
        }, permission=[from_acct]
    )

def setTokenAmount(contract_acct, set_acct, store_acct,
        req_balance, issuer=False):
    if req_balance < 0:
        raise ValueError('invalid requested balance: {} < 0'.format(
            req_balance))
    currBalance = getBalance(contract_acct.table("accounts",set_acct))
    if currBalance is None or currBalance == 0:
        if not issuer:
            raise Exception(
                'must create account {} before token transfer'.format(
                    set_acct.name))
    xfer_amount = req_balance - currBalance
    if xfer_amount == 0:
        return
    elif xfer_amount < 0:
        xfer_amount *= -1
        from_acct = set_acct
        to_acct = store_acct
    else:
        from_acct = store_acct
        to_acct = set_acct

    if not issuer or from_acct.name == set_acct.name:
        if getBalance(
                contract_acct.table("accounts",from_acct)) < xfer_amount:
            raise ValueError(
                'not enough tokens in supplier account {}'.format(
                    from_acct.name))
        transfer(contract_acct, from_acct, to_acct, xfer_amount, 'memo')
    elif issuer and from_acct.name == store_acct.name:
        issue(contract_acct,store_acct,to_acct,xfer_amount,'memo')

def getBalance(x):
    ret = 0
    if len(x.json['rows']) > 0:
        try:
            ret = float(x.json['rows'][0]['balance'].split()[0])
        except AttributeError as e:
            pass
    return ret

def getStakeParams(x):
    ret = {}
    for i in range(len(x.json['rows'])):
        ret[x.json['rows'][i]['stake_account']] = \
            {
             'auto_stake': x.json['rows'][i]['auto_stake'],
             'staked': x.json['rows'][i]['staked']
            }
    return ret

def getBoidpowers(x):
    ret = {}
    for i in range(len(x.json['rows'])):
        ret[x.json['rows'][i]['acct']] = x.json['rows'][i]['quantity']
    return ret

def get_state(contract, contract_owner, accts, dfs, p=False):

    stake_params = getStakeParams(contract.table('stakes',contract_owner))
    bps = getBoidpowers(contract.table('boidpowers', contract_owner))
    for account_num, acct in enumerate(accts):
        account = 'account%d' % (account_num + 1)
        acct_balance = getBalance(contract.table("accounts", acct))
        staked_tokens = float(stake_params[account]['staked'].split()[0]) \
            if account in stake_params.keys() else 0.0
        acct_bp = float(bps[account]) if account in bps.keys() else 0.0
        print(account_num)
        dfs[account_num] = dfs[account_num].append({
            'boid_power': acct_bp,
            'staked_boid_tokens': staked_tokens,
            'unstaked_boid_tokens': acct_balance - staked_tokens,
            'total_boid_tokens': acct_balance
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

##########################################################################################

test_accts = []

class Test(unittest.TestCase):
    def stats():
        eosf.print_stats(
            [master, boid_token, *test_accts],
            [
                "core_liquid_balance",
                "ram_usage",
                "ram_quota",
                "total_resources.ram_bytes",
                "self_delegated_bandwidth.net_weight",
                "self_delegated_bandwidth.cpu_weight",
                "total_resources.net_weight",
                "total_resources.cpu_weight",
                "net_limit.available",
                "net_limit.max",
                "net_limit.used",
                "cpu_limit.available",
                "cpu_limit.max",
                "cpu_limit.used"
            ]
        )


    @classmethod
    def setUpClass(cls):
        eosf.SCENARIO('''
        The ``master`` account sponsors the ``boid_token`` account
        that hosts the boidtoken contract and issues BOID tokens.
        There are ten test accounts ``accts``.
        We are testing the contract robustness to legal and illegal
        token staking scenarios.
        ''')

        testnet.verify_production()
                
        eosf.create_master_account("master", testnet)

        if arg_buy_ram > 0:
            master.buy_ram(arg_buy_ram)

        try:
            # Create contract owner account: boid_token
            eosf.create_account('boid_token', master, account_name='boidtoken123',
                buy_ram_kbytes=INITIAL_RAM_KBYTES,
                stake_net=INITIAL_STAKE_NET,
                stake_cpu=INITIAL_STAKE_CPU)
        except:
            pass

        for i in range(NUM_ACCOUNTS):
            eosf.create_account(
                'acct{}'.format(i),
                master,
            )
            test_accts.append(eval('acct{}'.format(i)))

        if not testnet.is_local():
            cls.stats()

        contract = eosf.Contract(boid_token, CONTRACT_WORKSPACE)
        if arg_build:
            contract.build(force=False)

        try:
            contract.deploy(payer=master)
        except eosf.errors.ContractRunningError:
            pass

        # Set up boid_token account as issuer of BOID
        eosf.COMMENT('''
        Attempting to create BOID token.
        This might fail if the BOID token has already been created:
        ''')
        try:
            boid_token.push_action(
                'create',
                {
                    'issuer': boid_token,
                    'maximum_supply': '{:.4f} BOID'.format(MAX_BOID_SUPPLY)
                }, [boid_token])
        except eosfactory.core.errors.Error as e:
            if "token already exists" in e.message:
                eosf.COMMENT('''
                BOID token already exists 
                ''')

                time.sleep(3)
            else:
                eosf.COMMENT('''
                The error is different than expected.
                ''')
                raise Error(str(e))

            stake_params = getStakeParams(
                    contract.table('stakes',boid_token))
            stakebreak(boid_token, boid_token, '1')
            for acct in test_accts:
                if acct.name in stake_params and\
                   float(
                    stake_params[acct.name]['staked'].split()[0]) > 0:
                    unstake(
                        boid_token,
                        acct,
                        acct,
                        'memo'
                    )
            # Sleep to wait for next block to clear so we don't
            # accidentally have duplicate transactions
            time.sleep(1)

    def setUp(self):
        pass

    def test_min_stake(self):
        eosf.COMMENT('''
        Staking less than minimum stake. This should fail. 
        ''')

        setTokenAmount(boid_token, test_accts[0], boid_token,
                5000, issuer=True)
        try:
            initStaking(boid_token, boid_token)
            stake(
                boid_token,
                test_accts[0],
                '%.4f BOID' % getBalance(
                    boid_token.table("accounts",test_accts[0])),
                'memo'
            )
        except eosfactory.core.errors.Error as e:
            print(e)

        time.sleep(1)

    def test_stake_single_acct(self):
        eosf.COMMENT('''
        Staking with single account. 
        ''')

        setTokenAmount(boid_token, test_accts[1], boid_token,
                100000, issuer=True)
        stakebreak(boid_token, boid_token, '1')
        stake(
            boid_token,
            test_accts[1],
            '%.4f BOID' % getBalance(
                boid_token.table("accounts",test_accts[1])),
            'memo'
        )
        setBoidpower(boid_token, boid_token, test_accts[1], 0)
        setAutostake(boid_token, test_accts[1], 0)
        stakebreak(boid_token, boid_token, '0')
        # run test over time
        for t in range(TEST_DURATION[0]):
            time.sleep(WEEK_WAIT)
            print('\n/-------------------- week %d --------------------------------\\' % (t+1))
            claim(boid_token, boid_token, test_accts[1])
            print('\\--------------------- week %d ---------------------------------/' % (t+1))

        unstake(boid_token, test_accts[1], boid_token, 'memo')

        # end season
        stakebreak(boid_token, boid_token, '1')

    def tearDown(self):
        pass

    @classmethod
    def tearDownClass(cls):
        if testnet.is_local():
            eosf.stop()
        else:
            cls.stats()



testnet     = None
arg_build   = False
arg_save    = False
arg_buy_ram = 0

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='''
    This is a unit test for the ``tic-tac-toe`` smart contract.
    It works both on a local testnet and remote testnet.
    The default option is local testnet.
    ''')

    parser.add_argument(
        "alias", nargs="?",
        help="Testnet alias")

    parser.add_argument(
        "-t", "--testnet", nargs=4,
        help="<url> <name> <owner key> <active key>")

    parser.add_argument(
        "-r", "--reset", action="store_true",
        help="Reset testnet cache")

    parser.add_argument(
        "-s","--save",
        action="store_true",
        help="save test data to csv file in ./results")

    parser.add_argument(
        "-b","--build",
        action="store_true",
        help="build new contract ABIs")

    parser.add_argument(
        "--buy_ram",
        help="buy more ram for master account")

    args = parser.parse_args()

    testnet = eosf.get_testnet(args.alias, args.testnet, reset=args.reset)
    testnet.configure()

    if args.reset and not testnet.is_local():
        testnet.clear_cache()

    if args.build:
        arg_build = True

    if args.save:
        arg_save = True

    if args.buy_ram and int(args.buy_ram.split()[0]) > 0:
        arg_buy_ram = int(args.buy_ram.split()[0])

    unittest.main(argv=[sys.argv[0]])