#include "../includes/DataGenerator.h"
#include <fstream>
#include <malloc.h>
#include <string.h>

using namespace std;


DataGenerator::DataGenerator(void)
{
}


DataGenerator::~DataGenerator(void)
{
}



/************************************************************************/
/* ● black: 0
/* ○ white: 1
/* _  empty: 2
/* Δ boundary: 3
/* 2 bits => one position
/************************************************************************/
int templateArray[MAX_TYPE*2][4] = {
	//for black
	0,-1,-1,-1,//●●●●●
	//200002,-1,-1,-1,//_●●●●_, active4 can be replaced by 2 sleep4
	2,-1,-1,-1,//●●●●_
	20,-1,-1,-1,//●●●_●
	200,-1,-1,-1,//●●_●●
	2000,-1,-1,-1,//●_●●●
	20000,-1,-1,-1,//_●●●●
	220002,-1,-1,-1,//__●●●_
	200022,-1,-1,-1,//_●●●__
	202002,-1,-1,-1,//_●_●●_
	200202,-1,-1,-1,//_●●_●_
	1200021,1200023,3200021,3200023,//○_●●●_○,○_●●●_Δ,Δ_●●●_○,Δ_●●●_Δ
	102002,302002,-1,-1,//○●_●●_,Δ●_●●_
	100202,300202,-1,-1,//○●●_●_,Δ●●_●_
	100022,300022,-1,-1,//○●●●__,Δ●●●__
	220001,220003,-1,-1,//__●●●○,__●●●Δ
	202001,202003,-1,-1,//_●_●●○,_●_●●Δ
	200201,200203,-1,-1,//_●●_●○,_●●_●Δ
	220,-1,-1,-1,//●●__●
	2200,-1,-1,-1,//●__●●
	2020,-1,-1,-1,//●_●_●
	200222,-1,-1,-1,//_●●___
	202022,-1,-1,-1,//_●_●__
	220022,-1,-1,-1,//__●●__
	222002,-1,-1,-1,//___●●_
	220202,-1,-1,-1,//__●_●_
	202202,-1,-1,-1,//_●__●_
	1220021,1220023,3220021,3220023,//○__●●_○,○__●●_Δ,Δ__●●_○,Δ__●●_Δ
	1202021,1202023,3202021,3202023,//○_●_●_○,○_●_●_Δ,Δ_●_●_○,Δ_●_●_Δ
	1200221,1200223,3200221,3200223,//○_●●__○,○_●●__Δ,Δ_●●__○,Δ_●●__Δ
	102202,302202,-1,-1,//○●__●_,Δ●__●_
	102022,302022,-1,-1,//○●_●__,Δ●_●__
	100222,300222,-1,-1,//○●●___,Δ●●___
	222001,222003,-1,-1,//___●●○,___●●Δ
	220201,220203,-1,-1,//__●_●○,__●_●Δ
	202201,202203,-1,-1,//_●__●○,_●__●Δ
	2220,-1,-1,-1,//●___●

	/************************************************************************/
	/* active1 the correct type is the comment ones, it brings too many active1,
	/* so to make a trade off, use a simply active1 instead of these 4 kinds of active1                                                                      */
	/************************************************************************/
	2220222,-1,-1,-1,//___●___
	-1,-1,-1,-1,//
	-1,-1,-1,-1,//
	-1,-1,-1,-1,//

	// 202222,-1,-1,-1,//_●____
	// 220222,-1,-1,-1,//__●___
	// 222202,-1,-1,-1,//____●_
	// 222022,-1,-1,-1,//___●__

	100001,100003,300001,300003,//○●●●●○,○●●●●Δ,Δ●●●●○,Δ●●●●Δ

	//for white
	11111,-1,-1,-1,//○○○○○
	//211112,-1,-1,-1,//_○○○○_, active4 can be replaced by 2 sleep4
	11112,-1,-1,-1,//○○○○_
	11121,-1,-1,-1,//○○○_○
	11211,-1,-1,-1,//○○_○○
	12111,-1,-1,-1,//○_○○○
	21111,-1,-1,-1,//_○○○○
	221112,-1,-1,-1,//__○○○_
	211122,-1,-1,-1,//_○○○__
	212112,-1,-1,-1,//_○_○○_
	211212,-1,-1,-1,//_○○_○_
	211120,211123,3211120,3211123,//●_○○○_●,●_○○○_Δ,Δ_○○○_●,Δ_○○○_Δ
	12112,312112,-1,-1,//●○_○○_,Δ○_○○_
	11212,311212,-1,-1,//●○○_○_,Δ○○_○_
	11122,311122,-1,-1,//●○○○__,Δ○○○__
	221110,221113,-1,-1,//__○○○●,__○○○Δ
	212110,212113,-1,-1,//_○_○○●,_○_○○Δ
	211210,211213,-1,-1,//_○○_○●,_○○_○Δ
	11221,-1,-1,-1,//○○__○
	12211,-1,-1,-1,//○__○○
	12121,-1,-1,-1,//○_○_○
	211222,-1,-1,-1,//_○○___
	212122,-1,-1,-1,//_○_○__
	221122,-1,-1,-1,//__○○__
	222112,-1,-1,-1,//___○○_
	221212,-1,-1,-1,//__○_○_
	212212,-1,-1,-1,//_○__○_
	221120,221123,3221120,3221123,//●__○○_●,●__○○_Δ,Δ__○○_●,Δ__○○_Δ
	212120,212123,3212120,3212123,//●_○_○_●,●_○_○_Δ,Δ_○_○_●,Δ_○_○_Δ
	211220,211223,3211220,3211223,//●_○○__●,●_○○__Δ,Δ_○○__●,Δ_○○__Δ
	12212,312212,-1,-1,//●○__○_,Δ○__○_
	12122,312122,-1,-1,//●○_○__,Δ○_○__
	11222,311222,-1,-1,//●○○___,Δ○○___
	222110,222113,-1,-1,//___○○●,___○○Δ
	221210,221213,-1,-1,//__○_○●,__○_○Δ
	212210,212213,-1,-1,//_○__○●,_○__○Δ
	12221,-1,-1,-1,//○___○

	/************************************************************************/
	/* active1 the correct type is the comment ones, it brings too many active1,
	/* so to make a trade off, use a simply active1 instead of these 4 kinds of active1                                                                      */
	/************************************************************************/
	2221222,-1,-1,-1,//___○___
	-1,-1,-1,-1,//
	-1,-1,-1,-1,//
	-1,-1,-1,-1,//

	// 212222,-1,-1,-1,//_○____
	// 221222,-1,-1,-1,//__○___
	// 222212,-1,-1,-1,//____○_
	// 222122,-1,-1,-1,//___○__


	11110,11113,311110,311113//●○○○○●,●○○○○Δ,Δ○○○○●,Δ○○○○Δ
};

