#ifndef _DATA_GENERATOR_
#define _DATA_GENERATOR_

#include "const.h"

class DataGenerator
{
public:
	DataGenerator(void);
	~DataGenerator(void);

	static int16* generateTypeArray();

	static int32* generateEmptyPosArray();

private:
	static bool isInitialized;
	static void initial();
	static bool isEndWith(int32 suffix, int32 target,int suffixLength,int targetLength);
	static int getSkip(int32 target,int targetLength);
	static void convertTemplateArray();
	static int16* generateTypeArrayInternal();
	static int16* readDataFile();
	static int16* writeDataFile();
};

#endif // _DATA_GENERATOR_
