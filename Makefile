#eosio-cpp...
all: boidtoken

.PHONY: boidtoken
boidtoken:
	mkdir -p build
	eosio-cpp -I /usr/local/eosio.cdt/include/ boidtoken.cpp -o build/boidtoken.wasm
	eosio-wasm2wast build/boidtoken.wasm -o build/boidtoken.wast
	eosio-abigen boidtoken.cpp -output=build/boidtoken.abi

.PHONY: test
test: boidtoken
	mkdir -p build_testboidpower
	eosio-cpp -I /usr/local/eosio.cdt/include/ testing/testboidpower.cpp -o build_testboidpower/testboidpower.wasm
	eosio-wasm2wast build_testboidpower/testboidpower.wasm -o build_testboidpower/testboidpower.wast
	eosio-abigen testing/testboidpower.cpp -output=build_testboidpower/testboidpower.abi

.PHONY: clean
clean:
	rm -rf build build_testboidpower
