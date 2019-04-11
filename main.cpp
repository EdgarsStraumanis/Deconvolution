#include <iostream>
#include <fstream>
#include <math.h>
#include <windows.h>

// V1.2.1

using namespace std;

struct oneExp{
    double tauZero = 1;
    double aZero = 0;
    double yZeroOne = 0;
    double yZeroTwo = 0;
    double yZero = (yZeroTwo - yZeroOne)/2;
};

struct twoBeams{
    /*
    Point that is located time = 0 which will have coordinates (X0,Y0) and angle Alpha

    Point that is materialLast time point which will have coordinates (Xk,Yk) and angle Beta
    */
    double xZero = 0;
    double yZero = 0;
    double xK = 0;
    double yK = 0;
    double alpha = 0;
    double beta = 0;

    void print(){
        cout << "xZero=" << xZero << " yZero=" << yZero << " xK=" << xK << " yK=" << yK << " alpha=" << alpha << " beta=" << beta << endl;
    }
};

struct twoExp{
    double tauZero = 0;
    double aZero = 0;
    double tauOne = 0;
    double aOne = 0;
    //double yZeroOne = 0;
    //double yZeroTwo = 0;
    double yZero = 0; //(yZeroTwo - yZeroOne)/2;
    twoBeams* beamCalculation = new twoBeams;
};


/*
struct twoCombinedLines{ //y=aZero*X+aOne*X+bZero
    double aZero
    double aOne
    double bZero
};
*/
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
struct dataNode{
    double time;
    int intensity;
    double deconvolutionSum = 0;
    dataNode* next = 0;
    dataNode* previous = 0;
};

// allows to hold a list of data-points - time, intensity; Contains a various amount of functions to work with the data
struct linkedList{
    dataNode* materialFirst = 0;
    dataNode* materialLast = 0;
    dataNode* instrumentalFirst = 0;
    dataNode* instrumentLast = 0;
    dataNode* instrumentPointCount = 0;
    int dataPointCount = 0;
    oneExp* oneExpFit = new oneExp;
    twoExp* twoExpFit = new twoExp;
    threeExp* threeExpFit = new threeExp;
    double accuracy = 0.0000001;