const int16 templateArrayLength[MAX_TYPE] = {
	5,5,5,5,5,5,6,6,6,6,7,6,6,6,6,6,6,5,5,5,6,6,6,6,6,6,7,7,7,6,6,6,6,6,6,5,/*6,6,6,6*/7,0,0,0,6
};

const int typeToEmptyPos[MAX_TYPE] = {
	-1,0,1,2,3,4,0x50400,0x50100,0x50300,0x50200,0x501,0x300,0x200,0x100,0x504,0x503,0x502,0x201,0x302,0x301,0x5020100,0x5030100,0x5040100,0x5040300,0x5040200,0x5030200,
	0x50401,0x50301,0x50201,0x30200,0x30100,0x20100,0x50403,0x50402,0x50302,0x30201,/*0x30201,0x40201,0x40302,0x40301*/0x5040201,-1,-1,-1,-1
};

const int NUM_BITS = 11;

const int NUM_SIZE = 1<<NUM_BITS*2;

inline bool DataGenerator::isEndWith(int32 suffix, int32 target,int suffixLength,int targetLength)
{
	if(targetLength>= suffixLength)
	{
		int32 mask = (1<<(suffixLength<<1)) - 1;

		return !((target ^ suffix) & mask);
	}
	return false;
}

int DataGenerator::getSkip(int32 target,int targetLength)
{
	int skip = 0;

	while(targetLength>0)
	{
		for (int type = 0;type<MAX_TYPE*2;type++)
		{
			int halfType = type<MAX_TYPE?type : type - MAX_TYPE;
			int templateLength = templateArrayLength[halfType];
			for (int k = 0;k<4;k++)
			{
				int templateType = templateArray[type][k];
				if(templateType == -1){
					break;
				}
				if(isEndWith(templateType,target,templateLength,targetLength) ||
					isEndWith(target,templateType,targetLength,templateLength))
				{
					return skip;
				}
			}

		}

		target>>=2;
		targetLength--;

		skip++;
	}

	return skip;
}

bool DataGenerator::isInitialized = false;

void DataGenerator::initial()
{
	convertTemplateArray();
	isInitialized = true;
}

void DataGenerator::convertTemplateArray()
{
	for (int i = 0;i<MAX_TYPE*2;i++)
	{
		for(int j = 0;j<4;j++)
		{
			if(templateArray[i][j] != -1)
			{
				int value = templateArray[i][j];

				int16 result = 0;
				int cnt = 0;
				while(value)
				{
					result |= ((value%10)&3 )<<(2*cnt);

					value = value/10;
					cnt++;
				}
				templateArray[i][j] = result;
			}
		}
	}
}

