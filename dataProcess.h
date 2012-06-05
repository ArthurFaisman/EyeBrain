#ifndef DATA_PROCESS_FUNCTIONS_H
#define DATA_PROCESS_FUNCTIONS_H

#include <fstream>
#include <iostream>

#define NUMBER_OF_TEST_CONDITIONS 10

void writeRecentDataToFile(char* filename, bool pickedDot, int pickedTestConditionIndex, double reactionTime);

void setTestConditions(int testCondition, bool * hill, bool* slant, bool* wobble, bool* mirror, bool* amplitude, bool* specularOnly, bool* shininess, bool* diffuseOnly, bool* textured, bool* lightPosition);
bool validTestConditions(bool hill, bool slant, bool wobble, bool mirror, bool amplitude, bool specularOnly, bool shininess, bool diffuseOnly, bool textured, bool lightPosition);

#endif