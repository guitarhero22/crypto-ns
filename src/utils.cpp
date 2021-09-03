#include "utils.h"

std::random_device rd;
std::mt19937 rnd_gen(rd());

int NUM_PEERS = 0;

void logerr(std::string str){
    std::cerr << str << std::endl;
    exit(1);
}

void log(std::string str){
    std::cout << str << std::endl;
    return;
}
