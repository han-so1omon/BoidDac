1. Install EOS (this took about 30 minutes)
https://github.com/EOSIO/eos

2. Install EOSFactory (look through the User Documentation until you feel comfortable using it)
https://github.com/tokenika/eosfactory

3. do a quick comparison of CI tools for eos development using GitHub
	CI = Continuous Integration, aka testing stuff automatically when it's pushed to GitHub

<<<<<<< HEAD
5. Make the ricardian documents for boidtoken.cpp, and make sure local_test.py runs error free.
	ricardian contract = legal document explaining the intent of the smart contract, and identifying the Arbitration Forum to settle disputes that arise from the contract.
	See EOS constitution:
		Article VII - Open Source -Each Member who makes available a smart contract on this blockchain shall be a Developer. Each Developer shall offer their smart contracts via a free and open source license, and each smart contract shall be documented with a Ricardian Contract stating the intent of all parties and naming the Arbitration Forum that will resolve disputes arising from that contract.
	source: https://github.com/EOSIO/eos/blob/5068823fbc8a8f7d29733309c0496438c339f7dc/constitution.md
=======
4. Make the ricardian documents for boidtoken.cpp, and make sure local_test.py runs error free.
>>>>>>> a857a8472bfe1dac37a94930ecfe14d3681a5dc6
	need to write in actual intent for ricardians for:
		transfer
		setoverflow
		running
		stake
		claim
		unstake
		create
		checkrun
		rembonus
		runpayout
		initstats
		reqnewbp
		setnewbp
		printstake
		printbpow

5. Go to the file local_test.py in Token-Staking-Upgrade/tests/, starting on line 83 you'll see comments:

	Set up boid staking contract to boid.stake

	Set up boid power contract to boid.power

	Run staking tests with acct1 and acct2

Thats all thats left to be done for this test.
