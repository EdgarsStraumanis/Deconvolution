#include <iostream>
#include <fstream>
#include <math.h>
#include <windows.h>

// V1.1.4

using namespace std;

struct oneExp{
    double tauZero = 1;
    double aZero = 0;
    double yZeroOne = 0;
    double yZeroTwo = 0;
    double yZero = (yZeroTwo - yZeroOne)/2;
};

struct twoExp{
    double tauZero = 1;
    double aZero = 0;
    double tauOne = 1;
    double aOne = 0;
    double yZeroOne = 0;
    double yZeroTwo = 0;
    double yZero = (yZeroTwo - yZeroOne)/2;
};

struct threeExp{
    double tauZero = 1;
    double aZero = 0;
    double tauOne = 1;
    double aOne = 0;
    double tauTwo = 1;
    double aTwo = 0;
    double yZeroOne = 0;
    double yZeroTwo = 0;
    double yZero = (yZeroTwo - yZeroOne)/2;

};

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
    oneExp* oneExpFit = new oneExp;
    twoExp* twoExpFit = new twoExp;
    threeExp* threeExpFit = new threeExp;
    double accuracy = 0.00001;

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
    void drawGoodnessToFile1(char fileName[255]){
        ofstream outputFile(fileName);
        dataPair* pointerPair = first;
        while (pointerPair != 0){
            outputFile << pointerPair->time << " " << (pointerPair->intensity - (oneExpFit->aZero * exp(-(pointerPair->time / oneExpFit->tauZero)) + oneExpFit->yZero) ) << endl;
            pointerPair = pointerPair->next;
        }
    }

    // write to file a pair of data < time, fitted function at time moment >
    void drawExpToFile1(char fileName[255]){
        ofstream outputFile(fileName);
        dataPair* pointerPair = first;
        while (pointerPair != 0){
            outputFile << pointerPair->time << " " << (oneExpFit->aZero * exp(-(pointerPair->time / oneExpFit->tauZero)) + oneExpFit->yZero) << endl;
            pointerPair = pointerPair->next;
        }
    }

    // -----------One exponent fitting-------------
    // Tries to find sector for Tau for exp using golden cut method by reducing a range by halving the region whichever fits better
    double findSectorTau1(){
        double sectorFirst = 0;
        double sectorSecond = last->time;
        double gap = sectorSecond - sectorFirst;
        for(;countGoodness(sectorFirst + 0.5 * gap, oneExpFit->aZero, oneExpFit->yZero) > countGoodness(sectorSecond + 0.5 * gap, oneExpFit->aZero, oneExpFit->yZero);){
            //cout << countGoodness(sectorFirst + 0.5 * gap, baseIntensity, baseZeroY) << " " << countGoodness(sectorSecond + 0.5 * gap, baseIntensity, baseZeroY) << endl;
            sectorFirst = sectorSecond;
            sectorSecond = sectorFirst + gap;
        }
        //cout << countGoodness(sectorFirst + 0.5 * gap, baseIntensity) << " " << countGoodness(sectorSecond + 0.5 * gap, baseIntensity) << endl;
        for(; sectorSecond - sectorFirst > accuracy  ;){
            if (countGoodness(sectorFirst + 0.5 * gap, oneExpFit->aZero, oneExpFit->yZero) < countGoodness(sectorSecond + 0.5 * gap, oneExpFit->aZero, oneExpFit->yZero)){
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
    double findSectorIntensity1(){
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
            if (countGoodness(oneExpFit->tauZero, sectorFirst + 0.5 * gap, oneExpFit->yZero) < countGoodness(oneExpFit->tauZero, sectorSecond + 0.5 * gap, oneExpFit->yZero))
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
    double findSectorZeroY1(){
        double gap = oneExpFit->yZeroTwo - oneExpFit->yZeroOne;
        if ( oneExpFit->yZeroTwo - oneExpFit->yZeroOne > accuracy ){
            if (countGoodness(oneExpFit->tauZero, oneExpFit->aZero, oneExpFit->yZeroOne + 0.25 * gap) < countGoodness(oneExpFit->tauZero, oneExpFit->aZero, oneExpFit->yZeroOne + 0.75 * gap))
                {
                    oneExpFit->yZeroTwo = oneExpFit->yZeroOne + gap/2;
                }
            else
                {
                    oneExpFit->yZeroOne = oneExpFit->yZeroOne + gap/2;
                }
        }
        return ((oneExpFit->yZeroTwo + oneExpFit->yZeroOne)/2);
    }
    // Fit one exp
    void findOneFittinExp(){
        oneExpFit->aZero = first->intensity;
        oneExpFit->yZeroTwo = first->intensity;
        oneExpFit->yZero = (oneExpFit->yZeroTwo + oneExpFit->yZeroOne)/2;

        // First phase
        for(int i = 0; i < 1000; i++){
            if ( oneExpFit->yZeroTwo - oneExpFit->yZeroOne <= accuracy ) break;
            oneExpFit->yZero = findSectorZeroY1();
            oneExpFit->tauZero = findSectorTau1();
            oneExpFit->aZero = findSectorIntensity1();
        }
        cout << "Exponent fitted to graph - " << "Tau=" << oneExpFit->tauZero << " A0=" << oneExpFit->aZero << " Y0=" << oneExpFit->yZero << " Goodness=" << (countGoodness(oneExpFit->tauZero, oneExpFit->aZero, oneExpFit->yZero) / (dataPointCount)) << endl;
    }
    // -----------Two exponent fitting--------------
    double countGoodness(double tauZero, double aZero, double tauOne, double aOne, double yZero){
        double holderGood = 0;
        dataPair* pointerPair = first;
        while (pointerPair != 0){
            holderGood += pow( aZero * exp(-(pointerPair->time / tauZero)) + aOne * exp(-(pointerPair->time / tauOne)) + yZero - pointerPair->intensity , 2  );
            // cout << pointerPair->intensity << " " << zeroA * exp(-(pointerPair->time / tau)) << endl;
            pointerPair = pointerPair->next;
        }
        return holderGood;
    }
    void firstExpFit(){
        findOneFittinExp();
        twoExpFit->aZero = oneExpFit->aZero;
        twoExpFit->tauZero = oneExpFit->tauZero;
        twoExpFit->yZero = oneExpFit->yZero;
    }
    double findSectorTau22(){
        double sectorFirst = 0;
        double sectorSecond = last->time;
        double gap = sectorSecond - sectorFirst;
        for(;countGoodness(twoExpFit->tauZero, twoExpFit->aZero, sectorFirst + 0.5 * gap, twoExpFit->aOne, twoExpFit->yZero) > countGoodness(twoExpFit->tauZero, twoExpFit->aZero, sectorSecond + 0.5 * gap, twoExpFit->aOne, twoExpFit->yZero);){
            //cout << countGoodness(sectorFirst + 0.5 * gap, baseIntensity, baseZeroY) << " " << countGoodness(sectorSecond + 0.5 * gap, baseIntensity, baseZeroY) << endl;
            sectorFirst = sectorSecond;
            sectorSecond = sectorFirst + gap;
        }
        //cout << countGoodness(sectorFirst + 0.5 * gap, baseIntensity) << " " << countGoodness(sectorSecond + 0.5 * gap, baseIntensity) << endl;
        for(; sectorSecond - sectorFirst > accuracy  ;){
            if (countGoodness(twoExpFit->tauZero, twoExpFit->aZero, sectorFirst + 0.5 * gap, twoExpFit->aOne, twoExpFit->yZero) < countGoodness(twoExpFit->tauZero, twoExpFit->aZero, sectorSecond + 0.5 * gap, twoExpFit->aOne, twoExpFit->yZero)){
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
    void findTwoFittingExp(){
        firstExpFit();
        twoExpFit->tauOne = findSectorTau22();
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

    oneExp* testOneExp = new oneExp;
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
                testList->findOneFittinExp();
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
                testList->drawGoodnessToFile1(nameOfOutputFileExp);
                cout << "Exp written to file - " << nameOfOutputFileExp << endl;
                break;
            }
        case 9 :
            {
                testList->drawExpToFile1(nameOfOutputFileNLLS);
                cout << "Difference written to file - " << nameOfOutputFileNLLS << endl;
                break;
            }
        case 10 :
            {
                testList->findTwoFittingExp();
                cout << testList->twoExpFit->tauOne << endl;
                //testOneExp->setupData(testList);
                //cout << "Exponents fitted to graph - " << testOneExp->aZero << endl;
                break;
            }
        }
    }
    return 0;
}

