// RunMipsInstructions.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define _CRT_SECURE_NO_WARNINGS

// compile by using: cl /EHsc /W4 /D_DEBUG /MDd debug_new.cpp
#define _CRTDBG_MAP_ALLOC  //定义宏，用于检测C、C++代码中的内存泄漏错误
#include <cstdlib>
#include <crtdbg.h>

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#else
#define DBG_NEW new
#endif

#include <iostream>
#include "Computer.h"
using namespace std;

#define FALSE false
#define TRUE true

int main(int argc, char* argv[]) 
{
	int argIndex;
	int printingRegisters = FALSE;
	int printingMemory = FALSE;
	int debugging = FALSE;
	int interactive = FALSE;
	FILE* filein;
	

	if (argc < 2) {
		fprintf(stderr, "Not enough arguments.\n");
		return 1;
	}

	for (argIndex = 1; argIndex < argc && argv[argIndex][0] == '-'; argIndex++) {
		/* Argument is an option, we hope one of -r, -m, -i, -d. */
		switch (argv[argIndex][1]) {
		case 'r':
			printingRegisters = TRUE;
			break;
		case 'm':
			printingMemory = TRUE;
			break;
		case 'i':
			interactive = TRUE;
			break;
		case 'd':
			debugging = TRUE;
			break;
		default:
			fprintf(stderr, "Invalid option \"%s\".\n", argv[argIndex]);//stderr标准错误流的常量指针，因此不能被赋值
			fprintf(stderr, "Correct options are -r, -m, -i, -d.\n");
			return 1;
		}
	}

	if (argIndex == argc) {
		fprintf(stderr, "No file name given.\n");
		return 1;
	}
	else if (argIndex < argc - 1) {
		fprintf(stderr, "Too many arguments.\n");
		return 1;
	}

	filein = fopen(argv[argIndex], "rb");
	if (filein == NULL) {
		fprintf(stderr, "Can't open file: %s\n", argv[argIndex]);
		return 1;
	}

	SimMipsComputer computer(filein, printingRegisters, printingMemory, debugging, interactive);
	computer.Simulate();

	_CrtDumpMemoryLeaks();//上面有这个 _CRTDBG_MAP_ALLOC  //定义宏，用于检测C、C++代码中的内存泄漏错误
	system("pause");
	return 0;
}
