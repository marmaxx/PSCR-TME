#include <iostream>
int main() {
    std::cout << "Hello World!" << std::endl;
    int* tab = new int[10];
    for (int i = 0; i < 10; ++i) {
        tab[i] = i;
        std::cout << tab[i] << " ";
    }
    std::cout << std::endl;
    
    /* il faut mettre ssize_t au lieu de size_t car sinon après i=0, i devient 18446744073709551615 */
    for (ssize_t i=9; i >= 0 ; i--) {
        if (tab[i] - tab[i-1] != 1) {
            using std::cout;
            cout << "probleme !";
        }
    }
    return 0;
}