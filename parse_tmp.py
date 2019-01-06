
import json
import subprocess

cmd = 'curl --request POST --url' + \
' http://api.eosn.io/v1/chain/get_table_by_scope' + \
' --data \'{"code":"boidcomtoken", "table":"accounts", "limit":1000000000}\''
subprocess.run(cmd, shell=True)

# parsed_json = json.loads(json_string)

