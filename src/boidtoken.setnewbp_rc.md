# Action - `{{ setnewbp }}`

This Contract is legally binding and can be used in the event of a dispute. Disputes shall be settled through the standard arbitration process established by EOS.IO.

### Description

The `{{ setnewbp }}` action... 

INTENT: The intent of {{ setnewbp }} is to allow an EOS account to add additional BOID Tokens to the total payout bonus available for the current weekly payout. This most usually will come from the overflow account, but could come from elsewhere. Adding to the payout bonus is always at the sole discretion of the contract owner.    

TERM: This action lasts for the duration of the processing of the contract.

### Inputs and Input Types

The `{{ setnewbp }}` action setuires the following `inputs` and `input types`:

| Action | Input | Input Type |
|:--|:--|:--|
| `{{ setnewbp }}` | `{{ bpVar }}`<br/>`{{ acctVar }}`<br/>`{{ boidpowerVar }}` | `{{ account_name }}`<br/>`{{ account_name }}`<br/>`{{ uint32_t }}` |