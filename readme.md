TODO fix staking
TODO press briefing for new contract
  Relevant actions
  Action variables
  User interface screenshot
TODO profitability calculator
  Equation written out
  Javascript
  Option to put in stake params manually or check from current stake params
TODO deploy token contract on kylin for testing with chintai
TODO update config table api so that table row can be deleted
TODO rewrite eosdac constitution for BOID
TODO write ricardian contracts
TODO cannot update timeout of token delegation
TODO tests for ram payer
TODO separate transstaked balance for stake and delegation tables
TODO fix stake parameters
TODO add cap to powered stake and get rid of max stake bonus rate. Perhaps 5% of
total tokens staked
TODO worker proposal fund
TODO percentage of stake bonus to staked account
TODO boidpower decay

Potential issue: call transtaked when account has nothing staked
  - Thus, we must pay for stake table RAM

TODO track and automatically change stake/power difficulty
parameters with off-chain script. Base this off of desired
inflation rate, monthly sliding average powered
stake and boidpower. Set target inflation at 10-15%.

TODO run the state history plugin and see if the actions show up in state
history

TODO regrab unused airdropped tokens
  Make list of accounts that have not used the boidtoken contract

#Testing phase for BoidDac
##Tests
Install required libraries:
NodeJS
> `npm i`
Python, either globally:
> `sudo pip install numpy pandas plotly`
or locally: 
> `pip install --user numpy pandas plotly`

These tests can be run locally or on CryptoKylin net

###All tests
Test contracts:
1113testtest (token proxy contract)

Files:
test/kylinDeploy.js

For testing of edge cases and others

To deploy a new version if this breaks:
> `npm --contract=<contract_dir> --account=<account_name> run-script kylin_deploy`

###Contract replace
Test contracts:
1111testtest (token proxy contract)
1112testtest (maintenance contract)

Files:
test/updateproc/

Tests the ability to replace contract with updated contract and
have identical data with no table read errors
(from `BoidDac` directory)
Set initial contract data

0) If using test data, set the old contract and fill it with data.
> `npm run-script m00`
If the old contract account has old code in it, add the `--contract=0` option
> `npm --contract=0 run-script m00`
If the old contract account has updated code in it, add the `--contract=1` option
> `npm --contract=1 run-script m00`
Add in the test data
> `npm run-script m01`

1) Set maintenance contract data
> `npm run-script m1`

2) Erase existing data from old contract
> `npm run-script m2`

3) Deploy updated contract
> `npm run-script m3`

4) Replace existing data in updated contract
> `npm run-script m4`

5) Check that the existing data matches the original data
> `npm run-script m5`

6) Erase the maintenance contract
> `npm run-script m6`

###Bonuses
Test contracts:
jimjamjimjam (token proxy contract)

Files:
test/kylinToken2Spec.js
test/results.py

Tests BOID bonus parameters for generating stake and power bonuses.

0) Set test script to run
> `chmod +x ./test/runKylinTokenTest.sh`

1) Run tests
> `./test/runKylinTokenTest.sh`

Set appropriate parameters by editing the following functions in test/kylinToken2Spec.js:

`setminstake` - set minimum BOID stake

`setpowerdiff` - set power difficulty (+diff -> +powerbonus)

`setpowerdiv` - set power divisor (+div -> -powerbonus)

`setpowerrate` - set power max rate (+rate -> +powerbonus potential)

`setstakediff` - set stake difficulty (+diff -> +stakebonus)

`setstakediv` - set stake divisor (+div -> -stakebonus)

`setstakerate` - set stake max rate (+rate -> +stakebonus potential)

`setpwrstkdiv` - set powered stake divisor (+div -> -stakebonus potential)

##Notes
Claim requires issuer permission to transfer tokens
Stake creates a map of {time: amount} relationships

#Serve docs
##Dependencies
###Npm
- `vuepress@next`
###Command to view docs locally
`chmod +x ./generate_docs.sh`
`chmod +x ./serve_docs.sh`
`./generate_docs.sh`
`./serve_docs.sh`
