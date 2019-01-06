# Action - `{{ setbpdiv }}`

This Contract is legally binding and can be used in the event of a dispute. Disputes shall be settled through the standard arbitration process established by EOS.IO.

### Description

The `{{ setbpdiv }}` action... 

INTENT: The intent of `{{ setbpdiv }}` is for the owner of the contract to set the boidpower divisor, a variable that effects the bonus a user recieves for providing enough boidpower to the network to surpass the boidpower bonus ratio. The boidpower bonus is calulated with: boidpower_bonus = (boidpower * staked_tokens) / boidpower_divisor.

TERM: This action lasts for the duration of the processing of the contract.

### Inputs and Input Types

The `{{ setbpdiv }}` action requires the following `inputs` and `input types`:

| Action | Input | Input Type |
|:--|:--|:--|
| `{{ setbpdiv }}` | `{{ bp_bonus_divisor }}` | `{{ float }}`|
