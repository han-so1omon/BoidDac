#eosio-cpp...
all: boidtoken

.PHONY: boidtoken
boidtoken:
	mkdir -p build/contracts/boidtoken
	eosio-cpp -I /usr/local/eosio.cdt/include/ boidtoken.cpp -o build/contracts/boidtoken/boidtoken.wasm
	eosio-wasm2wast build/contracts/boidtoken/boidtoken.wasm -o build/contracts/boidtoken/boidtoken.wast
	eosio-abigen boidtoken.cpp -output=build/contracts/boidtoken/boidtoken.abi

.PHONY: test
test: boidtoken
	mkdir -p build/contracts/testboidpower
	eosio-cpp -I /usr/local/eosio.cdt/include/ testing/testboidpower.cpp -o build/contracts/testboidpower/testboidpower.wasm
	eosio-wasm2wast build/contracts/testboidpower/testboidpower.wasm -o build/contracts/testboidpower/testboidpower.wast
	eosio-abigen testing/testboidpower.cpp -output=build/contracts/testboidpower/testboidpower.abi

.PHONY: clean
clean:
	rm -rf build
