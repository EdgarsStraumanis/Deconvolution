#include <iostream>
#include <fstream>
#include <math.h>
#include <windows.h>

// V1.1.2

using namespace std;

// node to save data which knows next and previous nodes connected in the linked list to travel through nodes
struct dataPair{
    double time;
    int intensity;
    dataPair* next = 0;
    dataPair* previous = 0;
};

// allows to hold a list of data-points - time, intensity; Contains a various amount of functions to work with the data
struct linkedList{
    dataPair* first = 0;
    dataPair* last = 0;
    int dataPointCount = 0;

    // Allows to add data-points to list as last element
    void addPair(double makeTime, int makeIntensity){
        dataPair* creatingPair = new dataPair;
        creatingPair->time = makeTime;
        creatingPair->intensity = makeIntensity;
        if (dataPointCount == 0){
            first = creatingPair;
        }
        else{
            creatingPair->previous = last;
            last->next = creatingPair;
        }
        dataPointCount++;
        last = creatingPair;
    }

    // Clears read data-pairs from first to last
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

    // Find time-point with maximum intensity
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

    //Removes all data-pairs from beginning till specified time but not including time that was written
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

    // Deducts all time values by first data-pair's time to move time to zero
    void moveTimeZero(){
        if (dataPointCount < 1) return;
        dataPair* pointerPair = last;
        while (pointerPair != 0){
            pointerPair->time -= first->time;
            pointerPair = pointerPair->previous;
        }
    }

    // Counts how good is the fitting of specific exponent to data that was input
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

    // Reads data from file and sends it to other function to add to list
    void readFile(char fileName[255]){
        ifstream inputFile(fileName);
        while (true){
                double timeFromFile;
                int intensityFromFile;
                inputFile >> timeFromFile >> intensityFromFile;
                addPair(timeFromFile,intensityFromFile);
                if( inputFile.eof() ) break;
        }
    }

    // write to file a pair of data < time, difference between read data and fitted function at time moment>
    void drawGoodnessToFile(char fileName[255], double zeroY, double tau, double zeroA){
        ofstream outputFile(fileName);
        dataPair* pointerPair = first;
        while (pointerPair != 0){
            outputFile << pointerPair->time << " " << (pointerPair->intensity - (zeroA * exp(-(pointerPair->time / tau)) + zeroY) ) << endl;
            pointerPair = pointerPair->next;
        }
    }

    // write to file a pair of data < time, fitted function at time moment >
    void drawExpToFile(char fileName[255], double zeroY, double tau, double zeroA){
        ofstream outputFile(fileName);
        dataPair* pointerPair = first;
        while (pointerPair != 0){
            outputFile << pointerPair->time << " " << (zeroA * exp(-(pointerPair->time / tau)) + zeroY) << endl;
            pointerPair = pointerPair->next;
        }
    }

    // Tries to find sector for Tau for exp using golden cut method by reducing a range by halving the region whichever fits better
    double findSectorTau(double accuracy, double second, double baseIntensity, double baseZeroY){
        double sectorFirst = 0;
        double sectorSecond = last->time;
        double gap = sectorSecond - sectorFirst;
        for(;countGoodness(sectorFirst + 0.5 * gap, baseIntensity, baseZeroY) > countGoodness(sectorSecond + 0.5 * gap, baseIntensity, baseZeroY);){
            //cout << countGoodness(sectorFirst + 0.5 * gap, baseIntensity, baseZeroY) << " " << countGoodness(sectorSecond + 0.5 * gap, baseIntensity, baseZeroY) << endl;
            sectorFirst = sectorSecond;
            sectorSecond = sectorFirst + gap;
        }
        //cout << countGoodness(sectorFirst + 0.5 * gap, baseIntensity) << " " << countGoodness(sectorSecond + 0.5 * gap, baseIntensity) << endl;
        for(; sectorSecond - sectorFirst > accuracy  ;){
            if (countGoodness(sectorFirst + 0.5 * gap, baseIntensity, baseZeroY) < countGoodness(sectorSecond + 0.5 * gap, baseIntensity, baseZeroY)){
                gap /= 2;
                sectorSecond = sectorFirst + gap;
            }
            else{
                gap /= 2;
                sectorFirst = sectorSecond;
                sectorSecond = sectorFirst + gap;
            }
        }
        return ((sectorSecond + sectorFirst)/2);
    }

    // Tries to find sector for Intensity0 for exp using golden cut method by reducing a range by halving the region whichever fits better
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
            else{
                gap /= 2;
                sectorFirst = sectorSecond;
                sectorSecond = sectorFirst + gap;
            }
        }
        return ((sectorSecond + sectorFirst)/2);
    }

    // Tries to find sector for Y0 for exp using golden cut method by reducing a range by halving the region whichever fits better
    double findSectorZeroY(double accuracy, double baseIntensity, double baseTau, double &sectorFirst, double &sectorSecond){
        double gap = sectorSecond - sectorFirst;
        if ( sectorSecond - sectorFirst > accuracy ){
            if (countGoodness(baseTau, baseIntensity, sectorFirst + 0.25 * gap) < countGoodness(baseTau, baseIntensity, sectorFirst + 0.75 * gap))
                {
                    sectorSecond = sectorFirst + gap/2;
                }
            else
                {
                    sectorFirst = sectorFirst + gap/2;
                }
        }
        return ((sectorSecond + sectorFirst)/2);
    }
};

