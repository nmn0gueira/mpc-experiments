#include "emp-sh2pc/emp-sh2pc.h"
#include <iostream>
#include <unistd.h>
using namespace emp;
using namespace std;

const int INPUT_LEN = 10;

void test_innerprod(int party, string inputs[]) {

	Integer sum(32, 0, PUBLIC);
	Integer prod(32, 0, PUBLIC);
	Integer *a = new Integer[INPUT_LEN];
	Integer *b = new Integer[INPUT_LEN];
	for(int i = 0; i < INPUT_LEN; i++)
		a[i] = Integer(32, stoi(inputs[i]), ALICE);

	for(int i = 0; i < INPUT_LEN; i++)
		b[i] = Integer(32, stoi(inputs[i]), BOB);

	for(int i = 0; i < INPUT_LEN; i++) {
		prod = a[i] * b[i];
		sum = sum + prod;
	}

	cout << "SUM: " << sum.reveal<int>() << endl;
	delete[] a;
	delete[] b;
}

int main(int argc, char **argv) {
	if (argc != 4 && argc != 5) {
		cout << "Usage for Alice (server): <program> <party> <port> <input file>" << endl;
		cout << "Usage for Bob (client): <program> <party> <port> <ip> <input file>" << endl;
		return 0;
	}
    
    int party, port;
	parse_party_and_port(argv, &party, &port);

	// Parse the IP address if Bob (client), otherwise set to nullptr since Alice (server) doesn't need it
	char * ip = nullptr;
	if(party == BOB) ip = argv[3];
	
	char* filename = argv[argc - 1];		// number is the last argument

	NetIO * io = new NetIO(ip, port);
	setup_semi_honest(io, party);
	
	ifstream infile(filename);
	
	string inputs[INPUT_LEN];
	string inputs_b[INPUT_LEN];

	if (!infile.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
		delete io;
		return 1;
    }
	
	for(int i = 0; i < INPUT_LEN; i++) {
		getline(infile, inputs[i]);
	}
	infile.close();
	
	test_innerprod(party, inputs);

	delete io;
    return 0;
}
