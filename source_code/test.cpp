#include <iostream>

using namespace std;
struct Node
{
    int value;
    Node* next;
};


int main() {

    Node a;
    Node b;
    a.value = 5;
    b.value = 6;
    b.next = NULL;
    a.next = &b;
    Node head = a;
    
    cout << "a";
    return 0;

}