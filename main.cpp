#include <iostream>
#include <fstream>
#include <math.h>

// V1.0.3

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
            creatingPair->previous = last;
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

    double maxPeak(){
        if (dataPointCount < 1) return -1;
        dataPair* findPair = first;
        dataPair* maxPair = findPair;
        while (findPair != 0){
            if (findPair->intensity > maxPair->intensity) maxPair = findPair;
            findPair = findPair->next;
        }
        return maxPair->time;
    }

    void clearTillTime(double timeTill){
        dataPair* deletePair = first;
        while (deletePair !=0 && deletePair->time < timeTill){
            dataPair* saveNext = deletePair->next;
            first = saveNext;
            delete deletePair;
            deletePair = saveNext;
            dataPointCount--;
        }
    }

    void moveTimeZero(){
        if (dataPointCount < 1) return;
        dataPair* pointerPair = last;
        while (pointerPair != 0){
            pointerPair->time -= first->time;
            pointerPair = pointerPair->previous;
        }
    }

    double countGoodness(double tau, double zeroA){
        double holderGood = 0;
        dataPair* pointerPair = first;
        while (pointerPair != 0){
            holderGood += pow( zeroA * exp(-(pointerPair->time / tau)) - pointerPair->intensity , 2  );
            cout << pointerPair->intensity << " " << zeroA * exp(-(pointerPair->time / tau)) << endl;
            pointerPair = pointerPair->next;
        }
        return holderGood;
    }

    void readFile(char fileName[255]){
        ifstream inputFile(fileName);
        while (true) {
                double timeFromFile;
                int intensityFromFile;
                inputFile >> timeFromFile >> intensityFromFile;
                addPair(timeFromFile,intensityFromFile);
                if( inputFile.eof() ) break;
        }
    }
};

void inputFileName(char* name){
    bool input;
    cout << "Input new file [1] or use default [0]." << endl;
    cin >> input;
    if (input == false) return;
    cout << "Input file name" << endl;
    cin >> name;
}



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
    /*
    testList->addPair(0,0);
    testList->addPair(1,1);
    testList->addPair(2,3);
    testList->addPair(3,7);
    testList->addPair(4,8);
    testList->addPair(5,6);
    testList->addPair(6,4);
    testList->addPair(7,3);
    testList->addPair(8,2);
    testList->addPair(9,2);
    testList->addPair(10,1);
    */
    char nameOfFile[255] = "dataForTesting.txt";
    inputFileName(nameOfFile);
    testList->readFile(nameOfFile);
    cout << testList->dataPointCount << endl;
    cout << testList->maxPeak() << endl;
    testList->clearTillTime(testList->maxPeak());
    cout << testList->dataPointCount << endl;
    testList->moveTimeZero();
    cout << testList->countGoodness(3,8) << endl;
    return 0;
}
