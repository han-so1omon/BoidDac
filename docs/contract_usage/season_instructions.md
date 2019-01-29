##General Process:

####OWNER starts stake break

USERs can: stake, unstake, toggle auto_stake (default value = false)

####OWNER ends stake break:

USERs can now: only toggle auto_stake

####on a weekly basis OWNER issues payouts to staked users

####at the end of the season:

####OWNER unstakes all users w/ auto_stake == false

####OWNER ends season and restarts stake break




##Scripts:

####Wallet Setup
open config.py in the season_scripts directory.

comment out the mainnet or the jungle test net variables depending if you want to go live or just test respectively

set WALLET_PASSWORD to the filepath to the file with the wallets password.

This file just contains the wallet password, and then a newline, and can be placed anywhere you want.

set WALLET_NAME to whatever you named your wallet.

####Open Wallet
run: python open_and_unlock_wallet.py

to quickly open and unlock your wallet in 1 easy step

if the contract is not already deployed, run:

	python deploy_initstats_and_running.py

else:

	skip that step

####Toggle Stake Break
run: python toggle_staking.py -s 1

to start the stake break. This is where users can stake their tokens

run: python toggle_staking.py -s 0

to end the stake break, and start the season. This is where anyone who staked can now recieve rewards for staking.

####Unstake Users
at the end of the season, run: python unstake_all.py

this will unstake all users who do have not turned on auto staking.

then run: python toggle_staking.py -s 1

again to start the next stake break. and the process repeats

