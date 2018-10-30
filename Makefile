#eosio-cpp...
all: boidtoken

.PHONY: boidtoken
boidtoken:
	mkdir -p build
	eosio-cpp -I /usr/local/eosio.cdt/include/ boidtoken.cpp -o build/boidtoken.wasm
	eosio-wasm2wast boidtoken.wasm -o build/boidtoken.wast
	eosio-abigen boidtoken.cpp -output=build/boidtoken.abi

.PHONY: clean
clean:
	rm -rf build
