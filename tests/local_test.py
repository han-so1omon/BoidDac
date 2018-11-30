import eosfactory.eosf as eosf
import sys
import os
import json
import argparse


# TODO: find a better way to reference the eos/contracts/eosio.token
#eosBuild = os.getenv('EOS_BUILD')
eosBuild = os.getenv('EOS_SRC')
EOS_TOKEN_CONTRACT_PATH = eosBuild + '/contracts/eosio.token'
if eosBuild == '' or eosBuild == None:
    raise ValueError(
            'EOS_BUILD environment variable must be set')

BOID_STAKE_CONTRACT_PATH = \
    os.path.join(os.path.dirname(os.path.abspath(__file__)), '..')

assetDict = {'BOID': 0, 'EOS': 1}
getAssetQuantity = lambda x: float(x.split()[0])
getAssetType = lambda x: assetDict(x.split()[1])

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

# TODO: the file paths are different for unknown reasons
def contract_built(contract_path, contract_name):
    base_path = os.path.join(contract_path, 'build', contract_name)
    # print(base_path)
    abi_exists = os.path.exists(base_path+'.abi')
    # print(abi_exists)
    wast_exists = os.path.exists(base_path+'.wast')
    # print(wast_exists)
    wasm_exists = os.path.exists(base_path+'.wasm')
    # print(wasm_exists)
    return abi_exists and wast_exists and wasm_exists

'''
    args:
        contractAcct - contract with reqnewbp action
        acct - account requesting new boidpower
        auth - account to authorize reqnewbp (aka contract owner)
    '''
def test_reqnewbp(contractAcct, acct, auth):
    print('before')
    print(contractAcct.table("accounts", acct))
    print('after')
    contractAcct.push_action(
        'reqnewbp',
        {
            'owner': acct
        }, [auth])
    print(contractAcct.table("accounts", acct))

#TODO
def getCurrencyBalance(acct):
    '''
  acct2_boid="$(cleos_local_test get currency balance eosio.token acct2 BOID \
    | tr -d '[:alpha:]')"
    '''
    pass

#TODO
def getBoidpower(acct):
    '''
  acct2_bpow="$(cleos_local_test push action boid.stake printbpow \
    '[ "acct2" ]' -p boid.stake | sed -n 2p | sed 's/[^0-9]*//g')"
    '''
    pass

#TODO
def getStakeType(acct):
    '''
  acct1_type="$(cleos_local_test push action boid.stake printstake \
    '[ "acct1" ]' -p boid.stake | sed -n 2p | sed 's/[^0-9]*//g')"
    '''
    pass

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

    #FIXME Issue boid before staking test
    #      Why is this necessary?
    #       Other than permissions on stake action...
    #cleos_local_test push action boid.stake issue \
    #   '[ "acct2", "2000.0000 BOID", "" ]' -p boid
    boidStake_c.push_action(
        'issue',
        {
            'to': acct1,
            'quantity': '1000.0000 BOID',
            'memo': ''
        }, [boid])
    boidStake_c.push_action(
        'issue',
        {
            'to': acct2,
            'quantity': '2000.0000 BOID',
            'memo': ''
        }, [boid])
    print(boidStake_c.table("accounts", acct1))
    print(boidStake_c.table("accounts", acct2))

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
    boidStake_c.push_action(
        'stake',
        {
            '_stake_account': acct1,
            '_stake_period': '1',
            '_staked': '500.0000 BOID'
        }, [acct1])
    boidStake_c.push_action(
        'stake',
        {
            '_stake_account': acct2,
            '_stake_period': '2',
            '_staked': '2000.0000 BOID'
        }, [acct2])

    print(getBalance(boidStake_c.table("accounts", acct1)))
    print(getBalance(boidStake_c.table("accounts", acct2)))

    print(getStakeParams(boidStake_c.table('stakes',boid_stake)))

    # stop the testnet and exit python
    eosf.stop()
    sys.exit()
