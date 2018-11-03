#!/bin/bash
########### Command line arguments
POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
  -d|--datafile)
  DATAFILE="$2"
  shift
  shift
  ;;
esac
done

set -- "${POSITIONAL[@]}"

echo "${DATAFILE}"

########### Usage & convenience: TODO clean up

if [ -z ${EOS_LOCAL_TEST_DATA} ]; then
  mkdir -p ${HOME}/eos_test/data
  EOS_LOCAL_TEST_DATA=${HOME}/eos_test/data
fi
if [ -z ${EOS_LOCAL_TEST_WALLETS} ]; then
  mkdir -p ${HOME}/eos_test/wallet
  EOS_LOCAL_TEST_WALLETS=${HOME}/eos_test/wallets
fi
if [ -z ${EOS_BUILD} ]; then
  echo "Must set EOS_BUILD env var" 
  exit -1
fi

if [ -d ${BOID}/Token-Staking-Upgrade/build ]; then
  CONTRACT_DIR=${BOID}/Token-Staking-Upgrade/build/
else
  echo "Could not find BOID token-staking contract directory"
  exit -1
fi

NODEHOST="127.0.0.1"
NODEPORT="8888"
WALLETHOST="127.0.0.1"
WALLETPORT="8900"

cleos_local_test="cleos -u http://${NODEHOST}:${NODEPORT} --wallet-url http://${WALLETHOST}:${WALLETPORT}"
nodeos_local_test_fresh="nodeos --data-dir ${EOS_LOCAL_TEST_DATA} --config-dir ${EOS_LOCAL_TEST_DATA} --config nodeos-config.ini --delete-all-blocks --genesis-json ${EOS_LOCAL_TEST_DATA}/genesis.json > ${EOS_LOCAL_TEST_DATA}/stdout.text 2> ${EOS_LOCAL_TEST_DATA}/stderr.txt & echo $! > ${EOS_LOCAL_TEST_DATA}/nodeos.pid"
keosd_local_test="keosd --wallet-dir ${EOS_LOCAL_TEST_WALLETS} --config-dir ${EOS_LOCAL_TEST_DATA} --config keosd-config.ini --unix-socket-path ${EOS_LOCAL_TEST_DATA}/keosd.sock --http-server-address ${WALLETHOST}:${WALLETPORT} & echo $! > ${EOS_LOCAL_TEST_WALLETS}/wallet.pid"
kill_nodeos_local_test="pkill $(cat ${EOS_LOCAL_TEST_DATA}"/nodeos.pid")"
kill_keosd_local_test="pkill $(cat ${EOS_LOCAL_TEST_WALLETS}"/wallet.pid")"

start=$(date +%s%N)
currTime='date +%s%N'

########### Functions
writeHeader() {
  printf "Writing data to $DATAFILE\n"
  printf \
    "time,\tacct1:EOS,\tacct1:BOID,\tacct1:BPOW,\tacct1:type,\tacct2:EOS,\tacct2:BOID\t,acct2:BPOW,\tacct2:type\n"\
    > ${DATAFILE}
}

appendData(){
  printf "Appending data to $DATAFILE\n"
  dt="$(($(date +%s%N)-$start))"
  setNextCurrencyBalances
  setNextBoidpower
  printf \
    "$dt,\t$acct1_eos,\t$acct1_boid,\t$acct1_bpow,\t$acct1_type,\t$acct2_eos,\t$acct2_boid,\t$acct2_bpow,\t$acct2_type\n" \
    >> ${DATAFILE}
}

setNextCurrencyBalances(){
  acct1_eos="$(cleos_local_test get currency balance eosio.token acct1 EOS \
    | tr -d '[:alpha:]')"
  acct1_boid="$(cleos_local_test get currency balance eosio.token acct1 BOID \
    | tr -d '[:alpha:]')"
  acct2_eos="$(cleos_local_test get currency balance eosio.token acct2 EOS \
    | tr -d '[:alpha:]')"
  acct2_boid="$(cleos_local_test get currency balance eosio.token acct2 BOID \
    | tr -d '[:alpha:]')"
}

setNextBoidpower(){
  acct1_bpow="$(cleos_local_test push action boid.stake get_boidpower \
    '[ "acct1" ]' -p acct1)"
  acct2_bpow="$(cleos_local_test push action boid.stake get_boidpower \
    '[ "acct2" ]' -p acct2)"
}

