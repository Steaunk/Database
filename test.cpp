#include<cstring>
#include<cstdio>
#include<iostream>
#include<fstream>
using namespace std;
int main(){
    ifstream file("test.cpp");
    int x;
    string s;
    while(getline(file,s)){
        cout << s << endl;
    }
    return 0;
}