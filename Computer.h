#pragma once

#include <string>
using namespace std;

#undef mips								/* gcc already has a def for mips */

const int MAX_NUM_INSTRS = 1024;		/* max # instrs in a program */
const int MAX_NUM_DATA   = 3072;		/* max # data words */


// 模拟计算机的基本硬件组成部分，包括32个通用寄存器
// 指令内存和数据内存，pc寄存器，
// 4个标记：是否打印寄存器，是否打印内存，是否交互运行，是否调试运行
struct SimulatedComputer {
	int memory[MAX_NUM_INSTRS + MAX_NUM_DATA];
	int registers[32];
	int pc;//保存将要执行的指令地址，根本上是一个内存指针
	int printingRegisters, printingMemory, interactive, debugging;
};


class SimMipsComputer
{
public:
	SimMipsComputer(FILE*, int printingRegisters, int printingMemory, 
		int debugging, int interactive);

	unsigned int GetMemoryContents(int addr);
	void PrintInfo(int changedReg, int changedMem);
	void Simulate();

	// 模拟一条MIPS指令的执行
	void SimulateInstr(unsigned int instr, int* changedReg, int* changedMem);
	// 对机器语言指令进行反汇编，返回汇编语言指令字符串
	string Disassemble(unsigned int instr, unsigned int pc);

private:
	SimulatedComputer mipsComupter;
};
