#include <iostream>

using namespace std;

struct dataPair{
    double time;
    int intensity;
    dataPair* next = 0;
    dataPair* previous = 0;
};

struct linkedList{
    dataPair* first = 0;
    dataPair* last = 0;
    int dataPointCount = 0;

    void addPair(double makeTime, int makeIntensity){
        dataPair* creatingPair = new dataPair;
        creatingPair->time = makeTime;
        creatingPair->intensity = makeIntensity;
        if (dataPointCount == 0){
            first = creatingPair;
        }
        else
        {
            last->next = creatingPair;
        }
        dataPointCount++;
        last = creatingPair;
    }

    void clearList(){
        dataPair* deletePair = first;
        while (deletePair != 0){
            dataPair* saveNext = deletePair->next;
            first = saveNext;
            delete deletePair;
            deletePair = saveNext;
            dataPointCount--;
        }
        first = 0;
        last = 0;
    }
};

int main()
{
    /*
    dataPair* firstLaser = new dataPair;
    firstLaser->time = 1;
    firstLaser->intensity = 2;
    cout << firstLaser->time << endl;
    delete firstLaser;
    if (firstLaser) cout << firstLaser->time << endl; else cout << "doesn't exist" << endl;
    */
    linkedList* testList = new linkedList;
    testList->addPair(1,2);
    testList->addPair(3,4);
    cout << testList->dataPointCount << endl;
    testList->clearList();
    cout << testList->dataPointCount << endl;
    return 0;
}
