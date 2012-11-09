#include "geneticalgorithm.h"

#include "qglobal.h"
#include "qmath.h"

GeneticAlgorithm::GeneticAlgorithm() {
    maxScore.push_back(0);
    avgScore.push_back(0);
}

//public

float GeneticAlgorithm::getAvgScore(const int index) {
    return avgScore[index];
}

float GeneticAlgorithm::getAxleAngle(const int index) {
    return chromes[currentCar][START_WHEELS_GEN + index*3 + 1]*M_PI*2;
}

unsigned int GeneticAlgorithm::getCarCallListNuber() {
    return callLists[currentCar];
}

unsigned int GeneticAlgorithm::getCarParentCallListNumber(const int parent) {
    if (parent)
        return parentsCallLists[currentCar][1];
    return parentsCallLists[currentCar][0];
}

int GeneticAlgorithm::getCarNum() {
    return currentCar;
}

float GeneticAlgorithm::getCartAngle(const int index) {
    return cacheAngles[currentCar][index];
}

QColor GeneticAlgorithm::getColorAxle(const int index) {
    return getColor(index + 8);
}

QColor GeneticAlgorithm::getColorCart(const int index) {
    return getColor(index);
}

int GeneticAlgorithm::getGenerationNum() {
    return generationNum;
}

float GeneticAlgorithm::getMagnitude(const int index) {
    return cacheMagnitudes[currentCar][index];
}

float GeneticAlgorithm::getMaxScore(const int index) {
    return maxScore[index];
}

float GeneticAlgorithm::getScore(const int index) {
    if (generationNum && !currentCar)
        return scores[index];
    if (currentCar <= index)
        return -1;
    return scores[index];
}

int GeneticAlgorithm::getSpringCount(const int index) {
    return springsCount[index];
}

int GeneticAlgorithm::getTime(const int index) {
    return times[index];
}

int GeneticAlgorithm::getWheelOn(const int index) {
    float chrome = chromes[currentCar][START_WHEELS_GEN + index*3];
    if (chrome > WHEEL_PROB0)
        return -1;
    return int(chrome/WHEEL_PROB0*7+0.5);
}

float GeneticAlgorithm::getWheelRadius(const int index) {
    return chromes[currentCar][START_WHEELS_GEN + index*3 + 2]*
            (MAX_WHEEL - MIN_WHEEL) + MIN_WHEEL;
}

void GeneticAlgorithm::init() {
    for (int i = 0; i < POP_SIZE; i++) {
        for (int j = 0; j < CROMES_SIZE; j++) {
            chromes[i][j] = float(qrand())/float(RAND_MAX);
        }
        unsigned short int  red = qrand()%256;
        unsigned short int green = qrand()%256;
        unsigned short int blue = qrand()%256;
        for (int j = 0; j < 16; j++) {
            colors[i][j][RED] = red;
            colors[i][j][GREEN] = green;
            colors[i][j][BLUE] = blue;
        }
        springsCount[i] = 0;
        callLists[i] = 0;
        parentsCallLists[i][0] = 0;
        parentsCallLists[i][1] = 0;
    }
    createCache();
    currentCar = -1;
    generationNum = 0;
}

bool GeneticAlgorithm::nextCar() {
    currentCar++;
    if (currentCar >= POP_SIZE)
        return false;
    return true;
}

void GeneticAlgorithm::nextGenetation() {
    copyChromes();
    int max1 = 0;
    int max2 = 1;
    if (compareCar(scores[max2], times[max2], scores[max1], times[max1]))
        qSwap(max1, max2);
    float total = scores[0] + scores[0];
    for (int i = 2; i < POP_SIZE; i++) {
        total += scores[i];
        if (compareCar(scores[i], times[i], scores[max1], times[max1])) {
            qSwap(max1, max2);
            max1 = i;
        } else if (compareCar(scores[i], times[i], scores[max2], times[max2]))
            max2 = i;
    }
    maxScore.push_back(scores[max1]);
    avgScore.push_back(total/POP_SIZE);
    copyChrome(max1, 0);
    parentsCallLists[0][0] = callLists[max1];
    parentsCallLists[0][0] = 0;
    copyChrome(max2, 1);
    parentsCallLists[1][0] = callLists[max2];
    parentsCallLists[1][0] = 0;
    int winners[POP_SIZE/2];
    bool queue[POP_SIZE];
    for (int i = 0; i < POP_SIZE; i++) {
        queue[i] = true;
        springsCount[i] = 0;
    }
    for (int i = 0; i < POP_SIZE/2; i++) {
        int a = getRandomChrome(queue);
        int b = getRandomChrome(queue);      
        winners[i] = compareCar(scores[a], times[a], scores[b], times[b])? a: b;
        springsCount[winners[i]]++;
    }
    crossover(winners[0], winners[1], 2, 3);
    for (int i = 0; i < POP_SIZE; i++)
        queue[i] = true;
    for (int i = 2; i < POP_SIZE/2; i++) {
        int parentA = winners[i];
        int parentB = getRandomChrome(queue);
        crossover(parentA, parentB, i*2, i*2 + 1);
        springsCount[parentB]++;
    }
    mutation();
    createCache();
    generationNum++;
    currentCar = 0;
}

