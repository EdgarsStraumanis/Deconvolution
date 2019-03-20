#include <iostream>
#include <fstream>
#include <math.h>
#include <windows.h>

// V1.1.6

using namespace std;

struct oneExp{
    double tauZero = 1;
    double aZero = 0;
    double yZeroOne = 0;
    double yZeroTwo = 0;
    double yZero = (yZeroTwo - yZeroOne)/2;
};

struct twoExp{
    double tauZero = 0;
    double aZero = 0;
    double tauOne = 0;
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
    double accuracy = 0.0000001;

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
    /*
    void firstExpFit(){
        findOneFittinExp();
        twoExpFit->aZero = oneExpFit->aZero;
        twoExpFit->tauZero = oneExpFit->tauZero;
        twoExpFit->yZeroTwo = oneExpFit->yZero;

        twoExpFit->tauOne = oneExpFit->tauZero;
        //twoExpFit->aOne = oneExpFit->yZero;
        twoExpFit->yZero = 0; //(twoExpFit->yZeroTwo + twoExpFit->yZeroOne)/2;
    }
    //
    double findSectorTau21(){
        double sectorFirst = 0;
        double sectorSecond = last->time;
        double gap = sectorSecond - sectorFirst;
        for(;countGoodness(sectorFirst + 0.5 * gap, twoExpFit->aZero, twoExpFit->tauOne, twoExpFit->aOne, twoExpFit->yZero) > countGoodness(sectorSecond + 0.5 * gap, twoExpFit->aZero, twoExpFit->tauOne, twoExpFit->aOne, twoExpFit->yZero);){
            sectorFirst = sectorSecond;
            sectorSecond = sectorFirst + gap;
        }
        for(; sectorSecond - sectorFirst > accuracy  ;){
            if (countGoodness(sectorFirst + 0.5 * gap, twoExpFit->aZero, twoExpFit->tauOne, twoExpFit->aOne, twoExpFit->yZero) < countGoodness(sectorSecond + 0.5 * gap, twoExpFit->aZero, twoExpFit->tauOne, twoExpFit->aOne, twoExpFit->yZero)){
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
    //
    double findSectorTau22(){
        double sectorFirst = 0;
        double sectorSecond = last->time;
        double gap = sectorSecond - sectorFirst;
        for(;countGoodness(twoExpFit->tauZero, twoExpFit->aZero, sectorFirst + 0.5 * gap, twoExpFit->aOne, twoExpFit->yZero) > countGoodness(twoExpFit->tauZero, twoExpFit->aZero, sectorSecond + 0.5 * gap, twoExpFit->aOne, twoExpFit->yZero);){
            sectorFirst = sectorSecond;
            sectorSecond = sectorFirst + gap;
        }
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
    //
    double findSectorIntensity21(){
        double sectorFirst = 0;
        double sectorSecond = first->intensity;
        double gap = sectorSecond - sectorFirst;
        for(; sectorSecond - sectorFirst > accuracy  ;){
            if (countGoodness(twoExpFit->tauZero, sectorFirst + 0.5 * gap, twoExpFit->tauOne, twoExpFit->aOne, twoExpFit->yZero) < countGoodness(twoExpFit->tauZero, sectorSecond + 0.5 * gap, twoExpFit->tauOne, twoExpFit->aOne, twoExpFit->yZero))
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
    //
    double findSectorIntensity22(){
        double sectorFirst = 0;
        double sectorSecond = first->intensity;
        double gap = sectorSecond - sectorFirst;
        for(; sectorSecond - sectorFirst > accuracy  ;){
            if (countGoodness(twoExpFit->tauZero, twoExpFit->aZero, twoExpFit->tauOne, sectorFirst + 0.5 * gap , twoExpFit->yZero) < countGoodness(twoExpFit->tauZero, twoExpFit->aZero, twoExpFit->tauOne, sectorSecond + 0.5 * gap, twoExpFit->yZero))
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

    double findSectorZeroY2(){
        double gap = twoExpFit->yZeroTwo - twoExpFit->yZeroOne;
        if ( twoExpFit->yZeroTwo - twoExpFit->yZeroOne > accuracy ){
            if (countGoodness( twoExpFit->tauZero, twoExpFit->aZero,  twoExpFit->tauOne, twoExpFit->aOne, twoExpFit->yZeroOne + 0.25 * gap) < countGoodness(twoExpFit->tauZero, twoExpFit->aZero,  twoExpFit->tauOne, twoExpFit->aOne, twoExpFit->yZeroOne + 0.75 * gap))
                {
                    twoExpFit->yZeroTwo = twoExpFit->yZeroOne + gap/2;
                }
            else
                {
                    twoExpFit->yZeroOne = twoExpFit->yZeroOne + gap/2;
                }
        }
        cout << " Tau0 - " << twoExpFit->tauZero << " A0 - " << twoExpFit->aZero << " Tau1 - " << twoExpFit->tauOne << " A1 - " << twoExpFit->aOne << " Y1 - " << twoExpFit->yZero << " Goodness - " << (countGoodness(twoExpFit->tauZero, twoExpFit->aZero, twoExpFit->tauOne, twoExpFit->aOne, twoExpFit->yZero) / (dataPointCount-5)) << endl;
        return ((twoExpFit->yZeroTwo + twoExpFit->yZeroOne)/2);
    }
    //
    void findTwoFittingExp(){
        firstExpFit();

        //twoExpFit->tauOne = findSectorTau22();
        //twoExpFit->aOne = findSectorIntensity22();
        //twoExpFit->tauZero = findSectorTau21();
        //twoExpFit->aZero = findSectorIntensity21();

        for(int i = 0; i < 500; i++){
            if ( twoExpFit->yZeroTwo - twoExpFit->yZeroOne <= accuracy ) break;
                twoExpFit->tauZero = findSectorTau21();
                twoExpFit->aZero = findSectorIntensity21();
                twoExpFit->tauOne = findSectorTau22();
                twoExpFit->aOne = findSectorIntensity22();
                //twoExpFit->yZero = findSectorZeroY2();
                cout << " Tau0 - " << twoExpFit->tauZero << " A0 - " << twoExpFit->aZero << " Tau1 - " << twoExpFit->tauOne << " A1 - " << twoExpFit->aOne << " Y1 - " << twoExpFit->yZero << " Goodness - " << (countGoodness(twoExpFit->tauZero, twoExpFit->aZero, twoExpFit->tauOne, twoExpFit->aOne, twoExpFit->yZero) / (dataPointCount-5)) << endl;
        }
    }
    */
    void findTwoFittingExp(){



        double best[5] = {0,0,0,0,0};
        int segments;
        double bestGoodness = -1;
        double rangeA0Start = 0;
        double rangeA0End = first->intensity*2;
        double rangeA1Start = 0;
        double rangeA1End = first->intensity*2;
        double rangeTau0Start = 0;
        double rangeTau0End = 0.05*2;
        double rangeTau1Start = 0;
        double rangeTau1End = 0.05*2;
        double rangeY0Start = 0;
        double rangeY0End = first->intensity;
        double gapA0, gapA1, gapTau0, gapTau1, gapY0;
        for(int i = 0;i<3;i++){
        cout << "Iteration - " << i << endl;
        segments = 3;
        for(;segments<=8;segments++){
            rangeA0Start = 0;
            rangeA0End = first->intensity*2;
            rangeA1Start = 0;
            rangeA1End = first->intensity*2;
            rangeTau0Start = 0;
            rangeTau0End = 0.05*2;
            rangeTau1Start = 0;
            rangeTau1End = 0.05*2;
            rangeY0Start = 0;
            rangeY0End = first->intensity;
            for(int ii=0;ii<3;ii++)
            for(int iterations=0; iterations < 50/segments; iterations++){
                gapA0 = (rangeA0End - rangeA0Start) / segments;
                gapTau0 = (rangeTau0End - rangeTau0Start) / segments;
                gapA1 = (rangeA1End - rangeA1Start) / segments;
                gapTau1 = (rangeTau1End - rangeTau1Start) / segments;
                gapY0 = (rangeY0End - rangeY0Start) / segments;
                for(int segmentA0 = 1; segmentA0 <= segments; segmentA0++){
                    for(int segmentTau0 = 1; segmentTau0 <= segments; segmentTau0++){
                        for(int segmentA1 = 1; segmentA1 <= segments; segmentA1++){
                            for(int segmentTau1 = 1; segmentTau1 <= segments; segmentTau1++){
                                for(int segmentY0 = 1; segmentY0 <= segments; segmentY0++){
                                    if (rangeA0Start+gapA0*segmentA0 < rangeA1Start+gapA1*segmentA1) continue;

                                    double currentGoodness = countGoodness(rangeTau0Start+gapTau0*segmentTau0, rangeA0Start+gapA0*segmentA0,rangeTau1Start+gapTau1*segmentTau1,rangeA1Start+gapA1*segmentA1,rangeY0Start+gapY0*(segmentY0-1));
                                    //cout << currentGoodness << endl;
                                    if ((currentGoodness < bestGoodness) || (bestGoodness == -1)){
                                            bestGoodness = currentGoodness;
                                            best[0] = rangeTau0Start+gapTau0*segmentTau0;
                                            best[1] = rangeA0Start+gapA0*segmentA0;
                                            best[2] = rangeTau1Start+gapTau1*segmentTau1;
                                            best[3] = rangeA1Start+gapA1*segmentA1;
                                            best[4] = rangeY0Start+gapY0*(segmentY0-1);
                                    }
                                }
                            }
                        }
                    }
                }
                rangeA0Start = best[1]-gapA0;
                rangeA0End = best[1]+gapA0;
                rangeA1Start = best[3]-gapA1;
                rangeA1End =  best[3]+gapA1;
                rangeTau0Start = best[0]-gapTau0;
                rangeTau0End = best[0]+gapTau1;
                rangeTau1Start = best[2]-gapTau1;
                rangeTau1End = best[2]+gapTau1;
                rangeY0Start = 0;
                rangeY0End = best[4]+gapY0;
                //cout << "Finding - " << segments << " " << best[0] << " " << best[1] << " " << best[2] << " " << best[3] << " " << best[4] << " " << bestGoodness/dataPointCount << endl;
            }
        cout << "Segments - " << segments << " " << best[0] << " " << best[1] << " " << best[2] << " " << best[3] << " " << best[4] << " " << bestGoodness << endl;
        }
    }
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
        - Fitting 2 exponents to data [10]
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
                //cout << " Tau0 - " << testList->twoExpFit->tauZero << " A0 - " << testList->twoExpFit->aZero << " Tau1 - " << testList->twoExpFit->tauOne << " A1 - " << testList->twoExpFit->aOne << " Y1 - " << testList->twoExpFit->yZero << " Goodness - " << (testList->countGoodness(testList->twoExpFit->tauZero, testList->twoExpFit->aZero, testList->twoExpFit->tauOne, testList->twoExpFit->aOne, testList->twoExpFit->yZero) / testList->dataPointCount) << endl;
                //testOneExp->setupData(testList);
                //cout << "Exponents fitted to graph - " << testOneExp->aZero << endl;
                break;
            }
        }
    }
    return 0;
}

