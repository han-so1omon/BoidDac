#!/bin/bash
echo "TEST 1 : mid investment, mid boidpower "
# Set up boid power
cleos_local_test push action boid.power create \
  '[ "boid.power", "100000.0000 BPOW" ]' -p boid.power@active

cleos_local_test push action boid.power insert \
  '[ "acct1", "1000" ]' -p acct1

cleos_local_test push action boid.power insert \
  '[ "acct2", "1000" ]' -p acct2

# Set up boid token-staking
# Stake: [ account, {1:daily | 2:weekly}, amount]
cleos_local_test  push action boid.stake create \
  '[ "boid.token", "1000000000.0000 BOID" ]' -p boid.token@active

cleos_local_test  push action boid.stake initstats \
  '[ ]' -p boid.stake

cleos_local_test  push action boid.stake runpayout \
  '[ ]' -p boid.stake

cleos_local_test push action boid.stake stake \
  '[ "acct1", "1", "1000.0000 BOID" ]' -p acct1

cleos_local_test push action boid.stake stake \
  '[ "acct2", "2", "2000.0000 BOID" ]' -p acct2

# Collect initial data
setStakeType
writeHeader
appendData

counter=0
while [ $counter -lt 10 ]
do
sleep(1)
  cleos_local_test  push action boid.stake runpayout \
    '[ ]' -p boid.stake

  cleos_local_test push action boid.stake claim \
    '[ "acct1" ]' -p acct1

  cleos_local_test push action boid.stake claim \
    '[ "acct2" ]' -p acct2

  appendData

  counter=$(( $counter + 1))
done