setStakeType(){
  acct1_type="$(cleos_local_test push action boid.stake get_staketype \
    '[ "acct1" ]' -p acct1)"
  acct2_type="$(cleos_local_test push action boid.stake get_staketype \
    '[ "acct2" ]' -p acct2)"
}

########### Boid staking setup and test
echo "Beginning BOID Token-Staking test"
# 1) Start keosd & nodeos
echo "Starting keosd"
keosd_local_test
sleep 0.5

echo "Starting nodeos"
nodeos_local_test_fresh
sleep 0.5

# 2) Unlock/create wallets
#TODO wallet creation
#TODO change to acct1,acct2,...
echo "Unlocking wallets"
cleos_local_test wallet unlock -n errol_test --password $(errol_test_pword)
cleos_local_test wallet unlock -n dude_test --password $(dude_test_pword)
cleos_local_test wallet import -n errol_test --private-key $(eosio_key)
cleos_local_test wallet import -n dude_test --private-key $(eosio_key)
sleep 0.2

# 3) Create 6 accounts: eosio.token, boid, boid.stake, boid.power, test1, test2
echo "Creating test accounts"
cleos_local_test set contract eosio ${EOS_BUILD}/contracts/eosio.bios -p eosio@active
cleos_local_test create account eosio eosio.token $(eosio_pubkey) $(eosio_pubkey)
cleos_local_test create account eosio boid        $(eosio_pubkey) $(eosio_pubkey)
cleos_local_test create account eosio boid.stake  $(eosio_pubkey) $(eosio_pubkey)
cleos_local_test create account eosio boid.power  $(eosio_pubkey) $(eosio_pubkey)
cleos_local_test create account eosio acct1       $(eosio_pubkey) $(eosio_pubkey)
cleos_local_test create account eosio acct2       $(eosio_pubkey) $(eosio_pubkey)
sleep 0.2

# 4) Set up token creation account to eosio.token
echo "Setting up token issuance authorities"
cleos_local_test set contract eosio.token ${EOS_BUILD}/contracts/eosio.token -p eosio.token
sleep 0.1

# 5) Set up eosio as issuer of EOS and boid as issuer of BOID
cleos_local_test push action eosio.token create \
  '[ "eosio", "1000000000.0000 EOS", 0, 0, 0]' -p eosio.token
cleos_local_test push action eosio.token create \
  '[ "boid", "1000000000.0000 BOID", 0, 0, 0]' -p eosio.token
sleep 0.1

# 6) Distribute initial quantities of EOS & BOID to test accounts
echo "Issuing initial tokens to test accounts"
cleos_local_test push action eosio.token issue \
  '[ "acct1", "1000.0000 EOS", "memo" ]' -p eosio
cleos_local_test push action eosio.token issue \
  '[ "acct2", "2000.0000 EOS", "memo" ]' -p eosio
cleos_local_test push action eosio.token issue \
  '[ "acct1", "1000.0000 BOID", "memo" ]' -p boid
cleos_local_test push action eosio.token issue \
  '[ "acct2", "2000.0000 BOID", "memo" ]' -p boid

# 7) Set up boid staking contract to boid.stake
echo "Setting up boid staking authorities"
cleos_local_test set contract boid.stake ${CONTRACT_DIR}/build/contracts/boidtoken -p boid.stake
sleep 0.1

# 8) Set up boid power contract to boid.power
echo "Setting up boid power authorities"
cleos_local_test set contract boid.power ${CONTRACT_DIR}/build/contracts/testboidpower -p boid.power
sleep 0.1

# 9) Run staking tests with test1 and test2
#TODO At this point maybe call a specified auxiliary script to run test conditions.
#     This will minimize commenting-in/out of new/old test-cases.

# Set up boid power
cleos_local_test push action boid.power create \
  '[ "boid.power", "100000.0000 BPOW" ]' -p boid.power@active

cleos_local_test push action boid.power insert \
  '[ "acct1", "10" ]' -p acct1

cleos_local_test push action boid.power insert \
  '[ "acct2", "10000" ]' -p acct2

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


# 10) End test
echo "Killing nodeos"
kill_nodeos_local_test
pkill nodeos # FIXME shouldn't need this
sleep 0.5

echo "Killing keosd"
kill_keosd_local_test
pkill keosd # FIXME shouldn't need this
sleep 0.5

echo "Test finished"
