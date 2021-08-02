#include <iostream>

using namespace std;

int main(){
    int a = 0x12345678;
    char *p = (char *)&a;
    if(*p == 0x78)
        cout << "小端序！\n";
    if(*p == 0x12)
        cout << "大端序! \n";
    return 0;
}