// prompt to ask for file name
void inputFileName(char* name){
    cout << "Input file name";
    cin >> name;
}

int main()
{
    /*
    Application currently focuses on analyzing experimental data that is saved in 2 columns <time, intensity>
    The data can be read [1] by specified file name [2]
    Data can be changed
        - Clear till first maximum [3]
        - Change time scale so the time starts with 0 [4]
    Data can be analyzed
        - Fitting Exp to data [5]
    Data can be written to file
        - Exp fitted to data [6] from selected folder [8]
        - Difference from Exp and read data [7] from selected folder [9]
    Stop analyzing data [0]
    */
    cout << "End work [0], Assign reading file [1], Read file [2], Clear till first Maximum [3]," << endl;
    cout << "Move graph time to 0 [4], Fit exp [5], Assign exp output file [6]," << endl;
    cout << "Assign difference output file [7] Write exp to file [8] Write difference to file [9]" << endl;

    int command;
    linkedList* testList = new linkedList;
    char nameOfInputFile[255] = "dataTwo.txt";
    char nameOfOutputFileExp[255] = "resultExp.txt";
    char nameOfOutputFileNLLS[255] = "resultNLLS.txt"; //NLLS --- non-linear least squares method
    bool caseTrue = true;

    double tau = 1; //testList->last->time;
    double inten = 0;
    double yZeroOne = 0;
    double yZeroTwo = 0;
    double yZero = (yZeroTwo - yZeroOne)/2;
    double accuracy = 0.00001;

    while (caseTrue == true){
        cout << "Type function: ";
        cin >> command;
        switch (command) {
        case 0 :
            {
                cout << "Ending work..." << endl;
                caseTrue = false;
                break;
            }
        case 1 :
            {
                inputFileName(nameOfInputFile);
                cout << "File selected for reading" << endl;
                break;
            }
        case 2 :
            {
                testList->readFile(nameOfInputFile);
                cout << "File read" << endl;
                break;
            }
        case 3 :
            {
                testList->clearTillTime(testList->maxPeak());
                cout << "Data cleared till first maximum" << endl;
                break;
            }
        case 4 :
            {
                testList->moveTimeZero();
                cout << "Graph moved on time axis to 0" << endl;
                break;
            }
        case 5 :
            {
                inten = testList->first->intensity;
                yZeroTwo = testList->first->intensity;
                yZero = (yZeroTwo - yZeroOne)/2;
                // cout << "Tau=" << tau << " A0=" << inten << " Y0=" << yZero << " Goodness=" << (testList->countGoodness(tau, inten, yZero) / (testList->dataPointCount)) << endl;
                for(int i = 0; i < 1000; i++){
                    if ( yZeroTwo - yZeroOne <= accuracy ) break;
                    yZero = testList->findSectorZeroY(accuracy, inten, tau, yZeroOne, yZeroTwo);
                    tau = testList->findSectorTau(accuracy, tau, inten, yZero);
                    inten = testList->findSectorIntensity(accuracy, inten, tau, yZero);
                    // cout << i << " " << "Tau=" << tau << " A0=" << inten << " Y0=" << yZero << " Goodness=" << (testList->countGoodness(tau, inten, yZero) / (testList->dataPointCount)) << endl;
                }
                cout << "Exponent fitted to graph - " << "Tau=" << tau << " A0=" << inten << " Y0=" << yZero << " Goodness=" << (testList->countGoodness(tau, inten, yZero) / (testList->dataPointCount)) << endl;
                break;
            }
        case 6 :
            {
                inputFileName(nameOfOutputFileExp);
                cout << "File selected for Exp" << endl;
                break;
            }
        case 7 :
            {
                inputFileName(nameOfOutputFileNLLS);
                cout << "File selected for Difference" << endl;
                break;
            }
        case 8 :
            {
                testList->drawGoodnessToFile(nameOfOutputFileExp, yZero, tau, inten);
                cout << "Exp written to file - " << nameOfOutputFileExp << endl;
                break;
            }
        case 9 :
            {
                testList->drawExpToFile(nameOfOutputFileNLLS, yZero, tau, inten);
                cout << "Difference written to file - " << nameOfOutputFileNLLS << endl;
                break;
            }
        }
    }
    return 0;
}

/*
    dataPair* firstLaser = new dataPair;
    firstLaser->time = 1;
    firstLaser->intensity = 2;
    cout << firstLaser->time << endl;
    delete firstLaser;
    if (firstLaser) cout << firstLaser->time << endl; else cout << "doesn't exist" << endl;

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