    // Allows to add data-points to list as materialLast element
    void addNode(double makeTime, int makeIntensity){
        dataNode* creatingNode = new dataNode;
        creatingNode->time = makeTime;
        creatingNode->intensity = makeIntensity;
        if (dataPointCount == 0){
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

    //Removes all data-Nodes from beginning till specified time but not including time that was written
    void clearTillTime(double timeTill){
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
    void moveTimeZero(){
        if (dataPointCount < 1) return;
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
            holderGood += pow( zeroA * exp(-(pointerNode->time / tau)) - pointerNode->intensity + zeroY, 2  );
            // cout << pointerNode->intensity << " " << zeroA * exp(-(pointerNode->time / tau)) << endl;
            pointerNode = pointerNode->next;
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
                addNode(timeFromFile,intensityFromFile);
                if( inputFile.eof() ) break;
        }
    }

    // write to file a Node of data < time, difference between read data and fitted function at time moment>
    void drawGoodnessToFile1(char fileName[255]){
        ofstream outputFile(fileName);
        dataNode* pointerNode = materialFirst;
        while (pointerNode != 0){
            outputFile << pointerNode->time << " " << (pointerNode->intensity - (oneExpFit->aZero * exp(-(pointerNode->time / oneExpFit->tauZero)) + oneExpFit->yZero) ) << endl;
            pointerNode = pointerNode->next;
        }
    }

    // write to file a Node of data < time, fitted function at time moment >
    void drawExpToFile1(char fileName[255]){
        ofstream outputFile(fileName);
        dataNode* pointerNode = materialFirst;
        while (pointerNode != 0){
            outputFile << pointerNode->time << " " << (oneExpFit->aZero * exp(-(pointerNode->time / oneExpFit->tauZero)) + oneExpFit->yZero) << endl;
            pointerNode = pointerNode->next;
        }
    }

    // -----------One exponent fitting-------------
    // Tries to find sector for Tau for exp using golden cut method by reducing a range by halving the region whichever fits better
    double findSectorTau1(){
        double sectorFirst = 0;
        double sectorSecond = materialLast->time;
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
        double sectorSecond = materialFirst->intensity;
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
        //cout << countGoodness(sectorFirst + 0.5 * gap, materialFirst->intensity) << " " << countGoodness(sectorSecond + 0.5 * gap, materialFirst->intensity) << endl;
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
        oneExpFit->aZero = materialFirst->intensity;
        oneExpFit->yZeroTwo = materialFirst->intensity;
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
        dataNode* pointerNode = materialFirst;
        while (pointerNode != 0){
            holderGood += pow( aZero * exp(-(pointerNode->time / tauZero)) + aOne * exp(-(pointerNode->time / tauOne)) + yZero - pointerNode->intensity , 2  );
            // cout << pointerNode->intensity << " " << zeroA * exp(-(pointerNode->time / tau)) << endl;
            pointerNode = pointerNode->next;
        }
        return holderGood;
    }
    /*
    void materialFirstExpFit(){
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
        double sectorSecond = materialLast->time;
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
        double sectorSecond = materialLast->time;
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
        double sectorSecond = materialFirst->intensity;
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
        double sectorSecond = materialFirst->intensity;
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
        materialFirstExpFit();

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
        for(int i = 0;i<4;i++){
            cout << "Iteration - " << i << endl;
            segments = 3;
            for(;segments<=4;segments++){
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
                for(int iterations=0; iterations < 100/segments; iterations++){
                    gapA0 = (rangeA0End - rangeA0Start) / segments;
                    gapTau0 = (rangeTau0End - rangeTau0Start) / segments;
                    gapA1 = (rangeA1End - rangeA1Start) / segments;
                    gapTau1 = (rangeTau1End - rangeTau1Start) / segments;
                    gapY0 = (rangeY0End - rangeY0Start) / segments;
                    for(int segmentTau0 = 1; segmentTau0 <= segments; segmentTau0++){
                        for(int segmentTau1 = 1; segmentTau1 <= segments; segmentTau1++){
                            //if (rangeTau1Start+gapTau1*(segmentTau1-100) >= rangeTau0Start+gapTau0*segmentTau0) break;
                            for(int segmentA0 = 1; segmentA0 <= segments; segmentA0++){
                                for(int segmentA1 = 1; segmentA1 <= segments; segmentA1++){
                                    for(int segmentY0 = 1; segmentY0 <= segments; segmentY0++){
                                        if (rangeA0Start+gapA0*segmentA0 < rangeA1Start+gapA1*segmentA1) continue;

                                        double currentGoodness = countGoodness(rangeTau0Start+gapTau0*segmentTau0, rangeA0Start+gapA0*segmentA0,rangeTau1Start+gapTau1*segmentTau1,rangeA1Start+gapA1*segmentA1,rangeY0Start+gapY0*(segmentY0-1));
                                        //cout << currentGoodness << endl;
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
                    rangeA0Start = twoExpFit->aZero-gapA0;
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
            cout << "Segments - " << segments << " " << twoExpFit->tauZero << " " << twoExpFit->aZero << " " << twoExpFit->tauOne << " " << twoExpFit->aOne << " " << twoExpFit->yZero << " " << bestGoodness << endl;
            }
        }
    }

    void findTwoFittingExpZero(){
        int segments;
        double bestGoodness = -1;//countGoodness(twoExpFit->tauZero, twoExpFit->aZero, twoExpFit->tauOne, twoExpFit->aOne, twoExpFit->yZero);
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
        for(int i = 0;i<3;i++){
            cout << "Iteration - " << i << endl;
            segments = 8;
            for(;segments<=20;segments++){
                rangeA0Start = 0;
                rangeA0End = twoExpFit->aZero*2;
                rangeTau0Start = 0;
                rangeTau0End = twoExpFit->tauZero*2;
                rangeY0Start = 0;
                rangeY0End = twoExpFit->yZero*2;
                for(int iterations=0; iterations < 100/segments; iterations++){
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
            cout << "Segments - " << segments << " " << twoExpFit->tauZero << " " << twoExpFit->aZero << " " << twoExpFit->tauOne << " " << twoExpFit->aOne << " " << twoExpFit->yZero << " " << bestGoodness << endl;
            }
        }
    }

    void findTwoFittingExpOne(){
        int segments;
        double bestGoodness = -1;//countGoodness(twoExpFit->tauZero, twoExpFit->aZero, twoExpFit->tauOne, twoExpFit->aOne, twoExpFit->yZero);

        double rangeA1Start;
        double rangeA1End;
        double rangeTau1Start;
        double rangeTau1End;
        double rangeY0Start;
        double rangeY0End;
        double gapA1, gapTau1, gapY0;
        for(int i = 0;i<3;i++){
            cout << "Iteration - " << i << endl;
            segments = 8;
            for(;segments<=20;segments++){
                rangeA1Start = 0;
                rangeA1End = twoExpFit->aOne*2;
                rangeTau1Start = 0;
                rangeTau1End = twoExpFit->tauOne*2;
                rangeY0Start = 0;
                rangeY0End = twoExpFit->yZero*2;
                for(int iterations=0; iterations < 100/segments; iterations++){
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
            cout << "Segments - " << segments << " " << twoExpFit->tauZero << " " << twoExpFit->aZero << " " << twoExpFit->tauOne << " " << twoExpFit->aOne << " " << twoExpFit->yZero << " " << bestGoodness << endl;
            }
        }
    }

    void findTwoFittingExp(){
        findTwoFittingExpAll();
        findTwoFittingExpOne();
        findTwoFittingExpZero();
    }

    /*
    double sumFrontX(int pointCount){
        double sum = 0;
        dataNode* pointerNode = materialFirst;
        for(int i = 0; (pointerNode != 0 && i < pointCount); pointerNode = pointerNode->next){
            sum += pointerNode->time;
            i++;
        }
        return sum;
    }

    double sumFrontY(int pointCount){
        double sum = 0;
        dataNode* pointerNode = materialFirst;
        for(int i = 0; (pointerNode != 0 && i < pointCount); pointerNode = pointerNode->next){
            sum += pointerNode->intensity;
            i++;
        }
        return sum;
    }

    double sumFrontXY(int pointCount){
        double sum = 0;
        dataNode* pointerNode = materialFirst;
        for(int i = 0; (pointerNode != 0 && i < pointCount); pointerNode = pointerNode->next){
            sum += pointerNode->intensity*pointerNode->time;
            i++;
        }
        return sum;
    }

    double sumFrontXX(int pointCount){
        double sum = 0;
        dataNode* pointerNode = materialFirst;
        for(int i = 0; (pointerNode != 0 and i < pointCount); pointerNode = pointerNode->next){
            sum += pointerNode->time*pointerNode->time;
            i++;
        }
        return sum;
    }

    double lineSegmentLeastSquaresStart(int materialLastPoint, double yAtZero, double coefficientAtX){
        double sum = 0;
        dataNode* pointerNode = materialFirst;
        for(int i = 0; (pointerNode != 0 && i < dataPointCount); pointerNode = pointerNode->next){
            if ((coefficientAtX*pointerNode->time+yAtZero) < pointerNode->intensity)
                sum += pow(pointerNode->intensity-(coefficientAtX*pointerNode->time+yAtZero), 2);
            else
                sum += pow((coefficientAtX*pointerNode->time+yAtZero)-pointerNode->intensity, 2);
            //cout << pointerNode->intensity << " " << coefficientAtX*pointerNode->time+yAtZero << endl;
            i++;
        }
        return sum/dataPointCount;
    }

    void countStart(){
        double previous = 0;
        double current = 0;
        for(int pointCount = 5; pointCount < dataPointCount; pointCount++){

            double base = (pointCount*sumFrontXY(pointCount)-sumFrontX(pointCount)*sumFrontY(pointCount))/(pointCount*sumFrontXX(pointCount)-sumFrontX(pointCount)*sumFrontX(pointCount));
            twoExpFit->beamCalculation->yZero = (sumFrontXX(pointCount)*sumFrontY(pointCount)-sumFrontX(pointCount)*sumFrontXY(pointCount))/(pointCount*sumFrontXX(pointCount)-sumFrontX(pointCount)*sumFrontX(pointCount));
            current = lineSegmentLeastSquaresStart(pointCount, twoExpFit->beamCalculation->yZero, base);
            cout << pointCount << " yZero is " << twoExpFit->beamCalculation->yZero << " base is" << base << " squares is " << current << " difference " << current - previous << endl;
            previous = current;
        }
    }

    double sumBackX(int pointCount){
        double sum = 0;
        dataNode* pointerNode = materialLast;
        for(int i = 0; (pointerNode != 0 && i < pointCount); pointerNode = pointerNode->previous){
            sum += pointerNode->time;
            i++;
        }
        return sum;
    }

    double sumBackY(int pointCount){
        double sum = 0;
        dataNode* pointerNode = materialLast;
        for(int i = 0; (pointerNode != 0 && i < pointCount); pointerNode = pointerNode->previous){
            sum += pointerNode->intensity;
            i++;
        }
        return sum;
    }

    double sumBackXX(int pointCount){
        double sum = 0;
        dataNode* pointerNode = materialLast;
        for(int i = 0; (pointerNode != 0 && i < pointCount); pointerNode = pointerNode->previous){
            sum += pointerNode->time*pointerNode->time;
            i++;
        }
        return sum;
    }

    double sumBackXY(int pointCount){
        double sum = 0;
        dataNode* pointerNode = materialLast;
        for(int i = 0; (pointerNode != 0 && i < pointCount); pointerNode = pointerNode->previous){
            sum += pointerNode->time*pointerNode->intensity;
            i++;
        }
        return sum;
    }

    double countEnd(double accuracy, double maximum){
        int pointCount = 2;
        double base = 0;
        int materialLastPoint = 2;
        //cout << maximum*accuracy << endl;
        for(int i = pointCount; i < dataPointCount ;i++){
            base = (pointCount*sumBackXY(pointCount)-sumBackX(pointCount)*sumBackY(pointCount))/(pointCount*sumBackXX(pointCount)-sumBackX(pointCount)*sumBackX(pointCount));
            //cout << dataPointCount-pointCount << " " << base << endl;
            if (base<maximum*accuracy&&base>-maximum*accuracy) materialLastPoint = i;
            pointCount++;
        }
        base = (materialLastPoint*sumBackXY(materialLastPoint)-sumBackX(materialLastPoint)*sumBackY(materialLastPoint))/(materialLastPoint*sumBackXX(materialLastPoint)-sumBackX(materialLastPoint)*sumBackX(materialLastPoint));
        //cout << materialLastPoint << endl;
        dataNode* pointerNode = materialLast;
        for(int i = 0; (pointerNode != 0 && i < materialLastPoint); pointerNode = pointerNode->previous){
            i++;
        }
        twoExpFit->beamCalculation->xK = pointerNode->time;
        //cout << base << endl;
        return base;//(sumBackXX(pointCount)*sumBackY(pointCount)-sumBackX(pointCount)*sumBackXY(pointCount))/(pointCount*sumBackXX(pointCount)-sumBackX(pointCount)*sumBackX(pointCount));
    }

    void assignBeams(){
        //twoExpFit->beamCalculation->xK = materialLast->time;
        //twoExpFit->beamCalculation->yZero = countStart();
    }
    */

    // To save instrumental point data similar like for material data
    void addNodeInstrument(double makeTime, int makeIntensity){
        dataNode* creatingNode = new dataNode;
        creatingNode->time = makeTime;
        creatingNode->intensity = makeIntensity;
        if (instrumentPointCount == 0){
            instrumentalFirst = creatingNode;
        }
        else{
            creatingNode->previous = instrumentLast;
            instrumentLast->next = creatingNode;
        }
        instrumentPointCount++;
        instrumentLast = creatingNode;
    }

    // To save instrumental data in pairs
    void readFileInstrumental(char fileName[255]){
        ifstream inputFile(fileName);
        while (true){
                double timeFromFile;
                int photonCountFromFile;
                inputFile >> timeFromFile >> photonCountFromFile;
                addNodeInstrument(timeFromFile,photonCountFromFile);
                if( inputFile.eof() ) break;
        }
    }

    // To test and see data
    void printInstrumentData(){
        dataNode* findNode = instrumentalFirst;
        while (findNode != 0){
            cout << findNode->time << " " << findNode->intensity << endl;
            findNode = findNode->next;
        }
    }

    // To know where is the peak of material points in height (intensity scale)
    int findMaximumInstrumental(){
        dataNode* findNode = instrumentalFirst;
        int maximum = 0;
        while (findNode != 0){
            if (findNode->intensity>maximum)
                maximum = findNode->intensity;
            findNode = findNode->next;
        }
        return maximum;
    }

    // To center out both graphs with the maximums at time 0
    void allignInstrumentalToMaterial(){
        int maximum = findMaximumInstrumental();
        dataNode* centeringNode = instrumentalFirst;
        while (centeringNode != 0 && centeringNode->intensity != maximum){
            centeringNode = centeringNode->next;
        }
        double centerTime = centeringNode->time;
        centeringNode = instrumentalFirst;
        while (centeringNode != 0){
            centeringNode->time -= centerTime;
            centeringNode = centeringNode->next;
        }
    }

    // To remove data that is not in actual instrument impulse and is often only noise
    void clearInstrumental(){
        int maximum = findMaximumInstrumental();
        dataNode* middleNode = instrumentalFirst;
        while (middleNode != 0 && middleNode->intensity != maximum){
            middleNode = middleNode->next;
        }

        dataNode* findNode = middleNode;
        while (findNode != 0 && findNode->intensity > maximum*0.005){
            findNode = findNode->next;
        }

        instrumentLast = findNode;
        instrumentLast->next = 0;
        findNode = findNode->next;
        dataNode* deleteNode = 0;
        while (findNode != 0){
            deleteNode = findNode;
            findNode = findNode->next;
            delete deleteNode;
        }

        findNode = middleNode;
        while (findNode != 0 && findNode->intensity > maximum*0.005){
            findNode = findNode->previous;
        }

        instrumentalFirst = findNode;
        instrumentalFirst->previous = 0;
        findNode = findNode->previous;
        while (findNode != 0){
            deleteNode = findNode;
            findNode = findNode->previous;
            delete deleteNode;
        }
    }

    // To find best fitting deconvolution function using least squares method for goodness factor
    void deconvoluteData(){
        double bestGoodness = -1;
        double bestCombination[8] = {0,0,0,0};
        double coefficientStart = 0;
        double coefficientEnd = materialFirst->intensity/findMaximumInstrumental();
        double tauCoefficientStart = 0;
        double tauCoefficientEnd = 1;
        double noiseStart = 0;
        double noiseEnd = materialFirst->intensity;
        double offsetStart = 0;
        double offsetEnd = 0.1;
        int sectorCount = 3;
        for(int i=0; i<70 && (noiseEnd-noiseStart+offsetEnd-offsetStart+tauCoefficientEnd-tauCoefficientStart+coefficientEnd-coefficientStart)>accuracy; i++){
            cout << i << " " ;
            for(int sectorCoefficient = 1; sectorCoefficient<=sectorCount; sectorCoefficient++){
                for(int sectorTau = 1; sectorTau<=sectorCount; sectorTau++){
                    for(int sectorNoise = 0; sectorNoise<sectorCount; sectorNoise++){
                        for(int sectorOffset = 0; sectorOffset<sectorCount; sectorOffset++){
                            deconvoluteSumCounting(coefficientStart+sectorCoefficient*(coefficientEnd-coefficientStart)/sectorCount,tauCoefficientStart+sectorTau*(tauCoefficientEnd-tauCoefficientStart)/sectorCount,noiseStart+sectorNoise*(noiseEnd-noiseStart)/sectorCount,offsetStart+sectorOffset*(offsetEnd-offsetStart)/sectorCount);
                            if (deconvolutionGoodness()<=bestGoodness||bestGoodness==-1){
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
                            //cout << coefficientStart+(sectorCoefficient-1)*(coefficientEnd-coefficientStart)/2 << " " << coefficientStart+sectorCoefficient*(coefficientEnd-coefficientStart)/2 << " " << tauCoefficientStart+(sectorTau-1)*(tauCoefficientEnd-tauCoefficientStart)/2 << " " << tauCoefficientStart+sectorTau*(tauCoefficientEnd-tauCoefficientStart)/2 << endl;
                            //cout << deconvolutionGoodness() << endl;
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
            cout << deconvolutionGoodness() << endl;
        }
        cout << (bestCombination[0]+bestCombination[1])/2 << " // Tau - " << (bestCombination[2]+bestCombination[3])/2 << " with A0 - " << materialFirst->deconvolutionSum << " // " << (bestCombination[4]+bestCombination[5])/2 << " " << (bestCombination[6]+bestCombination[7])/2 << endl;
        fixedDeconvoluteSumCounting((bestCombination[0]+bestCombination[1])/2,(bestCombination[2]+bestCombination[3])/2,(bestCombination[4]+bestCombination[5])/2,(bestCombination[6]+bestCombination[7])/2);
    }

    // To finds best fitting deconvolution function it needs to count for each material point how the function look in the end after adding up exponents
    void deconvoluteSumCounting(double coefficient, double tauCoefficient, double noise, double timeOffset){
        dataNode* materialNode = materialFirst;
        dataNode* instrumentNode = instrumentalFirst;
        while (materialNode != 0){ // Have to clear out from data since that memory region is reused and will use +=
            materialNode->deconvolutionSum = 0;
            materialNode = materialNode->next;
        }
        materialNode = materialFirst;
        while (materialNode != 0){ //To find how good is the function it is important to go through
            instrumentNode = instrumentalFirst;
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

    // To count last function but experimental data
    void fixedDeconvoluteSumCounting(double coefficient, double tauCoefficient, double noise, double timeOffset){
        dataNode* materialNode = materialFirst;
        dataNode* instrumentNode = instrumentalFirst;
        while (materialNode != 0){
            materialNode->deconvolutionSum = 0;
            materialNode = materialNode->next;
        }
        materialNode = materialFirst;
        while (materialNode != 0){
            instrumentNode = instrumentalFirst;
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

    // To show end calculated data
    void printDeconvolution(){
        dataNode* findNode = materialFirst;
        while(findNode !=0){
            cout << findNode->time << " " << findNode->deconvolutionSum << endl;
            findNode = findNode->next;
        }
    }

    // To compare different variables and see how good is the fit
    float deconvolutionGoodness(){
        double holderGood = 0;
        dataNode* pointerNode = materialFirst;
        while (pointerNode != 0){
            holderGood += pow( pointerNode->deconvolutionSum - pointerNode->intensity, 2  );
            // cout << pointerNode->intensity << " " << zeroA * exp(-(pointerNode->time / tau)) << endl;
            pointerNode = pointerNode->next;
        }
        return holderGood/dataPointCount;
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
        - Clear till materialFirst maximum [3]
        - Change time scale so the time starts with 0 [4]
    Data can be analyzed
        - Fitting Exp to data [5]
        - Fitting 2 exponents to data [10]
    Data can be written to file
        - Exp fitted to data [6] from selected folder [8]
        - Difference from Exp and read data [7] from selected folder [9]
    Stop analyzing data [0]
    */
    cout << "End work [0], Assign reading file [1], Read file [2], Clear till materialFirst Maximum [3]," << endl;
    cout << "Move graph time to 0 [4], Fit exp [5], Assign exp output file [6]," << endl;
    cout << "Assign difference output file [7] Write exp to file [8] Write difference to file [9]" << endl;

    int command;
    linkedList* testList = new linkedList;
    char nameOfInputFile[255] = "dataTwo.txt";
    char nameOfInputFileInstrument[255] = "InstrumentData.txt";
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
                cout << "Data cleared till materialFirst maximum" << endl;
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
                //testList->countStart();
                //testList->countEnd(0.08, testList->twoExpFit->beamCalculation->yZero);
                //testList->twoExpFit->beamCalculation->print();

                cout << " Tau0 - " << testList->twoExpFit->tauZero << " A0 - " << testList->twoExpFit->aZero << " Tau1 - " << testList->twoExpFit->tauOne << " A1 - " << testList->twoExpFit->aOne << " Y1 - " << testList->twoExpFit->yZero << " Goodness - " << (testList->countGoodness(testList->twoExpFit->tauZero, testList->twoExpFit->aZero, testList->twoExpFit->tauOne, testList->twoExpFit->aOne, testList->twoExpFit->yZero) / testList->dataPointCount) << endl;
                //testOneExp->setupData(testList);
                //cout << "Exponents fitted to graph - " << testOneExp->aZero << endl;
                break;
            }
        case 11 :
            {
                testList->readFileInstrumental(nameOfInputFileInstrument);
                testList->clearInstrumental();
                testList->allignInstrumentalToMaterial();
                //testList->printInstrumentData();
                testList->deconvoluteData();
                testList->printDeconvolution();
            }
        }
    }
    return 0;
}

