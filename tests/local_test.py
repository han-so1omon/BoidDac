from eosfactory.eosf import *
import sys


CONTRACT_PATH = sys.path[0] + '/../'

if __name__ == '__main__':

	# start single-node local testnet
	reset()

	# create master account from which
	# other account can be created
	# accessed via global variable: master
	create_master_account('master')

	# Create 6 accounts: eosio_token, boid, boid_stake, boid_power, acct1, acct2
	create_account('boid', master, account_name='boid')
	create_account('boid_stake', master, account_name='boid.stake')
	create_account('boid_power', master, account_name='boid.power')
	create_account('acct1', master, account_name='account1')
	create_account('acct2', master, account_name='account2')


	# set which account will host the contract we're testing
	create_account('host', master)

	# create reference to the token staking contract
	contract = Contract(host, CONTRACT_PATH)

	# build the token staking contract
	contract.build()

	# deploy the token staking contract on the testnet
	contract.deploy()

	############# now we can call functions ##############
	########## (aka actions) from the contract! ##########

	# Set up master as issuer of EOS and boid as issuer of BOID
	# account.push_action(
	#		action_name,
	#		action_arguments_in_json,
	#		account_whose_permission_is_needed)
	host.push_action(
		'create',
		{'issuer': master,
			'maximum_supply': '1000000000.0000 EOS'},
		[master, host])

	host.push_action(
		'create',
		{'issuer': boid,
			'maximum_supply': '1000000000.0000 BOID'},
		[boid, host])

	# Distribute initial quantities of EOS & BOID to test accounts
	host.push_action(
		'issue',
		{'to': acct1,
		'quantity': '1000.0000 EOS',
		'memo': 'memo'},
		[master])
	host.push_action(
		'issue',
		{'to': acct2,
		'quantity': '2000.0000 EOS',
		'memo': 'memo'},
		[master])
	host.push_action(
		'issue',
		{'to': acct1,
		'quantity': '1000.0000 BOID',
		'memo': 'memo'},
		[boid])
	host.push_action(
		'issue',
		{'to': acct2,
		'quantity': '2000.0000 BOID',
		'memo': 'memo'},
		[boid])

	# Set up boid staking contract to boid.stake

	# Set up boid power contract to boid.power

	# Run staking tests with acct1 and acct2


	# stop the testnet
	stop()

