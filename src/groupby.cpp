#include "emp-sh2pc/emp-sh2pc.h"
#include <iostream>
#include <unistd.h>
#include "Pair.h"
using namespace emp;
using namespace std;

const int BITSIZE = 32;
const int LEN = 10;
const int SUMLEN = 4;


void test_sum(int party, string inputs[]) {
	Integer *a = new Integer[LEN];
	Integer *b = new Integer[LEN];
	Integer sums [SUMLEN];		// TODO: change to dynamic size
	string categories[] = {"0", "1", "2", "3"}; // TODO: change to dynamic input

	// Initialize the secure integers
	for(int i = 0; i < LEN; i++)
		a[i] = Integer(BITSIZE, stoi(inputs[i]), ALICE);

	for(int i = 0; i < LEN; i++)
		b[i] = Integer(BITSIZE, stoi(inputs[i]), BOB);

	// Initialize sums
	for(int i = 0; i < SUMLEN; i++) {
		sums[i] = Integer(BITSIZE, 0, PUBLIC);
	}

	// Calculate sums
	Integer zero(BITSIZE, 0);	// Default party is PUBLIC
	for (int i = 0; i < LEN; i++) {
		for (int j = 0; j < SUMLEN; j++) {
			Integer cat(BITSIZE, j);
			// if a[i] == j then result = b[i] else result = 0
			Bit eqcat = a[i].equal(cat);
			Integer result = zero.select(eqcat, b[i]);	
			
			sums[j] = sums[j] + result;
		}	
	}

	// Reveal sums
    for(int i = 0; i < SUMLEN; i++) {
        cout << "sum " << ": " << sums[i].reveal<int>() << endl;
    }
}


void test_average(int party, string inputs[]) {

	Integer *a = new Integer[LEN];
	Integer *b = new Integer[LEN];
	Pair averages [LEN];

	// Initialize the secure integers
	for(int i = 0; i < LEN; i++)
		a[i] = Integer(BITSIZE, stoi(inputs[i]), ALICE);

	for(int i = 0; i < LEN; i++)
		b[i] = Integer(BITSIZE, stoi(inputs[i]), BOB);
	/*
	// Initialize averages
	for(int i = 0; i < sumlen; i++) {
		averages[i] = Pair(PUBLIC, BITSIZE, categories[], 0);
	}

	// Calculate averages

	// reveal sums
    for(int i = 0; i < sumlen; i++) {
        cout << "sum " << averages[i].get_id().reveal<int>() << ": " << averages[i].get_val().reveal<int>() << endl;
    }*/
}


int main(int argc, char **argv) {
	if (argc != 5 && argc != 6) {
		cout << "Usage for Alice (server): <program> 1 <port> <aggregation> <input file>" << endl;
		cout << "Usage for Bob (client): <program> 2 <port> <ip> <aggregation> <input file>" << endl;
		return 0;
	}
    
    int party, port;
	parse_party_and_port(argv, &party, &port);
	// Parse the IP address if Bob (client), otherwise set to nullptr since Alice (server) doesn't need it
	char * ip = nullptr;
	if(party == BOB) ip = argv[3];
	char* aggregation = argv[argc - 2];
	char* filename = argv[argc - 1];		// number is the last argument
	
	NetIO * io = new NetIO(ip, port);
	setup_semi_honest(io, party);
	
	ifstream infile(filename);
	if (!infile.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
		delete io;
		return 1;
    }
	
	string inputs[LEN];
	for(int i = 0; i < LEN; i++) {
		getline(infile, inputs[i]);
	}
	infile.close();

	switch (aggregation[0]) {
		case 's':
			test_sum(party, inputs);
			break;
		case 'a':
			test_average(party, inputs);
			break;
		case 'm':
			cout << "Mode" << endl;
			break;
		case 'v':
			cout << "Variance" << endl;
			break;
		case 'd':
			cout << "Standard Deviation" << endl;
			break;
		case 'c':
			cout << "Count" << endl;
			break;
		default:
			cout << "Invalid aggregation type" << endl;
			break;
	}
	
	delete io;
    return 0;
}
