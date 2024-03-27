#include "pch.h"
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <windows.h>
using namespace std;

string k = "\x83\x30";
stringstream stre(k);
int main() {
    stop_handle(stre.get());
}
int stop_handle(char c_) {
    int temp = 0;
    char c = c_;
    while (true) {
        if ((c << 7) == 1) {
            temp = (temp << 7) + (c & 0b01111111);
            stre.get(c);
        }
        else {
            temp = (temp << 7) + c;
            break;
        }
    }
    return temp;
}