#include <iostream>
#include <fstream>
#include <math.h>
#include <windows.h>

#include <time.h>
// V1.3.1

using namespace std;

// To save data about 1 exponents fitted to one Experiment data function - tau0, A0, y0 (noise)
struct oneExp{
    double tauZero = 1;
    double aZero = 0;
    double yZeroOne = 0;
    double yZeroTwo = 0;
    double yZero = 0;
};

// To save data about 2 exponents fitted to one Experiment data function - tau0, A0, tau1, A1, y0 (noise)
struct twoExp{
    double tauZero = 0;
    double aZero = 0;
    double tauOne = 0;
    double aOne = 0;
    double yZero = 0;
};

// node to save data which knows next and previous nodes connected in the linked list to travel through nodes
struct dataNode{
    double time;
    int intensity;
    double deconvolutionSum = 0; // used to store deconvolution and interpolation for a moment
    dataNode* next = 0;
    dataNode* previous = 0;
};

// allows to hold a list of data-points - time, intensity; Contains a various amount of functions to work with the data
struct linkedList{
    dataNode* materialFirst = 0;
    dataNode* materialLast = 0;
    dataNode* instrumentFirst = 0;
    dataNode* instrumentLast = 0;
    int instrumentPointCount = 0;
    int dataPointCount = 0;
    int interpolationStepCount = 0;
    float interpolationStep = 0;
    oneExp* oneExpFit = new oneExp;
    twoExp* twoExpFit = new twoExp;
    double accuracy = 0.0000001;

    // Allows to add data-points to list as materialLast element
    void addNode(double makeTime, int makeIntensity){
        dataNode* creatingNode = new dataNode;
        creatingNode->time = makeTime;
        creatingNode->intensity = makeIntensity;
        if (dataPointCount == 0){ // only exception if there is no data yet
            materialFirst = creatingNode;
        }
        else{
            creatingNode->previous = materialLast;
            materialLast->next = creatingNode;
        }
        dataPointCount++;
        materialLast = creatingNode;
    }

    // Clears read data-Nodes from materialFirst to materialLast
    void clearList(){
        dataNode* deleteNode = materialFirst;
        while (deleteNode != 0){
            dataNode* saveNext = deleteNode->next;
            materialFirst = saveNext;
            delete deleteNode;
            deleteNode = saveNext;
            dataPointCount--;
        }
        materialFirst = 0;
        materialLast = 0;
        // resets the equation variables for error correctness
        oneExpFit->tauZero = 1;
        oneExpFit->aZero = 0;
        oneExpFit->yZeroOne = 0;
        oneExpFit->yZeroTwo = 0;
        oneExpFit->yZero = 0;
        twoExpFit->tauZero = 0;
        twoExpFit->aZero = 0;
        twoExpFit->tauOne = 0;
        twoExpFit->aOne = 0;
        twoExpFit->yZero = 0;
    }

    // Find time-point with maximum intensity
    double maxPeak(){
        if (dataPointCount < 1) return -1;
        dataNode* findNode = materialFirst;
        dataNode* maxNode = findNode;
        while (findNode != 0){
            if (findNode->intensity > maxNode->intensity) maxNode = findNode;
            findNode = findNode->next;
        }
        return maxNode->time;
    }

    // Removes all data-Nodes from beginning till specified time but not including time that was written
    int clearTillTime(double timeTill){
        if (dataPointCount < 2) return -1; // No real data to even delete
        dataNode* deleteNode = materialFirst;
        while (deleteNode !=0 && deleteNode->time < timeTill){
            dataNode* saveNext = deleteNode->next;
            materialFirst = saveNext;
            delete deleteNode;
            deleteNode = saveNext;
            dataPointCount--;
        }
    }

    // Deducts all time values by materialFirst data-Node's time to move time to zero
    int moveTimeZero(){
        if (dataPointCount < 1) return -1;
        dataNode* pointerNode = materialLast;
        while (pointerNode != 0){
            pointerNode->time -= materialFirst->time;
            pointerNode = pointerNode->previous;
        }
    }

    // Counts how good is the fitting of specific exponent to data that was input
    double countGoodness(double tau, double zeroA, double zeroY){
        double holderGood = 0;
        dataNode* pointerNode = materialFirst;
        while (pointerNode != 0){
            holderGood += pow( zeroA * exp(-(pointerNode->time / tau)) - pointerNode->intensity + zeroY, 2  ); // calculates square difference between experimental and newly calculated equation
            pointerNode = pointerNode->next;
        }
        return holderGood;
    }

    // Reads data from file and sends it to other function to add to list
    int readFile(char fileName[255]){
        clearList();
        ifstream inputFile(fileName);
        if (inputFile) // file may not be present
            while (true){
                    double timeFromFile;
                    int intensityFromFile;
                    inputFile >> timeFromFile >> intensityFromFile;
                    addNode(timeFromFile,intensityFromFile);
                    if( inputFile.eof() ) break; //reads all the data points that are in the file till the end
            }
        else
            return -1;
        if (dataPointCount < 1) return -2; // file may not have any data and will warn a user
        return 0;
    }

    // write to file a Node of data <time, difference between read data and fitted function at time moment>
    int drawGoodnessToFile1(char fileName[255]){
        if (dataPointCount < 2) return -1;
        if (oneExpFit->aZero == 0) return -2;
        ofstream outputFile(fileName);
        dataNode* pointerNode = materialFirst;
        while (pointerNode != 0){
            outputFile << pointerNode->time << " " << (pointerNode->intensity - (oneExpFit->aZero * exp(-(pointerNode->time / oneExpFit->tauZero)) + oneExpFit->yZero) ) << endl;
            pointerNode = pointerNode->next;
        }
        return 0;
    }

    // write to file a Node of data <time, fitted function at time moment >
    int drawExpToFile1(char fileName[255]){
        if (dataPointCount < 2) return -1;
        if (oneExpFit->aZero == 0) return -2;
        ofstream outputFile(fileName);
        dataNode* pointerNode = materialFirst;
        while (pointerNode != 0){
            outputFile << pointerNode->time << " " << (oneExpFit->aZero * exp(-(pointerNode->time / oneExpFit->tauZero)) + oneExpFit->yZero) << endl;
            pointerNode = pointerNode->next;
        }
        return 0;
    }

