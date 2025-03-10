#include "emp-sh2pc/emp-sh2pc.h"
#include <iostream>
#include <unistd.h>
using namespace emp;
using namespace std;

const int LEN = 10;

void test_innerprod(int bitsize, string inputs_a[], string inputs_b[], int len) {

	Integer sum(bitsize, 0, PUBLIC);
	Integer prod(bitsize, 0, PUBLIC);
	Integer *a = new Integer[len];
	Integer *b = new Integer[len];
	for(int i = 0; i < len; i++)
		a[i] = Integer(bitsize, stoi(inputs_a[i]), ALICE);

	for(int i = 0; i < len; i++)
		b[i] = Integer(bitsize, stoi(inputs_b[i]), BOB);

	for(int i = 0; i < len; i++) {
		prod = a[i] * b[i];
		sum = sum + prod;
	}

	cout << "SUM: " << sum.reveal<int>() << endl;
	delete[] a;
	delete[] b;
}

int main(int argc, char **argv) {

	if (argc != 4) {
		cout << "Usage: ./innerprod <party> <port> <bitsize>" << endl;
		return 0;
	}
    
    int bitsize = atoi(argv[3]);
	int port, party;

	parse_party_and_port(argv, &party, &port);
	
	NetIO * io = new NetIO(party == ALICE ? nullptr : "127.0.0.1", port);
	
	setup_semi_honest(io, party);	

	char fname_a[40];
	char fname_b[40];
	
	// Program is executed from the build folder
	sprintf(fname_a, "../src/data/innerprod/%d.1.dat", bitsize);
	sprintf(fname_b, "../src/data/innerprod/%d.2.dat", bitsize);

	ifstream infile_a(fname_a);
	ifstream infile_b(fname_b);
	
	string inputs_a[LEN];
	string inputs_b[LEN];

	if (!infile_a.is_open()) {
        cerr << "Failed to open file: " << fname_a << endl;
    }
    if (!infile_b.is_open()) {
        cerr << "Failed to open file: " << fname_b << endl;
    }

	if(infile_a.is_open() && infile_b.is_open()) {
		for(int i = 0; i < LEN; i++) {
			getline(infile_a, inputs_a[i]);
			getline(infile_b, inputs_b[i]);
		}
		infile_a.close();
		infile_b.close();
	}

	test_innerprod(bitsize, inputs_a, inputs_b, LEN);

	delete io;

    return 0;
}
