#include <iostream>

using namespace std;

struct dataTable{
double time;
int intensity;
dataTable* next = 0;
dataTable* previous = 0;
};

int main()
{
    dataTable* firstLaser = new dataTable;
    dataTable pointerLaser = *firstLaser;
    pointerLaser.time = 1;
    pointerLaser.intensity = 2;
    cout << pointerLaser.time << endl;
    return 0;
}
