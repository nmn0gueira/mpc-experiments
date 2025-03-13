#include "emp-sh2pc/emp-sh2pc.h"
using namespace emp;
using namespace std;

void test_millionaire(int party, int number) {
	Integer a(32, number, ALICE);
	Integer b(32, number, BOB);
	Bit res = a > b;

	cout << "ALICE larger?\t"<< res.reveal<bool>()<<endl;
}

int main(int argc, char** argv) {
	if (argc != 4 && argc != 5) {
		cout << "Usage for Alice (server): ./millionaire <party> <port> <number>" << endl;
		cout << "Usage for Bob (client): ./millionaire <party> <port> <ip> <number>" << endl;
		return 0;
	}

	int party, port;
	parse_party_and_port(argv, &party, &port);

	// Parse the IP address if Bob (client), otherwise set to nullptr since Alice (server) doesn't need it
	char * ip = nullptr;
	if(party == BOB) ip = argv[3];
	
	int num = atoi(argv[argc - 1]);		// number is the last argument

	NetIO * io = new NetIO(ip, port);

	setup_semi_honest(io, party);
	test_millionaire(party, num);
//	test_sort(party);
	cout << CircuitExecution::circ_exec->num_and()<<endl;
	finalize_semi_honest();
	delete io;
	return 0;
}
