import eosfactory.eosf as eosf
import sys
import os
import json
import argparse
import subprocess


# TODO: find a better way to reference the eos/contracts/eosio.token
#eosBuild = os.getenv('EOS_BUILD')
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
        'eosio_token', master, account_name='eosio.token')
    eosf.create_account(
        'eos', master, account_name='eos')
    eosf.create_account(
        'boid', master, account_name='boid')
    eosf.create_account(
        'boid_stake', master, account_name='boid.stake')
    eosf.create_account(
        'boid_power', master, account_name='boid.power')
    eosf.create_account(
        'acct1', master, account_name='account1')
    eosf.create_account(
        'acct2', master, account_name='account2')

    # make build directory if it does not exist
    build_dir = os.path.join(BOID_STAKE_CONTRACT_PATH, 'build')
    if not os.path.exists(build_dir):
        os.mkdir(build_dir)

    # create reference to the token staking contract
    eosioToken_c = eosf.Contract(
        eosio_token, EOS_TOKEN_CONTRACT_PATH)
    boidStake_c = eosf.Contract(
        boid_stake, BOID_STAKE_CONTRACT_PATH)

    # build the token staking contract
    if args.build:
        eosioToken_c.build()
        boidStake_c.build()

    # deploy the token staking contract on the testnet
    eosioToken_c.deploy()
    boidStake_c.deploy()

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
    '''
    setAccountPermission(
            acct1.name, 'active',
            transferPermission(acct1.active_key.key_public, boid_stake.name),
            'owner')

    ############# now we can call functions ##############
    ########## (aka actions) from the contract! ##########

    # Set up master as issuer of EOS and boid as issuer of BOID
    # account.push_action(
    #		action_name,
    #		action_arguments_in_json,
    #		account_whose_permission_is_needed)
    eosioToken_c.push_action(
        'create',
        {
            'issuer': eos,
            'maximum_supply': '1000000000.0000 EOS'
        }, [eosio_token])

    eosioToken_c.push_action(
        'create',
        {
            'issuer': boid,
            'maximum_supply': '1000000000.0000 BOID'
        }, [eosio_token])

    # Import eosio.token into staking
    

    # Distribute initial quantities of EOS & BOID
    eosioToken_c.push_action(
        'issue',
        {
            'to': acct1,
            'quantity': '1000.0000 EOS',
            'memo': 'memo'
        }, [eos])
    eosioToken_c.push_action(
        'issue',
        {
            'to': acct2,
            'quantity': '2000.0000 EOS',
            'memo': 'memo'
        }, [eos])
    eosioToken_c.push_action(
        'issue',
        {
            'to': acct1,
            'quantity': '1000.0000 BOID',
            'memo': 'memo'
        }, [boid])
    eosioToken_c.push_action(
        'issue',
        {
            'to': acct2,
            'quantity': '2000.0000 BOID',
            'memo': 'memo'
        }, [boid])
    print(eosioToken_c.table("accounts", acct1))
    print(eosioToken_c.table("accounts", acct2))

    # Initialize boid staking contract
    boidStake_c.push_action(
        'create',
        {
            'issuer': boid,
            'maximum_supply': '1000000000.0000 BOID'
        }, [boid_stake])
    # TODO find way to print total number of boid tokens
    # to determine if this function above is minting more coins
    # or if its alocating pre-existing coins
    boidStake_c.push_action(  # running - sets payouts to on
        'running',
        {
            'on_switch': '1',
        }, [boid_stake])
    # initstats - reset/setup configuration of contract
    boidStake_c.push_action(
        'initstats',   
        {}, [boid_stake])

    # insert - ad hoc way to give accounts boid power
    #boid_stake.get_info()
    boidStake_c.push_action(
        'setnewbp',
        {
            'acct': acct1,
            'boidpower': 10
        }, [boid_stake, acct1])

    boidStake_c.push_action(
        'setnewbp',
        {
            'acct': acct2,
            'boidpower': '10000'
        }, [boid_stake, acct2])
    print(getBoidpowers(boidStake_c.table('boidpowers', boid_stake)))

    # Run staking tests with acct1 and acct2
    print(boid_stake.info())

    ''' Simple transfer action
    eosioToken_c.push_action(
        'transfer',
        {
            'from': acct1,
            'to': acct2,
            'quantity': '100.0000 BOID',
            'memo': ''
        }, permission = [(acct1)]
    ) 
    '''

    boidStake_c.push_action(
        'stake',
        {
            '_stake_account': acct1,
            '_stake_period': '1',
            '_staked': '100.0000 BOID'
        }, permission=[(acct1)]
    )

    print(eosioToken_c.table("accounts", acct1))
    print(eosioToken_c.table("accounts", acct2))
    print(eosioToken_c.table("accounts", boid_stake))
    '''
    boidStake_c.push_action(
        'stake',
        {
            '_stake_account': acct1,
            '_stake_period': '1',
            '_staked': '500.0000 BOID'
        }, permission=[acct1, (boid_stake, '@eosio.code')])
        #}, permission=[(acct1), (boid_stake, '@active'), (boid_stake, '@xfer')])
    boidStake_c.push_action(
        'stake',
        {
            '_stake_account': acct2,
            '_stake_period': '2',
            '_staked': '2000.0000 BOID'
        }, [acct2])
    '''

    #print(getBalance(boidStake_c.table("accounts", acct1)))
    #print(getBalance(boidStake_c.table("accounts", acct2)))

    #print(getStakeParams(boidStake_c.table('stakes',boid_stake)))

    # stop the testnet and exit python
    eosf.stop()
    sys.exit()
