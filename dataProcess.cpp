#include "dataProcess.h"


void writeRecentDataToFile(char* filename, bool pickedDot, int pickedTestConditionIndex, double reactionTime){

	bool actualDot, slant, wobble, mirror, amplitude, specularOnly, shininess, diffuseOnly, textured, lightPosition, stereo;


	std::ofstream outputFile (filename, std::ios::out|std::ios::app);

	if (outputFile.is_open()){
		setTestConditions(pickedTestConditionIndex, &actualDot, &slant, &wobble, &mirror, &amplitude, &specularOnly, &shininess, &diffuseOnly, &textured, &lightPosition);
		outputFile << pickedDot;
		outputFile << " ";
		outputFile << actualDot;
		outputFile << " ";
		outputFile << slant;
		outputFile << " ";
		outputFile << wobble;
		outputFile << " ";
		outputFile << mirror;
		outputFile << " ";
		outputFile << amplitude;
		outputFile << " ";
		outputFile << specularOnly;
		outputFile << " ";
		outputFile << shininess;
		outputFile << " ";
		outputFile << diffuseOnly;
		outputFile << " ";
		outputFile << textured;
		outputFile << " ";
		outputFile << lightPosition;
		outputFile << " ";
		outputFile << reactionTime;
		outputFile << "\n";
	}
	outputFile.close();

}

void setTestConditions(int testConditionIndex, bool * hill, bool* slant, bool* wobble, bool* mirror, bool* amplitude, bool* specularOnly, bool* shininess, bool* diffuseOnly, bool* textured, bool* lightPosition){

	// set up the next test conditions, using the following index conversion:
	// hill: 0
	// slant: 1
	// wobble: 2
	// mirror: 4
	// amplitude:5

	*hill = (bool)(testConditionIndex % 2);
	*slant = (bool)((testConditionIndex/2) % 2);
	*wobble = (bool)((testConditionIndex/4) % 2);
	*mirror = (bool)((testConditionIndex/8) % 2);
	*amplitude = (bool)((testConditionIndex/16) % 2);
	*specularOnly = (bool)((testConditionIndex/32) % 2);
	*shininess = (bool)((testConditionIndex/64) % 2);
	*diffuseOnly = (bool)((testConditionIndex/128) % 2);
	*textured = (bool)((testConditionIndex/256) % 2);
	*lightPosition = (bool)((testConditionIndex/512) % 2);
}


bool validTestConditions(bool hill, bool slant, bool wobble, bool mirror, bool amplitude, bool specularOnly, bool shininess, bool diffuseOnly, bool textured, bool lightPosition){

//	hill, slant, wobble, mirror, amplitude, specularOnly, shininess, diffuseOnly, textured, lightPosition


	return  amplitude && !shininess && !textured && slant && lightPosition && 
		((mirror && !specularOnly && !diffuseOnly) || (!mirror &&  ((diffuseOnly && !specularOnly) || (!diffuseOnly && specularOnly) || (!diffuseOnly && !specularOnly)))); //(!diffuseOnly || !specularOnly)));
	
	/*
	return  ((lightPosition && slant && amplitude && lightPosition) && ((diffuseOnly && !shininess && !specularOnly && !textured && !mirror) || (!diffuseOnly && !textured && !mirror) || 
		 (textured && !diffuseOnly && !shininess && !specularOnly && !mirror) || (mirror && !textured && !diffuseOnly && !shininess && !specularOnly))) ;
//		 || (!lightPosition && !slant && !mirror && amplitude && specularOnly && !diffuseOnly && !textured); // varies on shininess and wobble given inverted slant/lighting and specularOnly
	*/
}