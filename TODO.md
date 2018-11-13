1. Install EOS (this took about 30 minutes)
https://github.com/EOSIO/eos

2. Install EOSFactory (look through the User Documentation until you feel comfortable using it)
https://github.com/tokenika/eosfactory

3. Fork Token-Staking-Upgrade repo (make sure to pick the branch "testboidpower")
https://github.com/han-so1omon/Token-Staking-Upgrade/tree/testboidpower

4. do a quick comparison of CI tools for eos development using GitHub
	CI = Continuous Integration, aka testing stuff automatically when it's pushed to GitHub

5. Make the ricardian documents for boidtoken.cpp, and make sure local_test.py runs error free.

6. Go to the file local_test.py in Token-Staking-Upgrade/tests/, starting on line 83 you'll see:

	# Set up boid staking contract to boid.stake

	# Set up boid power contract to boid.power

	# Run staking tests with acct1 and acct2

Thats all thats left to be done for this test.