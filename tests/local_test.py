from eosfactory.eosf import *
import sys

''' NOTES:

	TO DO:



	'''

# CONTRACT_PATH = '~/rooms/BOID/Token-Staking-Upgrade'
CONTRACT_PATH = sys.path[0] + '/../'

if __name__ == '__main__':

	# start single-node local testnet
	reset()

	# create master account from which
	# other account can be created
	# accessed via global variable: master
	create_master_account("master")

	# 3) Create 6 accounts: eosio_token, boid, boid_stake, boid_power, acct1, acct2
	create_account('eosio_token', master, account_name='eosio.token')
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
	sys.exit()

	############# now we can call functions ##############
	########## (aka actions) from the contract! ##########

	# account.push_action(
	#		action_name,
	#		action_arguments_in_json,
	#		account_whose_permission_is_needed
	# )
	host.push_action(
		'create', {
			'issuer': master,
			'maximum_supply': '1000000000.0000 EOS'
		}
	)
	# host.push_action(
	# 	'create', {
	# 		'issuer': master,
	# 		'maximum_supply': "boid", "1000000000.0000 BOID"
	# 	}
	# )



