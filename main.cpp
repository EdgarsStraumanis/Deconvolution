#include <iostream>
#include <fstream>
#include <math.h>
#include <windows.h>

// V1.0.4

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

    double countGoodness(double tau, double zeroA, double zeroY){
        double holderGood = 0;
        dataPair* pointerPair = first;
        while (pointerPair != 0){
            holderGood += pow( zeroA * exp(-(pointerPair->time / tau)) - pointerPair->intensity + zeroY, 2  );
            // cout << pointerPair->intensity << " " << zeroA * exp(-(pointerPair->time / tau)) << endl;
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

    double findSectorTau(double accuracy, double second, double baseIntensity, double baseZeroY){
        double sectorFirst = 0;
        double sectorSecond = last->time;
        double gap = sectorSecond - sectorFirst;

        for(;countGoodness(sectorFirst + 0.5 * gap, baseIntensity, baseZeroY) > countGoodness(sectorSecond + 0.5 * gap, baseIntensity, baseZeroY);){
                cout << countGoodness(sectorFirst + 0.5 * gap, baseIntensity, baseZeroY) << " " << countGoodness(sectorSecond + 0.5 * gap, baseIntensity, baseZeroY) << endl;
                sectorFirst = sectorSecond;
                sectorSecond = sectorFirst + gap;
        }

        //cout << countGoodness(sectorFirst + 0.5 * gap, baseIntensity) << " " << countGoodness(sectorSecond + 0.5 * gap, baseIntensity) << endl;
        for(; sectorSecond - sectorFirst > accuracy  ;){
            if (countGoodness(sectorFirst + 0.5 * gap, baseIntensity, baseZeroY) < countGoodness(sectorSecond + 0.5 * gap, baseIntensity, baseZeroY))
                {
                    gap /= 2;
                    sectorSecond = sectorFirst + gap;
                }
            else
                {
                    gap /= 2;
                    sectorFirst = sectorSecond;
                    sectorSecond = sectorFirst + gap;
                }
        }
        return ((sectorSecond + sectorFirst)/2);
    }

    double findSectorIntensity(double accuracy, double second, double baseTau, double baseZeroY){
        double sectorFirst = 0;
        double sectorSecond = first->intensity;
        double gap = sectorSecond - sectorFirst;
        /*
        for(;countGoodness(baseTau, sectorFirst + 0.5 * gap) > countGoodness(baseTau, sectorSecond + 0.5 * gap);)
        {
                    cout << countGoodness(baseTau, sectorFirst + 0.5 * gap) << " " << countGoodness(baseTau, sectorSecond + 0.5 * gap) << endl;
                    sectorFirst = sectorSecond;
                    sectorSecond = sectorFirst + gap;
                    Sleep(1);
        }
        */
        //cout << countGoodness(sectorFirst + 0.5 * gap, first->intensity) << " " << countGoodness(sectorSecond + 0.5 * gap, first->intensity) << endl;
        for(; sectorSecond - sectorFirst > accuracy  ;){
            if (countGoodness(baseTau, sectorFirst + 0.5 * gap, baseZeroY) < countGoodness(baseTau, sectorSecond + 0.5 * gap, baseZeroY))
                {
                    gap /= 2;
                    sectorSecond = sectorFirst + gap;
                }
            else
                {
                    gap /= 2;
                    sectorFirst = sectorSecond;
                    sectorSecond = sectorFirst + gap;
                }
        }
        return ((sectorSecond + sectorFirst)/2);
    }

    double findSectorZeroY(double accuracy, double baseIntensity, double baseTau, double baseZeroY){
        double sectorFirst = 0;
        double sectorSecond = last->intensity;
        double gap = sectorSecond - sectorFirst;
        /*
        for(;countGoodness(baseTau, sectorFirst + 0.5 * gap) > countGoodness(baseTau, sectorSecond + 0.5 * gap);)
        {
                    cout << countGoodness(baseTau, sectorFirst + 0.5 * gap) << " " << countGoodness(baseTau, sectorSecond + 0.5 * gap) << endl;
                    sectorFirst = sectorSecond;
                    sectorSecond = sectorFirst + gap;
                    Sleep(1);
        }
        */
        //cout << countGoodness(sectorFirst + 0.5 * gap, first->intensity) << " " << countGoodness(sectorSecond + 0.5 * gap, first->intensity) << endl;
        for(; sectorSecond - sectorFirst > accuracy  ;){
            if (countGoodness(baseTau, baseIntensity, sectorFirst + (1-(1/exp(1))) * gap) < countGoodness(baseTau, baseZeroY, sectorSecond + (1-(1/exp(1))) * gap))
                {
                    gap /= 2;
                    sectorSecond = sectorFirst + gap;
                }
            else
                {
                    gap /= 2;
                    sectorFirst = sectorSecond;
                    sectorSecond = sectorFirst + gap;
                }
        }
        return ((sectorSecond + sectorFirst)/2);
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
    cout << 1/exp(1) << endl;
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
    char nameOfFile[255] = "dataTwo.txt";
    inputFileName(nameOfFile);
    testList->readFile(nameOfFile);
    //cout << testList->dataPointCount << endl;
    //cout << testList->maxPeak() << endl;
    testList->clearTillTime(testList->maxPeak());
    // cout << testList->dataPointCount << endl;
    testList->moveTimeZero();
    //cout << testList->countGoodness(3,8) << endl;
    double tau = 1; //testList->last->time;
    double inten = testList->first->intensity;
    double yZero = 24.76;
    double accuracy = 0.1;

    cout << "Tau=" << tau << " A0=" << inten << " Y0=" << yZero << " Goodness=" << (testList->countGoodness(tau, inten, yZero) / (testList->dataPointCount - 3)) << endl;
    for(int i = 0; i < 25; i++){
    tau = testList->findSectorTau(accuracy, tau, inten, yZero);
    inten = testList->findSectorIntensity(accuracy, inten, tau, yZero);
    //yZero = testList->findSectorZeroY(accuracy, inten, tau, yZero);
    //tau = testList->findSectorTau(accuracy, tau, inten, yZero);

    accuracy/=2;
    cout << "Tau=" << tau << " A0=" << inten << " Y0=" << yZero << " Goodness=" << (testList->countGoodness(tau, inten, yZero) / (testList->dataPointCount - 3)) << endl;
    }
    return 0;
}
