#TODO images instead of text
# Token-Staking-Upgrade
Modification of EDNA token staking contract for boid token

# Token staking basics
## Investor:
1) Invest in Boid using EOS tokens
2) Receive Boid tokens
2a) Not allowed to withdraw Boid tokens for a period of time
3) Withdraw/use Boid tokens after some period of time

## Boid:
1) Receive in investment in EOS tokens
2) Pay out Boid tokens to investors at some exchange rate
3) Use EOS tokens for funding of boid

# Token staking vs token sale
## Token staking
Frozen investment
Like certification of deposit or bond

## Token selling
Exchange of tokens at fixed rate

## Benefits
Token staking preferred bc investors are given investment bonuses
Token staking preferred bc token trading is encouraged once tokens are worth
substantial value. Ideally there is minimal risk for holding and selling, as the
initial purchase price should be low
Token selling preferred bc higher immediate token liquidity
...

## Testing
1) Set environment variables and aliases
  - `EOS_LOCAL_TEST_DATA` : Contains nodeos-config.ini and keosd-config.ini
  - `EOS_LOCAL_TEST_WALLETS` : Contains .key, .pword, .wallet, and wallet.pid
  - `EOS_BUILD` : Contains build artifacts of `eosio/eos/` repo. E.g. contract paths
  - `BOID` : Contains BOID repos. E.g. Token-Staking-Upgrade
  - `errol_test_pword` : alias to wallet1 password
  - `dude_test_pword` : alias to wallet2 password
  - `errol_test_key` : alias to account1 key
  - `errol_test_pubkey` : alias to account1 public key
  - `dude_test_key` : alias to account2 key
  - `dude_test_pubkey` : alias to account2 public key
  - `eosio_key` : alias to system account key
  - `eosio_pubkey` : alias to system account public key

2) run 
```
  $ cd testing && chmod +x ./local_test_setup.sh
  $ bash -i ./local_test_setup.sh
```

## Building
make : Full version
make test : Include testboidpower contract

## Utilities
Convenience tools for file-based access of EOS keys and passwords stored in
plaintext files

### extract_key.sh
```
  $ chmod +x extract_key.sh
  $ ./extract_key.sh --type <public|private> --file <walletname>.key
```

```
# .key file format
Private key: <pubkey>
Public key: <key>
```

### extract_wallet_password.sh
```
  $ chmod +x extract_wallet_password.sh
  $ ./extract_wallet_password.sh --file <walletname>.pword
```

```
# .pword file format
"<pword>"
```

