import json
import subprocess
import sys

# get_table_by_scope
# source: https://developers.eos.io/eosio-nodeos/reference#get_table_by_scope

''' Get accounts:
cmd = 'curl --request POST --url' + \
' http://api.eosn.io/v1/chain/get_table_by_scope' + \
' --data \'{"code":"boidcomtoken", "table":"accounts", "limit":1000000000}\' > tmp.txt'
subprocess.run(cmd, shell=True)
'''

''' Get stats table:
cmd = 'curl --request POST --url' + \
' http://api.eosn.io/v1/chain/get_table_rows' + \
' --data \'{"scope": "......24d5bo2", "code":"boidcomtoken",' + \
' "table":"stat", "json":true} > stat.txt'
subprocess.run(cmd, shell=True)
# output: {"rows":[{"supply":"6569785015.6634 BOID","max_supply":"25000000000.0000 BOID","issuer":"boidcomtoken"}],"more":false}
'''

# this fn. gets all account names from the temp_filename
def parse_accounts(temp_filename):
	accts_list = []
	f = open('tmp.txt', 'r')
	j = json.load(f)
	rows = j['rows']
	for row in rows:
		acct = row['scope']
		accts_list.append(acct)
	return accts_list

# this fn. requests the accounts in batches and puts them in a list and in a file
def get_accounts(limit=10000, temp_filename='tmp.txt'):

	all_accts = []  # list to store all accounts

	# first queri only has limit
	cmd = 'curl --request POST --url' + \
		' http://api.eosn.io/v1/chain/get_table_by_scope' + \
		' --data \'{"code":"boidcomtoken", "table":"accounts", ' + \
		'"limit":%d}\' > %s' \
		% (limit, temp_filename)
	subprocess.run(cmd, shell=True)
	accts_list = parse_accounts(temp_filename)
	all_accts += accts_list
	last_result = accts_list[-1]

	# subsequent queris use the last result of the previous queri
	# as the lower_bound of the next queri
	while True:
		# print('last_result = ' + last_result)
		cmd = 'curl --request POST --url' + \
			' http://api.eosn.io/v1/chain/get_table_by_scope' + \
			' --data \'{"code":"boidcomtoken", "table":"accounts", ' + \
			'"lower_bound":"%s", "limit":%d}\' > %s' \
			% (last_result, limit, temp_filename)
		subprocess.run(cmd, shell=True)
		accts_list = parse_accounts(temp_filename)
		if len(accts_list) == 1:
			break
		else:
			all_accts += accts_list[1:]  # the 1st element equals last_result
			last_result = accts_list[-1]

	return all_accts

## test get_accounts()
#all_accts = get_accounts()
#print('len(all_accts) = %d' % len(all_accts))