void GeneticAlgorithm::setCarCallList(const unsigned int callListNumber) {
    callLists[currentCar] = callListNumber;
}

void GeneticAlgorithm::setScoreAndTime(float score, float time) {
    scores[currentCar] = score;
    times[currentCar] = time;
}

//private

bool GeneticAlgorithm::compareCar(const float scoreA, const float timeA, const int scoreB,
                                  const float timeB) {
    if (scoreA > scoreB)
        return true;
    if (scoreB > scoreA)
        return false;
    if (timeA <= timeB)
        return true;
    return false;
}

void GeneticAlgorithm::copyChrome(const int parent, const int offspring) {
    for (int j = 0; j < CROMES_SIZE; j++) {
        chromes[offspring][j] = oldChromes[parent][j];
    }
    for (int j = 0; j < 16; j++) {
        for (int channel = 0; channel < 3; channel++)
        colors[offspring][j][channel] = oldColors[parent][j][channel];
    }
}

void GeneticAlgorithm::copyChromes() {
    for (int i = 0; i < POP_SIZE; i++) {
        for (int j = 0; j < CROMES_SIZE; j++) {
            oldChromes[i][j] = chromes[i][j];
        }
        for (int j = 0; j < 16; j++) {
            for (int channel = 0; channel < 3; channel++)
            oldColors[i][j][channel] = colors[i][j][channel];
        }
    }
}

void GeneticAlgorithm::createCache() {
    for (int i = 0; i < POP_SIZE; i++) {
        float angles[8];
        float angleSum = 0;
        for (int j = 0; j < 8; j++) {
            cacheMagnitudes[i][j] = chromes[i][j*2 + 1]*
                    (MAX_CART - MIN_CART) + MIN_CART;
            angles[j] = chromes[i][j*2]*(1 - MIN_ANGLE) + MIN_ANGLE;
            angleSum += angles[j];
        }
        float angle = 0;
        for (int j = 0; j < 8; j++) {
            cacheAngles[i][j] = angle + angles[j]/angleSum*M_PI*2;
            angle = cacheAngles[i][j];
        }
    }
}

void GeneticAlgorithm::crossover(const int parentA, const int parentB,
                                 const int offspringA, const int offspringB) {
    int bend0 = qrand()%CROMES_SIZE;
    int bend1 = qrand()%CROMES_SIZE;
    if (bend0 > bend1)
        qSwap(bend0, bend1);
    for (int i = 0; i < CROMES_SIZE; i++) {
        if (i >= bend0 && i  <= bend1) {
            chromes[offspringA][i] = oldChromes[parentB][i];
            chromes[offspringB][i] = oldChromes[parentA][i];
            setColors(parentB, offspringA, parentA, offspringB, i);
        } else {
            chromes[offspringA][i] = oldChromes[parentA][i];
            chromes[offspringB][i] = oldChromes[parentB][i];
            setColors(parentA, offspringA, parentB, offspringB, i);
        }
    }
    parentsCallLists[offspringA][0] = callLists[parentA];
    parentsCallLists[offspringA][1] = callLists[parentB];
    parentsCallLists[offspringB][0] = callLists[parentB];
    parentsCallLists[offspringB][1] = callLists[parentA];
}

QColor GeneticAlgorithm::getColor(const int index) {
    return QColor(colors[currentCar][index][RED],
                  colors[currentCar][index][GREEN],
                  colors[currentCar][index][BLUE]);
}

int GeneticAlgorithm::getRandomChrome(bool queue[], const int excluding) {
    int index = qrand()%POP_SIZE;
    while (!queue[index] || index == excluding) {
        index = (index + 1)%POP_SIZE;
    }
    queue[index] = false;
    return index;
}

void GeneticAlgorithm::mutation() {
    for (int i = 2; i < POP_SIZE; i++) {
        for (int j = 0; j < CROMES_SIZE; j++) {
            if (qrand()%100 < MUTATION_RATE) {
                chromes[i][j] =  float(qrand())/float(RAND_MAX);
                int colorIndex = j < 16? j/2: (j - 16)/3;
                for (int channel = 0; channel < 3; channel++) {
                    colors[i][colorIndex][channel] = qrand()%256;
                }
            }
        }

    }
}

void GeneticAlgorithm::setColors(const int parentA, const int offspringA,
                                 const int parentB, const int offspringB,
                                 const int index) {
    int i = -1;
    if (index < 16) {
        if (index%2 == 0)
            i = index/2;
    } else {
        if ((index - 16)%3 == 0)
            i = (index - 15)/3 + 8;
    }
    if (i < 0)
        return;
    for (int channel = 0; channel < 3; channel++) {
        colors[offspringA][i][channel] = oldColors[parentA][i][channel];
        colors[offspringB][i][channel] = oldColors[parentB][i][channel];
    }
}
