#include "emp-sh2pc/emp-sh2pc.h"
using namespace emp;

template<typename IO>
int main(int argc, char **argv) {
    
    // Initialize a party
    int party = atoi(argv[1]);

    if (party == ALICE) {
        std::cout << "Alice (garbler)" << std::endl;
    } else if (party == BOB) {
        std::cout << "Bob (evaluator)" << std::endl;
    }
    else {
        std::cerr << "Invalid party number, please use 1 for Alice (garbler) and 2 for Bob (evaluator)" << std::endl;
        return 1;
    }

    NetIO *io = new NetIO("127.0.0.1", 12345);

    // Initialize the secure computation
    SemiHonestParty<IO>* shp = emp::setup_semi_honest(io, party);

    SemiHonestGen<IO>* gen = shp->prot_exec;


    delete io;
    return 0;
}