    // write to file a Node of data <time, difference between read data and fitted function at time moment>
    int drawGoodnessToFile2(char fileName[255]){
        if (dataPointCount < 2) return -1;
        if (twoExpFit->aZero == 0) return -2;
        ofstream outputFile(fileName);
        dataNode* pointerNode = materialFirst;
        while (pointerNode != 0){
            outputFile << pointerNode->time << " " << (pointerNode->intensity - (twoExpFit->aZero * exp(-(pointerNode->time / twoExpFit->tauZero)) + twoExpFit->aOne * exp(-(pointerNode->time / twoExpFit->tauOne)) + twoExpFit->yZero) ) << endl;
            pointerNode = pointerNode->next;
        }
        return 0;
    }

    // write to file a Node of data <time, fitted function at time moment>
    int drawExpToFile2(char fileName[255]){
        if (dataPointCount < 2) return -1;
        if (oneExpFit->aZero == 0) return -2;
        ofstream outputFile(fileName);
        dataNode* pointerNode = materialFirst;
        while (pointerNode != 0){
            outputFile << pointerNode->time << " " << (twoExpFit->aZero * exp(-(pointerNode->time / twoExpFit->tauZero)) + twoExpFit->aOne * exp(-(pointerNode->time / twoExpFit->tauOne)) + twoExpFit->yZero) << endl;
            pointerNode = pointerNode->next;
        }
        return 0;
    }

