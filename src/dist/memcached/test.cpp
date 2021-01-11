//
// Created by iclab on 1/7/21.
//

#include <iostream>
#include <bitset>

using namespace std;

int main() {
    int count = 1;
    for (int i = 0; i < 1024; i++) {
        bitset<10> bs = i;
        if (bs.count() == 5) {
            cout << count++ << ": " << bs << endl;
        }
    }
}