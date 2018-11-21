import eosfactory.eosf as eosf
import sys
import os

EOS_TOKEN_CONTRACT_PATH = os.getenv('EOSFACTORY_DIR') 
if EOS_TOKEN_CONTRACT_PATH == '':
    raise ValueError(
            'EOSFACTORY_DIR environment variable must be set')

BOID_STAKE_CONTRACT_PATH =
    os.path.dirname(os.path.abspath(__file__)) + '/../'

TEST_BOIDPOWER_CONTRACT_PATH =
    os.path.dirname(os.path.abspath(__file__)) + '/../tests'

#TODO return something?
def reqnewbp(contractAcct, acct):
    contractAcct.push_action(
        'reqnewbp',
        {
            'owner': acct
        }, [contractAcct])

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
    testBoidpower_c = eosf.Contract(
            boid_power, TEST_BOIDPOWER_CONTRACT_PATH)

    # build the token staking contract
    boidStake_c.build()
    testBoidpower_c.build() # May want to pre-build and go straight to deploy

    # deploy the token staking contract on the testnet
    eosioToken_c.deploy()
    boidStake_c.deploy()
    testBoidpower_c.deploy()

    ############# now we can call functions ##############
    ########## (aka actions) from the contract! ##########

    # Set up master as issuer of EOS and boid as issuer of BOID
    # account.push_action(
    #		action_name,
    #		action_arguments_in_json,
    #		account_whose_permission_is_needed)
    eosToken_c.push_action(
            'create',
            {
                'issuer': eos,
                'maximum_supply': '1000000000.0000 EOS'
            }, [eos, eosio_token])

    eosToken_c.push_action(
            'create',
            {
                'issuer': boid,
                'maximum_supply': '1000000000.0000 BOID'
            }, [boid, eosio_token])

    # Distribute initial quantities of EOS & BOID
    eosToken_c.push_action(
            'issue',
            {
                'to': acct1,
                'quantity': '1000.0000 EOS',
                'memo': 'memo'
            }, [eos])
    eosToken_c.push_action(
            'issue',
            {
                'to': acct2,
                'quantity': '2000.0000 EOS',
                'memo': 'memo'
            }, [eos])
    eosToken_c.push_action(
            'issue',
            {
                'to': acct1,
                'quantity': '1000.0000 BOID',
                'memo': 'memo'
            }, [boid])
    eosToken_c.push_action(
            'issue',
            {
                'to': acct2,
                'quantity': '2000.0000 BOID',
                'memo': 'memo'
            }, [boid])

    # Set up accounts with boidpower
    testBoidpower_c.push_action(
            'create',
            {
                'issuer': boid_power,
                'maximum_supply': '100000.0000 BPOW'
            }, [boid_power])
    # insert - ad hoc way to give accounts boid power
    testBoidpower_c.push_action(
            'insert',
            {
                'user': acct1,
                'boidpower': '10'
            }, [acct1])
    testBoidpower_c.push_action(
            'insert',
            {
                'user': acct2,
                'boidpower': '10000'
            }, [acct2])

    # Initialize boid staking contract
    boidStake_c.push_action(
            'create',
            {
                'issuer': boid_stake,
                'maximum_supply', '1000000000.0000 BOID'
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

    # Run staking tests with acct1 and acct2
    boidStake_c.push_action(
            'stake',
            {
                '_stake_account': acct1,
                '_stake_period': '1',
                '_staked': '1000.0000 BOID'
            }, [boid_stake])
    boidStake_c.push_action(
            'stake',
            {
                '_stake_account': acct2,
                '_stake_period': '2',
                '_staked': '2000.0000 BOID'
            }, [boid_stake])

    print(eosToken_c.table("accounts", acct1))
    print(eosToken_c.table("accounts", acct2))

    # stop the testnet and exit python
    eosf.stop()
    exit()