    // -----------One exponent fitting------------- //
    // Tries to find sector for Tau for exp using golden cut method by reducing a range by halving the region whichever fits better
    double findSectorTau1(){
        double sectorFirst = 0;
        double sectorSecond = materialLast->time;
        double gap = sectorSecond - sectorFirst;
        for(;countGoodness(sectorFirst + 0.5 * gap, oneExpFit->aZero, oneExpFit->yZero) > countGoodness(sectorSecond + 0.5 * gap, oneExpFit->aZero, oneExpFit->yZero);){ // tau may be further in other region than it has been calculated
            sectorFirst = sectorSecond;
            sectorSecond = sectorFirst + gap;
        }
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
        double sectorSecond = materialFirst->intensity;
        double gap = sectorSecond - sectorFirst;
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
    int findOneFittinExp(){
        if (dataPointCount < 2) return -1;
        if (materialFirst->time != maxPeak()) return -2;
        if (materialFirst->time != 0) return -3;
        oneExpFit->aZero = materialFirst->intensity;
        oneExpFit->yZeroTwo = materialFirst->intensity;
        oneExpFit->yZero = (oneExpFit->yZeroTwo + oneExpFit->yZeroOne)/2;

        for(int i = 0; i < 1000; i++){ // has iterations till reaches accuracy or stops if it has reached limit
            if ( oneExpFit->yZeroTwo - oneExpFit->yZeroOne <= accuracy ) break;
            oneExpFit->yZero = findSectorZeroY1();
            oneExpFit->tauZero = findSectorTau1();
            oneExpFit->aZero = findSectorIntensity1();
        }
        cout << "Exponent fitted to graph - " << "Tau=" << oneExpFit->tauZero << " A0=" << oneExpFit->aZero << " Y0=" << oneExpFit->yZero << " Goodness=" << (countGoodness(oneExpFit->tauZero, oneExpFit->aZero, oneExpFit->yZero) / (dataPointCount)) << endl;
        return 0;
    }
    // -----------Two exponent fitting--------------
    // Counts how good is the fitting of specific exponent to data that was input
    double countGoodness(double tauZero, double aZero, double tauOne, double aOne, double yZero){
        double holderGood = 0;
        dataNode* pointerNode = materialFirst;
        while (pointerNode != 0){
            holderGood += pow( aZero * exp(-(pointerNode->time / tauZero)) + aOne * exp(-(pointerNode->time / tauOne)) + yZero - pointerNode->intensity , 2  );
            pointerNode = pointerNode->next;
        }
        return holderGood;
    }

    // Fit two exp with 5 arguments
    void findTwoFittingExpAll(){
        int segments;
        double bestGoodness = -1;
        double rangeA0Start = 0;
        double rangeA0End = materialFirst->intensity*2;
        double rangeA1Start = 0;
        double rangeA1End = materialFirst->intensity*2;
        double rangeTau0Start = 0;
        double rangeTau0End = 1;
        double rangeTau1Start = 0;
        double rangeTau1End = 1;
        double rangeY0Start = 0;
        double rangeY0End = materialFirst->intensity;
        double gapA0, gapA1, gapTau0, gapTau1, gapY0;
        for(int i = 0;i<4;i++){ // iterations
            segments = 3;
            for(;segments<=4;segments++){ // cutting with 3 and 4 segments
                rangeA0Start = 0;
                rangeA0End = materialFirst->intensity*2;
                rangeA1Start = 0;
                rangeA1End = materialFirst->intensity*2;
                rangeTau0Start = 0;
                rangeTau0End = 1;
                rangeTau1Start = 0;
                rangeTau1End = 1;
                rangeY0Start = 0;
                rangeY0End = materialFirst->intensity;
                for(int iterations=0; iterations < 75/segments; iterations++){
                    gapA0 = (rangeA0End - rangeA0Start) / segments;
                    gapTau0 = (rangeTau0End - rangeTau0Start) / segments;
                    gapA1 = (rangeA1End - rangeA1Start) / segments;
                    gapTau1 = (rangeTau1End - rangeTau1Start) / segments;
                    gapY0 = (rangeY0End - rangeY0Start) / segments;
                    for(int segmentTau0 = 1; segmentTau0 <= segments; segmentTau0++){
                        for(int segmentTau1 = 1; segmentTau1 <= segments; segmentTau1++){
                            for(int segmentA0 = 1; segmentA0 <= segments; segmentA0++){
                                for(int segmentA1 = 1; segmentA1 <= segments; segmentA1++){
                                    for(int segmentY0 = 1; segmentY0 <= segments; segmentY0++){
                                        if (rangeA0Start+gapA0*segmentA0 < rangeA1Start+gapA1*segmentA1) continue;
                                        double currentGoodness = countGoodness(rangeTau0Start+gapTau0*segmentTau0, rangeA0Start+gapA0*segmentA0,rangeTau1Start+gapTau1*segmentTau1,rangeA1Start+gapA1*segmentA1,rangeY0Start+gapY0*(segmentY0-1));
                                        if ((currentGoodness < bestGoodness) || (bestGoodness == -1)){
                                                bestGoodness = currentGoodness;
                                                twoExpFit->tauZero = rangeTau0Start+gapTau0*segmentTau0;
                                                twoExpFit->aZero = rangeA0Start+gapA0*segmentA0;
                                                twoExpFit->tauOne = rangeTau1Start+gapTau1*segmentTau1;
                                                twoExpFit->aOne = rangeA1Start+gapA1*segmentA1;
                                                twoExpFit->yZero = rangeY0Start+gapY0*(segmentY0-1);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    rangeA0Start = twoExpFit->aZero-gapA0; // saves best fitting exponents
                    rangeA0End = twoExpFit->aZero+gapA0;
                    rangeA1Start = twoExpFit->aOne-gapA1;
                    rangeA1End =  twoExpFit->aOne+gapA1;
                    rangeTau0Start = twoExpFit->tauZero-gapTau0;
                    rangeTau0End = twoExpFit->tauZero+gapTau0;
                    rangeTau1Start = twoExpFit->tauOne-gapTau1;
                    rangeTau1End = twoExpFit->tauOne+gapTau1;
                    rangeY0Start = 0;
                    rangeY0End = twoExpFit->yZero+gapY0;
            }
            //cout << "Segments - " << segments << " " << twoExpFit->tauZero << " " << twoExpFit->aZero << " " << twoExpFit->tauOne << " " << twoExpFit->aOne << " " << twoExpFit->yZero << " " << bestGoodness << endl;
            }
        }
    }

    // Fit two exp with 3 arguments (zero)
    void findTwoFittingExpZero(){
        int segments;
        double bestGoodness = -1;
        double rangeA0Start = 0;
        double rangeA0End = materialFirst->intensity*2;
        double rangeA1Start = 0;
        double rangeA1End = materialFirst->intensity*2;
        double rangeTau0Start = 0;
        double rangeTau0End = 1;
        double rangeTau1Start = 0;
        double rangeTau1End = 1;
        double rangeY0Start = 0;
        double rangeY0End = materialFirst->intensity;
        double gapA0, gapTau0, gapY0;
        for(int i = 0;i<1;i++){
            //cout << "Iteration - " << i << endl;
            segments = 3;
            for(;segments<=7;segments++){
                rangeA0Start = 0;
                rangeA0End = twoExpFit->aZero*2;
                rangeTau0Start = 0;
                rangeTau0End = twoExpFit->tauZero*2;
                rangeY0Start = 0;
                rangeY0End = twoExpFit->yZero*2;
                for(int iterations=0; iterations < 50/segments; iterations++){
                    gapA0 = (rangeA0End - rangeA0Start) / segments;
                    gapTau0 = (rangeTau0End - rangeTau0Start) / segments;
                    gapY0 = (rangeY0End - rangeY0Start) / segments;
                    for(int segmentTau0 = 1; segmentTau0 <= segments; segmentTau0++){
                        for(int segmentA0 = 1; segmentA0 <= segments; segmentA0++){
                            for(int segmentY0 = 1; segmentY0 <= segments; segmentY0++){
                                double currentGoodness = countGoodness(rangeTau0Start+gapTau0*segmentTau0, rangeA0Start+gapA0*segmentA0,twoExpFit->tauOne,twoExpFit->aOne,rangeY0Start+gapY0*(segmentY0-1));
                                //cout << currentGoodness << endl;
                                if ((currentGoodness < bestGoodness) || (bestGoodness == -1)){
                                    bestGoodness = currentGoodness;
                                    twoExpFit->tauZero = rangeTau0Start+gapTau0*segmentTau0;
                                    twoExpFit->aZero = rangeA0Start+gapA0*segmentA0;
                                    twoExpFit->yZero = rangeY0Start+gapY0*(segmentY0-1);
                                }
                            }
                        }
                    }
                    rangeA0Start = twoExpFit->aZero-gapA0;
                    rangeA0End = twoExpFit->aZero+gapA0;
                    rangeTau0Start = twoExpFit->tauZero-gapTau0;
                    rangeTau0End = twoExpFit->tauZero+gapTau0;
                    rangeY0Start = twoExpFit->yZero-gapY0;
                    rangeY0End = twoExpFit->yZero+gapY0;
                }
            //cout << "Segments - " << segments << " " << twoExpFit->tauZero << " " << twoExpFit->aZero << " " << twoExpFit->tauOne << " " << twoExpFit->aOne << " " << twoExpFit->yZero << " " << bestGoodness << endl;
            }
        }
    }

    // Fit two exp with 3 arguments (one)
    void findTwoFittingExpOne(){
        int segments;
        double bestGoodness = -1;
        double rangeA1Start;
        double rangeA1End;
        double rangeTau1Start;
        double rangeTau1End;
        double rangeY0Start;
        double rangeY0End;
        double gapA1, gapTau1, gapY0;
        for(int i = 0;i<1;i++){
            segments = 3;
            for(;segments<=9;segments++){
                rangeA1Start = 0;
                rangeA1End = twoExpFit->aOne*2;
                rangeTau1Start = 0;
                rangeTau1End = twoExpFit->tauOne*2;
                rangeY0Start = 0;
                rangeY0End = twoExpFit->yZero*2;
                for(int iterations=0; iterations < 50/segments; iterations++){
                    gapA1 = (rangeA1End - rangeA1Start) / segments;
                    gapTau1 = (rangeTau1End - rangeTau1Start) / segments;
                    gapY0 = (rangeY0End - rangeY0Start) / segments;
                    for(int segmentTau1 = 1; segmentTau1 <= segments; segmentTau1++){
                        for(int segmentA1 = 1; segmentA1 <= segments; segmentA1++){
                            for(int segmentY0 = 1; segmentY0 <= segments; segmentY0++){
                                double currentGoodness = countGoodness(twoExpFit->tauZero, twoExpFit->aZero, rangeTau1Start+gapTau1*segmentTau1, rangeA1Start+gapA1*segmentA1,rangeY0Start+gapY0*(segmentY0-1));
                                if ((currentGoodness < bestGoodness) || (bestGoodness == -1)){
                                    bestGoodness = currentGoodness;
                                    twoExpFit->tauOne = rangeTau1Start+gapTau1*segmentTau1;
                                    twoExpFit->aOne = rangeA1Start+gapA1*segmentA1;
                                    twoExpFit->yZero = rangeY0Start+gapY0*(segmentY0-1);
                                }
                            }
                        }
                    }
                    rangeA1Start = twoExpFit->aOne-gapA1;
                    rangeA1End =  twoExpFit->aOne+gapA1;
                    rangeTau1Start = twoExpFit->tauOne-gapTau1;
                    rangeTau1End = twoExpFit->tauOne+gapTau1;
                    rangeY0Start = twoExpFit->yZero-gapY0;
                    rangeY0End = twoExpFit->yZero+gapY0;
                }
            }
        }
    }

    // Fit two exponents
    int findTwoFittingExp(){
        if (dataPointCount < 2) return -1;
        findTwoFittingExpAll();
        findTwoFittingExpOne();
        findTwoFittingExpZero();
    }

    // To save Instrument point data similar like for material data
    void addNodeInstrument(double makeTime, int makeIntensity){
        dataNode* creatingNode = new dataNode;
        creatingNode->time = makeTime;
        creatingNode->intensity = makeIntensity;
        if (instrumentPointCount == 0){ // in case no points yet
            instrumentFirst = creatingNode;
        }
        else{
            creatingNode->previous = instrumentLast;
            instrumentLast->next = creatingNode;
        }
        instrumentPointCount++;
        instrumentLast = creatingNode;
    }

    // To save Instrument data in pairs
    int readFileInstrument(char fileName[255]){
        ifstream inputFile(fileName);
        removeAllInstrumentData();
        if (inputFile)
        while (true){
                double timeFromFile;
                int photonCountFromFile;
                inputFile >> timeFromFile >> photonCountFromFile;
                addNodeInstrument(timeFromFile,photonCountFromFile);
                if( inputFile.eof() ) break;
        }
        else
            return -1;
        return 0;
    }

    // To know where is the peak of material points in height (intensity scale)
    int findMaximumInstrument(){
        dataNode* findNode = instrumentFirst;
        int maximum = 0;
        while (findNode != 0){
            if (findNode->intensity>maximum)
                maximum = findNode->intensity;
            findNode = findNode->next;
        }
        return maximum;
    }

    // To center out both graphs with the maximums at time 0
    int alignInstrumentToMaterial(){
        if (instrumentPointCount < 1) return -1;
        int maximum = findMaximumInstrument();
        dataNode* centeringNode = instrumentFirst;
        while (centeringNode != 0 && centeringNode->intensity != maximum){ // find which is the maximum
            centeringNode = centeringNode->next;
        }
        double centerTime = centeringNode->time;
        centeringNode = instrumentFirst;
        while (centeringNode != 0){ // deduct time from other points to center it
            centeringNode->time -= centerTime;
            centeringNode = centeringNode->next;
        }
    }

    // To remove data that is not in actual instrument impulse and is often only noise
    int clearInstrument(){
        if (instrumentPointCount < 2) return -1;
        int maximum = findMaximumInstrument();
        dataNode* middleNode = instrumentFirst;
        while (middleNode != 0 && middleNode->intensity != maximum){ // find middle node
            middleNode = middleNode->next;
        }

        dataNode* findNode = middleNode;
        while (findNode != 0 && findNode->intensity > maximum*0.001){ // find precision to delete a region starting at
            findNode = findNode->next;
        }

        instrumentLast = findNode;
        findNode = findNode->next;
        instrumentLast->next = 0;
        dataNode* deleteNode = 0;
        while (findNode != 0){ // remove small data region
            instrumentPointCount--;
            deleteNode = findNode;
            findNode = findNode->next;
            delete deleteNode;
        }

        findNode = middleNode;
        while (findNode != 0 && findNode->intensity > maximum*0.001){ // find precision to delete a region starting at
            findNode = findNode->previous;
        }

        instrumentFirst = findNode;
        findNode = findNode->previous;
        instrumentFirst->previous = 0;

        while (findNode != 0){ // remove small data region
            instrumentPointCount--;
            deleteNode = findNode;
            findNode = findNode->previous;
            delete deleteNode;
        }
    }

    // To find best fitting deconvolution function using least squares method for goodness factor
    int deconvoluteData(){
        if (instrumentPointCount < 1 && dataPointCount < 2) return -1;
        if (instrumentPointCount < 1) return -2;
        if (dataPointCount < 2) return -3;
        int maximum = findMaximumInstrument();
        dataNode* errorChecker = instrumentFirst;
        int minimumCount = 0;
        bool alligned = false;
        for(;errorChecker!=0;errorChecker=errorChecker->next){
            if (errorChecker->intensity<maximum*0.001) minimumCount++;
            if (errorChecker->time == 0 && errorChecker->intensity == maximum) alligned = true;
        }
        if (minimumCount>2) return -6; // if Instrument is cleared from noise
        if (materialFirst->time != 0) return -5; // if Experiment data aligned
        if (alligned == false) return -4; // if Instrument is matched as maximum
        if (materialFirst->time!=0) return -8;
        if (maxPeak()!=0) return -7;

        double bestGoodness = -1; // Make a temporary best function saving point to later assign to output
        double bestCombination[8] = {0,0,0,0,0,0,0,0};
        double coefficientStart = 0;
        double coefficientEnd = materialFirst->intensity/findMaximumInstrument();
        double tauCoefficientStart = 0;
        double tauCoefficientEnd = 1;
        double noiseStart = 0;
        double noiseEnd = materialFirst->intensity;
        double offsetStart = 0;
        double offsetEnd = 0.1;
        int sectorCount = 3;
        for(int i=0; i<70 && (noiseEnd-noiseStart+offsetEnd-offsetStart+tauCoefficientEnd-tauCoefficientStart+coefficientEnd-coefficientStart)>accuracy; i++){
            for(int sectorCoefficient = 1; sectorCoefficient<=sectorCount; sectorCoefficient++){
                for(int sectorTau = 1; sectorTau<=sectorCount; sectorTau++){
                    for(int sectorNoise = 0; sectorNoise<sectorCount; sectorNoise++){
                        for(int sectorOffset = 0; sectorOffset<sectorCount; sectorOffset++){
                            deconvoluteSumCounting(coefficientStart+sectorCoefficient*(coefficientEnd-coefficientStart)/sectorCount,
                                                   tauCoefficientStart+sectorTau*(tauCoefficientEnd-tauCoefficientStart)/sectorCount,
                                                   noiseStart+sectorNoise*(noiseEnd-noiseStart)/sectorCount,offsetStart+sectorOffset*(offsetEnd-offsetStart)/sectorCount);
                            if (deconvolutionGoodness()<=bestGoodness||bestGoodness==-1){ //temporary save of best combination within segments
                                bestGoodness=deconvolutionGoodness();
                                bestCombination[0] = coefficientStart+(sectorCoefficient-1)*(coefficientEnd-coefficientStart)/sectorCount;
                                bestCombination[1] = coefficientStart+sectorCoefficient*(coefficientEnd-coefficientStart)/sectorCount;
                                bestCombination[2] = tauCoefficientStart+(sectorTau-1)*(tauCoefficientEnd-tauCoefficientStart)/sectorCount;
                                bestCombination[3] = tauCoefficientStart+sectorTau*(tauCoefficientEnd-tauCoefficientStart)/sectorCount;
                                bestCombination[4] = noiseStart+sectorNoise*(noiseEnd-noiseStart)/sectorCount;
                                bestCombination[5] = noiseStart+(sectorNoise+1)*(noiseEnd-noiseStart)/sectorCount;
                                bestCombination[6] = offsetStart+sectorOffset*(offsetEnd-offsetStart)/sectorCount;
                                bestCombination[7] = offsetStart+(sectorOffset+1)*(offsetEnd-offsetStart)/sectorCount;
                            }
                        }
                    }
                }
            }
            coefficientStart = bestCombination[0];
            coefficientEnd = bestCombination[1];
            tauCoefficientStart = bestCombination[2];
            tauCoefficientEnd = bestCombination[3];
            noiseStart = bestCombination[4];
            noiseEnd = bestCombination[5];
            offsetStart = bestCombination[6];
            offsetEnd = bestCombination[7];
        }
        cout << (bestCombination[0]+bestCombination[1])/2 << " // Tau - " << (bestCombination[2]+bestCombination[3])/2 << " with A0 - " << materialFirst->deconvolutionSum <<
            " // " << (bestCombination[4]+bestCombination[5])/2 << " " << (bestCombination[6]+bestCombination[7])/2 << endl;
        fixedDeconvoluteSumCounting((bestCombination[0]+bestCombination[1])/2,(bestCombination[2]+bestCombination[3])/2,(bestCombination[4]+bestCombination[5])/2,(bestCombination[6]+bestCombination[7])/2);
    }

    // To finds best fitting deconvolution function it needs to count for each material point how the function look in the end after adding up exponents
    void deconvoluteSumCounting(double coefficient, double tauCoefficient, double noise, double timeOffset){
        dataNode* materialNode = materialFirst;
        dataNode* instrumentNode = instrumentFirst;
        while (materialNode != 0){ // Have to clear out from data since that memory region is reused and will use +=
            materialNode->deconvolutionSum = 0;
            materialNode = materialNode->next;
        }
        materialNode = materialFirst;
        while (materialNode != 0){ //To find how good is the function it is important to go through
            instrumentNode = instrumentFirst;
            while (instrumentNode!=0){
                if (materialNode->time>=instrumentNode->time){
                    materialNode->deconvolutionSum += coefficient*instrumentNode->intensity*exp(-((materialNode->time-instrumentNode->time-timeOffset)/tauCoefficient));
                }
                instrumentNode = instrumentNode->next;
            }
            materialNode->deconvolutionSum += noise;
            materialNode = materialNode->next;
        }
    }

    // To count last function but Experiment data
    void fixedDeconvoluteSumCounting(double coefficient, double tauCoefficient, double noise, double timeOffset){
        dataNode* materialNode = materialFirst;
        dataNode* instrumentNode = instrumentFirst;
        while (materialNode != 0){
            materialNode->deconvolutionSum = 0;
            materialNode = materialNode->next;
        }
        materialNode = materialFirst;
        while (materialNode != 0){
            instrumentNode = instrumentFirst;
            while (instrumentNode!=0 && instrumentNode->time<=0){
                if (materialNode->time>=instrumentNode->time+timeOffset){
                    materialNode->deconvolutionSum += coefficient*instrumentNode->intensity*exp(-((materialNode->time-instrumentNode->time)/tauCoefficient));
                }
                instrumentNode = instrumentNode->next;
            }
            materialNode->deconvolutionSum += noise;
            materialNode = materialNode->next;
        }
    }

    // To compare different variables and see how good is the fit
    float deconvolutionGoodness(){
        double holderGood = 0;
        dataNode* pointerNode = materialFirst;
        while (pointerNode != 0){
            holderGood += pow( pointerNode->deconvolutionSum - pointerNode->intensity, 2  );
            pointerNode = pointerNode->next;
        }
        return holderGood/dataPointCount;
    }

    // To deconvolute the data with two exponents
    int deconvoluteDataTwoExp(){
        if (instrumentPointCount < 1 && dataPointCount < 2) return -1;
        if (instrumentPointCount < 1) return -2;
        if (dataPointCount < 2) return -3;
        int maximum = findMaximumInstrument();
        dataNode* errorChecker = instrumentFirst;
        int minimumCount = 0;
        bool alligned = false;
        for(;errorChecker!=0;errorChecker=errorChecker->next){
            if (errorChecker->intensity<maximum*0.001) minimumCount++;
            if (errorChecker->time == 0 && errorChecker->intensity == maximum) alligned = true;
        }
        if (minimumCount>2) return -6; // if Instrument is cleared from noise
        if (materialFirst->time != 0) return -5; // if Experiment data aligned
        if (alligned == false) return -4; // if Instrument is matched as maximum
        if (materialFirst->time!=0) return -8;
        if (maxPeak()!=0) return -7;
        double bestGoodness = -1;
        double bestCombination[10] = {0,0,0,0,0,0,0,0,0,0};
        double coefficientStart = 0;
        double coefficientEnd = materialFirst->intensity/findMaximumInstrument();
        double tauCoefficientStart = 0;
        double tauCoefficientEnd = 1;
        double tauTwoCoefficientStart = 0;
        double tauTwoCoefficientEnd = 0.5;
        double noiseStart = 0;
        double noiseEnd = materialFirst->intensity;
        double offsetStart = 0;
        double offsetEnd = 0.1;
        int sectorCount = 2;
        for(int i=0; i<70 && (noiseEnd-noiseStart+offsetEnd-offsetStart+tauCoefficientEnd-tauCoefficientStart+coefficientEnd-coefficientStart)>accuracy; i++){
            for(int sectorTau = 1; sectorTau<=sectorCount; sectorTau++){
                for(int sectorTauTwo = 1; sectorTauTwo<=sectorCount; sectorTauTwo++){
                    if (tauCoefficientStart+sectorTau*(tauCoefficientEnd-tauCoefficientStart)/sectorCount>=tauTwoCoefficientStart+sectorTau*(tauTwoCoefficientEnd-tauTwoCoefficientStart)/sectorCount)
                        for(int sectorCoefficient = 1; sectorCoefficient<=sectorCount; sectorCoefficient++){
                            for(int sectorNoise = 0; sectorNoise<sectorCount; sectorNoise++){
                                for(int sectorOffset = 0; sectorOffset<sectorCount; sectorOffset++){
                                    deconvoluteSumCounting(coefficientStart+sectorCoefficient*(coefficientEnd-coefficientStart)/sectorCount,
                                                           tauCoefficientStart+sectorTau*(tauCoefficientEnd-tauCoefficientStart)/sectorCount,
                                                           tauTwoCoefficientStart+sectorTau*(tauTwoCoefficientEnd-tauTwoCoefficientStart)/sectorCount,
                                                           noiseStart+sectorNoise*(noiseEnd-noiseStart)/sectorCount,offsetStart+sectorOffset*(offsetEnd-offsetStart)/sectorCount);
                                    if (deconvolutionGoodness()<=bestGoodness||bestGoodness==-1){
                                        bestGoodness=deconvolutionGoodness();
                                        bestCombination[0] = coefficientStart+(sectorCoefficient-1)*(coefficientEnd-coefficientStart)/sectorCount;
                                        bestCombination[1] = coefficientStart+sectorCoefficient*(coefficientEnd-coefficientStart)/sectorCount;
                                        bestCombination[2] = tauCoefficientStart+(sectorTau-1)*(tauCoefficientEnd-tauCoefficientStart)/sectorCount;
                                        bestCombination[3] = tauCoefficientStart+sectorTau*(tauCoefficientEnd-tauCoefficientStart)/sectorCount;
                                        bestCombination[4] = tauTwoCoefficientStart+(sectorTau-1)*(tauTwoCoefficientEnd-tauTwoCoefficientStart)/sectorCount;
                                        bestCombination[5] = tauTwoCoefficientStart+sectorTau*(tauTwoCoefficientEnd-tauTwoCoefficientStart)/sectorCount;
                                        bestCombination[6] = noiseStart+sectorNoise*(noiseEnd-noiseStart)/sectorCount;
                                        bestCombination[7] = noiseStart+(sectorNoise+1)*(noiseEnd-noiseStart)/sectorCount;
                                        bestCombination[8] = offsetStart+sectorOffset*(offsetEnd-offsetStart)/sectorCount;
                                        bestCombination[9] = offsetStart+(sectorOffset+1)*(offsetEnd-offsetStart)/sectorCount;
                                    }
                                }
                            }
                        }
                }
            }
            coefficientStart = bestCombination[0];
            coefficientEnd = bestCombination[1];
            tauCoefficientStart = bestCombination[2];
            tauCoefficientEnd = bestCombination[3];
            tauTwoCoefficientStart = bestCombination[4];
            tauTwoCoefficientEnd = bestCombination[5];
            noiseStart = bestCombination[6];
            noiseEnd = bestCombination[7];
            offsetStart = bestCombination[8];
            offsetEnd = bestCombination[9];
        }
        cout << (bestCombination[0]+bestCombination[1])/2 << " // Tau - " << (bestCombination[2]+bestCombination[3])/2 << " // TauTwo - " << (bestCombination[4]+bestCombination[5])/2 << " //  A0 - " << materialFirst->deconvolutionSum << " Noise " << (bestCombination[6]+bestCombination[7])/2 << " timeOffset " << (bestCombination[8]+bestCombination[9])/2 << " " << bestGoodness << endl;
    }

    // To count the NLLS values
    deconvoluteSumCounting(double coefficient, double tauCoefficient, double tauTwoCoefficient, double noise, double timeOffset){
        dataNode* materialNode = materialFirst;
        dataNode* instrumentNode = instrumentFirst;
        while (materialNode != 0){ // Have to clear out from data since that memory region is reused and will use +=
            materialNode->deconvolutionSum = 0;
            materialNode = materialNode->next;
        }
        materialNode = materialFirst;
        while (materialNode != 0){ //To find how good is the function it is important to go through
            instrumentNode = instrumentFirst;
            while (instrumentNode!=0){
                if (materialNode->time>=instrumentNode->time){
                    materialNode->deconvolutionSum += coefficient*instrumentNode->intensity*exp(-((materialNode->time-instrumentNode->time-timeOffset)/tauCoefficient));
                    materialNode->deconvolutionSum += coefficient*instrumentNode->intensity*exp(-((materialNode->time-instrumentNode->time-timeOffset)/tauTwoCoefficient));
                }
                instrumentNode = instrumentNode->next;
            }
            materialNode->deconvolutionSum += noise;
            materialNode = materialNode->next;
        }
    }

    // To work on a specific data point section and count the values
    double interpolateSegment(dataNode* basePointer, double xi, int n){
        double result = 0; // Initialize result
        dataNode* instrumentPointer = basePointer;
        for (int i=0; i<n; i++)
        {
            // Compute individual terms of above formula
            double term = instrumentPointer->intensity;//double term = f[i].y;
            dataNode* secondInstrumentPointer = basePointer;
            for (int j=0;j<n;j++)
            {
                if (j!=i)
                    term = term*double(xi - secondInstrumentPointer->time)/double(instrumentPointer->time - secondInstrumentPointer->time);
                secondInstrumentPointer = secondInstrumentPointer->next;
            }

            // Add current term to result
            result += term;
            instrumentPointer = instrumentPointer->next;
        }
        return result;
    }

    // To reduce point count in instrument data using interpolation with 5 points
    int interpolateInstrumentData(){
        cout << "Interpolation point count: ";
        cin >> interpolationStepCount;
        if (instrumentPointCount < 5)
            return -3;
        if (interpolationStepCount < 1)
            return -1;
        if (interpolationStepCount >= instrumentPointCount)
            return -2;
        interpolationStep = (instrumentLast->time-instrumentFirst->time) / (interpolationStepCount-1);
        dataNode* instrumentDataPointer = instrumentFirst;
        dataNode* saveInterpolationPointer = instrumentFirst;
        for(int newPoint=0; newPoint < interpolationStepCount; newPoint++){
            while(instrumentDataPointer->next->next->next->next->next->next!=0 && instrumentDataPointer->next->next->next->time<=instrumentFirst->time+newPoint*interpolationStep)
                instrumentDataPointer = instrumentDataPointer->next;
            saveInterpolationPointer->deconvolutionSum = interpolateSegment(instrumentDataPointer,instrumentFirst->time+newPoint*interpolationStep,5);
            if (saveInterpolationPointer->deconvolutionSum < 0) saveInterpolationPointer->deconvolutionSum = 0;
            saveInterpolationPointer = saveInterpolationPointer->next;
        }


        instrumentDataPointer = instrumentFirst;
        for(int i=0; i < interpolationStepCount;i++){
            instrumentDataPointer->intensity=instrumentDataPointer->deconvolutionSum;
            instrumentDataPointer->deconvolutionSum = 0;
            instrumentDataPointer->time = instrumentFirst->time+i*interpolationStep;
            instrumentDataPointer = instrumentDataPointer->next;
        }


        saveInterpolationPointer = instrumentDataPointer;
        instrumentLast = instrumentDataPointer->previous;
        instrumentDataPointer->previous->next = 0;
        while (instrumentDataPointer != 0){
            saveInterpolationPointer = instrumentDataPointer;
            instrumentDataPointer = instrumentDataPointer->next;
            delete saveInterpolationPointer;
        }

        instrumentDataPointer = instrumentFirst;
        while(instrumentDataPointer!=0){
            instrumentDataPointer = instrumentDataPointer->next;
        }
    }

    // To clear data from previously saved data points
    void removeAllInstrumentData(){
        if (instrumentPointCount == 0) return;
        dataNode* instrumentDataPointer = instrumentFirst;
        dataNode* deleteInterpolationPointer = instrumentDataPointer;
        instrumentFirst = 0;
        instrumentLast = 0;
        instrumentPointCount = 0;
        while (instrumentDataPointer != 0){
            deleteInterpolationPointer = instrumentDataPointer;
            instrumentDataPointer = instrumentDataPointer->next;
            delete deleteInterpolationPointer;
        }
    }
};

// prompt to ask for file name
void inputFileName(char* name){
    cout << "Input file name ";
    cin >> name;
}
// prompt to ask for file name
void inputInstrumentFileName(char* name){
    cout << "Input Instrument file name ";
    cin >> name;
}

int main()
{
    /*
    Application currently focuses on analyzing Experiment data that is saved in 2 columns <time, intensity>
    The material data can be read [2] by specified file name [1]
    The instrument data can b read [12] by specified file name [11]
    Material data can be changed
        - Clear till materialFirst maximum [3]
        - Change time scale so the time starts with 0 [4]
    Material Data can be analyzed
        - Fitting Exp to data [5]
        - Fitting 2 exponents to data [10]
        - Deconvolution [15]
    Instrument data can be changed
        - Align instrument maximum peak with 0 on time scale [13]
        - Clear the data to leave only the impulse of instrument [14]
    Exponent data can be written to file
        - Exp fitted to data [6] from selected folder [8]
        - Difference from Exp and read data [7] from selected folder [9]
    Stop analyzing data [0]
    */
    int functionDescriptionCount = 16;
    string functionDescription[functionDescriptionCount] = {
        "End work", //0
        "!! Assign reading file and read file",
        "Clear till material first maximum",
        "Move graph time to 0",
        "Fit exp",
        "Assign exp output file and write exp to file", //5
        "Assign difference output file and write difference to file",
        "Fit of two exponents",
        "Assign two exp output file and write exp to file",
        "Assign difference output file and write difference to file",
        "!! Assign instrument data file and read instrument data", //10
        "Align Instrument data to 0",
        "Remove small data range from Instrument data",
        "Interpolate instrument data points",
        "Deconvolve material data with instrument data",
        "Deconvolve material data with instrument data with 2 exponents"//15
        };
    string errorResponses[100] = {
        "Ending work...", // 0
        "Experiment data set has 1 or no data points",
        "Experiment data has no data points",
        "Instrument data set has 1 or less data points",
        "Instrument and experiment data doesn't contain enough data",
        "Instrument data has no data points", //5
        "Instrument data is not aligned to time 0 with maximum",
        "Experiment data is not aligned to time 0",
        "Instrument data still contains noise",
        "Invalid interpolation step count - negative",
        "Invalid interpolation step count - more than instrument data points", //10
        "Instrument data contains no data points or less than 5",
        "Experiment data doesn't have maximum at 0",
        "Experiment data has data before maximum",
        "File doesn't exist",
        "Exponent(s) is(are) not fitted"
        };
    string functionResponses[100] = {
        "Ending work...", // 0
        "Experiment data file is selected",
        "Experiment data file was read",
        "Data cleared till material first maximum",
        "Graph moved on time axis to 0",
        "File selected for Exp", // 5
        "File selected for Difference",
        "Exp written to file - ",
        "Difference written to file - ",
        "Instrument data file is selected",
        "Instrument data file was read", //10
        "Instrument data was aligned with material data with maximums at 0",
        "Instrument data has been cleared and only impulse is left",
        "Interpolation complete"
        };
    for(int i=0;i<functionDescriptionCount;i++){
        cout << i;
        if (i<10)
            cout << "   ";
        else
            cout << "  ";

        cout << functionDescription[i];
        if (i < functionDescriptionCount-1)
            cout << "," << endl;
        else
            cout << "." << endl;
    }


    int command;
    linkedList* testList = new linkedList;
    char nameOfInputFile[255] = "";
    char nameOfInputFileInstrument[255] = "";
    char nameOfOutputFileExp[255] = "";
    char nameOfOutputFileNLLS[255] = ""; //NLLS --- non-linear least squares method
    char nameOfOutputFileExpTwo[255] = "";
    char nameOfOutputFileNLLStwo[255] = "";
    bool caseTrue = true;
    int errorCode = 0;

    oneExp* testOneExp = new oneExp;
    double accuracy = 0.00001;

    while (caseTrue == true){ // Functions are picked by entering a number ranging from 0 to 15 to analize data
        cout << "Type function: ";
        cin >> command;
        clock_t start = clock();
        switch (command) {
        case 0 :
            { // to close program
                cout << functionResponses[0] << endl;
                caseTrue = false;
                break;
            }
        case 1 :
            { // to pick experiment file user selects it and it reads it right after
                inputFileName(nameOfInputFile);
                cout << functionResponses[1] << endl;
                errorCode = testList->readFile(nameOfInputFile);
                switch (errorCode){
                case 0 :
                {
                    cout << functionResponses[2] << endl;
                    break;
                }
                case -1 :
                {
                    cout << errorResponses[14] << endl;
                    break;
                }
                case -2 :
                {
                    cout << errorResponses[2] << endl;
                    break;
                }
                }
                break;
            }
        case 2 :
            { // use to advance to next functions
                errorCode = testList->clearTillTime(testList->maxPeak());
                if (errorCode>=0)
                    cout << functionResponses[3] << endl;
                else if (errorCode == -1)
                    cout << errorResponses[1] << endl;
                break;
            }
        case 3 :
            { // use to advance to next functions
                errorCode = testList->moveTimeZero();
                if (errorCode>=0)
                    cout << functionResponses[4] << endl;
                else if (errorCode == -1)
                    cout << errorResponses[2] << endl;
                break;
            }
        case 4 :
            { // will produce a fitting equation for user
                errorCode = testList->findOneFittinExp();
                switch (errorCode){
                case 0 :
                    {}
                case -1 :
                    {
                       cout << errorResponses[1] << endl;
                       break;
                    }
                case -2 :
                    {
                        cout << errorResponses[13] << endl;
                        break;
                    }
                case -3 :
                    {
                        cout << errorResponses[12] << endl;
                        break;
                    }
                }
                break;
            }
        case 5 :
            { // will produce a file with data points of exponent for user
                inputFileName(nameOfOutputFileExp);
                cout << functionResponses[5] << endl;
                errorCode = testList->drawExpToFile1(nameOfOutputFileExp);
                switch (errorCode){
                case 0 :
                    {
                        cout << functionResponses[7] << nameOfOutputFileExp << endl;
                        break;
                    }
                case -1 :
                    {
                        cout << errorResponses[1] << endl;
                        break;
                    }
                case -2 :
                    {
                        cout << errorResponses[15] << endl;
                    }
                }
                break;
            }
        case 6 :
            { // will produce a file with data point difference of exponent and experiment data for user
                inputFileName(nameOfOutputFileNLLS);
                cout << functionResponses[6] << endl;
                errorCode = testList->drawGoodnessToFile1(nameOfOutputFileNLLS);
                switch (errorCode){
                case 0 :
                    {
                        cout << functionResponses[8] << nameOfOutputFileNLLS << endl;
                        break;
                    }
                case -1 :
                    {
                        cout << errorResponses[1] << endl;
                        break;
                    }
                case -2 :
                    {
                        cout << errorResponses[15] << endl;
                    }
                }
                break;
            }
        case 7 :
            { // will produce fitting a equation for user
                errorCode = testList->findTwoFittingExp();
                if (errorCode>=0)
                {
                    cout << " Tau0 - " << testList->twoExpFit->tauZero << " A0 - " << testList->twoExpFit->aZero << " Tau1 - " << testList->twoExpFit->tauOne << " A1 - " << testList->twoExpFit->aOne << " Y1 - " << testList->twoExpFit->yZero << " Goodness - " << (testList->countGoodness(testList->twoExpFit->tauZero, testList->twoExpFit->aZero, testList->twoExpFit->tauOne, testList->twoExpFit->aOne, testList->twoExpFit->yZero) / testList->dataPointCount) << endl;
                }
                else if (errorCode == -1)
                    cout << errorResponses[1] << endl; //errorResponses[]
                break;
            }
        case 8 :
            { // will produce a file with data points of exponent for user
                inputFileName(nameOfOutputFileExpTwo);
                cout << functionResponses[5] << endl;
                errorCode =  testList->drawExpToFile2(nameOfOutputFileExpTwo);
                switch (errorCode){
                case 0 :
                    {
                cout << functionResponses[7] << nameOfOutputFileExpTwo << endl;
                break;
                    }
                case -1 :
                    {
                        cout << errorResponses[1] << endl;
                        break;
                    }
                case -2 :
                    {
                        cout << errorResponses[15] << endl;
                    }
                }
                break;
            }
        case 9 :
            { // will produce a file with data point difference of exponent and experiment data for user
                inputFileName(nameOfOutputFileNLLStwo);
                cout << functionResponses[6] << endl;
                errorCode = testList->drawGoodnessToFile2(nameOfOutputFileNLLStwo);
                switch (errorCode){
                case 0 :
                    {
                        cout << functionResponses[8] << nameOfOutputFileNLLStwo << endl;
                        break;
                    }
                case -1 :
                    {
                        cout << errorResponses[1] << endl;
                        break;
                    }
                case -2 :
                    {
                        cout << errorResponses[15] << endl;
                    }
                }
                break;
            }
        case 10 :
            { // to pick instrument file user selects it and it reads it right after
                inputInstrumentFileName(nameOfInputFileInstrument);
                cout << functionResponses[9] << endl;
                errorCode = testList->readFileInstrument(nameOfInputFileInstrument);
                if (errorCode >= 0)
                    cout << functionResponses[10] << endl;
                else
                    cout << errorResponses[5] << endl;
                break;
            }
        case 11 :
            { // use to analize data for user to use further
                errorCode = testList->alignInstrumentToMaterial();
                if (errorCode>=0)
                    cout << functionResponses[11] << endl;
                else if (errorCode == -1)
                    cout << errorResponses[3] << endl;
                break;
            }
        case 12 :
            { // use to analize data for user to use further
                errorCode = testList->clearInstrument();
                if (errorCode>=0)
                    cout << functionResponses[12] << endl;
                else if (errorCode == -1)
                    cout << errorResponses[3] << endl;
                break;
            }
        case 13 :
            { // use for user to change data to use further
                errorCode = testList->interpolateInstrumentData();
                switch (errorCode){
                case 0 :
                    {
                        cout << functionResponses[14] << endl;
                        break;
                    }
                case -1 :
                    {
                        cout << errorResponses[9] << endl;
                        break;
                    }
                case -2 :
                    {
                        cout << errorResponses[10] << endl;
                        break;
                    }
                case -3 :
                    {
                        cout << errorResponses[11] << endl;
                    }
                }
                break;

            }
        case 14 :
            { // allows user to find what is one descending exponent equation variables
                errorCode = testList->deconvoluteData();
                switch (errorCode){
                case 0:
                    {
                        //cout << functionResponses[14] << endl;
                        break;
                    }
                case -1:
                    {
                        cout << errorResponses[4] << endl;
                        break;
                    }
                case -2:
                    {
                        cout << errorResponses[5] << endl;
                        break;
                    }
                case -3:
                    {
                        cout << errorResponses[1] << endl;
                        break;
                    }
                case -4:
                    {
                        cout << errorResponses[6] << endl;
                        break;
                    }
                case -5:
                    {
                        cout << errorResponses[7] << endl;
                        break;
                    }
                case -6:
                    {
                        cout << errorResponses[8] << endl;
                        break;
                    }
                case -7:
                    {
                        cout << errorResponses[12] << endl;
                        break;
                    }
                case -8:
                    {
                        cout << errorResponses[13] << endl;
                        break;
                    }
                }
                break;

            }
        case 15 :
            { // will produce a deconvolution equation for user
                errorCode = testList->deconvoluteDataTwoExp();
                switch (errorCode){
                case 0:
                    {
                        break;
                    }
                case -1:
                    {
                        cout << errorResponses[4] << endl;
                        break;
                    }
                case -2:
                    {
                        cout << errorResponses[5] << endl;
                        break;
                    }
                case -3:
                    {
                        cout << errorResponses[1] << endl;
                        break;
                    }
                case -4:
                    {
                        cout << errorResponses[6] << endl;
                        break;
                    }
                case -5:
                    {
                        cout << errorResponses[7] << endl;
                        break;
                    }
                case -6:
                    {
                        cout << errorResponses[8] << endl;
                        break;
                    }
                }
                break;
            }
        }
        clock_t stop = clock();
        double elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC; // time passed during a work of function
        printf("Time elapsed in ms: %f", elapsed);
        cout << endl;
        errorCode = 0;
    }
    return 0;
}