const char* FILE_NAME = "five.dat";

int16* DataGenerator::readDataFile()
{
	ifstream file(FILE_NAME,ios::binary | ios::in);

	if(file && file.is_open())
	{
//		int16* typeArray = (int16*)aligned_alloc(sizeof(int16)*NUM_SIZE, 128);

		int16* typeArray = (int16*)malloc(sizeof(int16)*NUM_SIZE);

		file.read((char*)typeArray,sizeof(int16) * NUM_SIZE);

		file.close();

		return typeArray;
	}
	else
	{
		return writeDataFile();
	}
}

int16* DataGenerator::writeDataFile()
{
	ofstream file(FILE_NAME,ios::binary);

	int16* typeArray = generateTypeArrayInternal();

	file.write((char*)typeArray, NUM_SIZE*sizeof(int16));

	file.close();

	return typeArray;
}

int16* DataGenerator::generateTypeArray()
{
	return readDataFile();
}

int16* DataGenerator::generateTypeArrayInternal()
{
	if (!isInitialized)
	{
		initial();
	}

	// fix a bug, after memalign, then do memset, it always failed
//	int16* typeArray = (int16*)memalign(sizeof(int16)*NUM_SIZE, 128);

	int16* typeArray = (int16*)malloc(sizeof(int16)*NUM_SIZE);

	memset(typeArray,0,NUM_SIZE*sizeof(int16));

	for (int i = 0;i<NUM_SIZE;i++)
	{
		for (int type = 0;type<MAX_TYPE*2;type++)
		{
			int halfType = type<MAX_TYPE?type : type - MAX_TYPE;
			int templateLength = templateArrayLength[halfType];
			int32 mask = (1<<(templateLength<<1)) - 1;

			for (int k = 0;k<4;k++)
			{
				int templateType = templateArray[type][k];
				if(templateType == -1){
					break;
				}

				if(!((i ^ templateType) & mask))
				{
					int color = type < MAX_TYPE? 0 : 1;
					int realType = halfType + 1;

					typeArray[i] = ((color&3)<<6) | (realType&63);

					goto next;
				}
			}
		}
next:
		int16 skip = 1+getSkip(i>>2,NUM_BITS - 1);
		typeArray[i] |= skip<<8;
	}

	return typeArray;
}

int32* DataGenerator::generateEmptyPosArray()
{
	if (!isInitialized)
	{
		initial();
	}

	//int32* emptyArray = (int32*)memalign(sizeof(int32)*(1<<16), 128);
	int32* emptyArray = (int32*)malloc(sizeof(int32)*(1<<16));

	memset(emptyArray,0,(1<<16)*sizeof(int32));

	for (int i = 0;i<(1<<16);i++)
	{
		int16 type = i&63;
		int8 direction = (i>>6)&3;
		int8 startPos = (i>>8)&0xFF;

		if(type>=1 && type<=MAX_TYPE)
		{
			int empty = typeToEmptyPos[type-1];

			if(empty == -1)
			{
				emptyArray[i] = -1;
			}
			else
			{
				//adjust start position, because x or y will be -1
				int8 x = RANK_X(startPos);
				int8 y = RANK_Y(startPos);

				switch(direction)
				{
				case DIRECTION_HORIZONTAL:
					if(x == 0xF)
					{
						startPos -= 16;
					}
					break;
				case DIRECTION_VERTICAL:
					if(y == 0xF)
					{
						startPos -= 16*16;
					}
					break;
				case DIRECTION_DIAGNOAL:
					if (x == 0xF)
					{
						startPos -= 16;
					}

					if(y == 0xF)
					{
						startPos -= 16*16;
					}
					break;
				case DIRECTION_ANTIDIAGNOAL:
					if(x == 0xF)
					{
						startPos -= 16;
					}
					break;
				default:
					break;
				}



				int8 last = 0;
				for (int k = 0;k<4;k++)
				{
					int8 posDiff = (empty>>(8*k))&0xFF;
					if(k > 0 && posDiff <= last)
					{
						break;
					}
					last = posDiff;

					// FIXME: there is a bug, pos will be negative, we can ignore it, because it will never happen in reality.
					int8 pos = (startPos + posDiff*directionDiff[direction]) & 0xFF;

					//to make low position has low value, it means from low position to high, the value is sorted from small to large
					if(direction == DIRECTION_ANTIDIAGNOAL)
					{
						emptyArray[i] = (emptyArray[i]<<8) | pos;
					}
					else
					{
						emptyArray[i] |= pos<<(8*k);
					}
				}
			}
		}
		else
		{
			emptyArray[i] = -1;
		}
	}

	return emptyArray;
}
