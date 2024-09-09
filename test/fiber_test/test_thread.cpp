#include "fiber/util.h"
#include <fiber/thread.h>
#include <iostream>
#include <vector>
void func1() {
    std::cout << "name: " << ::Thread::GetThis()->GetName() << ",id: " << ::GetThreadId()
              << std::endl;
}

void func2() {
    std::cout << "name: " << ::Thread::GetName() << ",id: " << ::GetThreadId()
              << std::endl;
}

int main(int argc, char** argv) {
    std::vector<::Thread::ptr> tpool;
    for (int i = 0; i < 5; i++) {
        // std::cout<<"haha";
        ::Thread::ptr t(new ::Thread(&func1, "name_" + std::to_string(i)));
        tpool.push_back(t);
    }

    for (int i = 0; i < 5; i++) {
        tpool[i]->join();
    }

    std::cout << "-----thread_test end-----" << std::endl;
}
