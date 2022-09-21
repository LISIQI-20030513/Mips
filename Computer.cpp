#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include "Computer.h"

using namespace std;

char g_str_instruction[50];//这个是用来干什么的？？？全局变量？？？

/*
 *  Return an initialized computer with the stack pointer set to the
 *  address of the end of data memory, the remaining registers initialized
 *  to zero, and the instructions read from the given file.
 *  The other arguments all govern how the program interacts with the user.
 */
//SimMipsComputer构造函数
SimMipsComputer::SimMipsComputer(FILE* filein, int printingRegisters, 
	int printingMemory, int debugging, int interactive) 
{
	memset(&mipsComupter, 0, sizeof(SimulatedComputer));//初始化函数，可将一段连续的内存初始化为某个值，且是以字节为单位进行初始化的
	                                                    //一般格式为：memset(首地址,value,sizeof(地址总大小))
	int i;
	unsigned int instr;   //instr是机器二进制码，将机器码读入"内存"

	for (i = 0; i < 32; i++) 
		mipsComupter.registers[i] = 0;//其他所有寄存器初始化为 0x00000000
	
	/* stack pointer starts at end of data area and grows down */
	mipsComupter.registers[29] = 0x00400000 + (MAX_NUM_INSTRS + MAX_NUM_DATA) * 4;
	//栈指针初始化为 0x00404000

	i = 0;
	while (1)
	{
		instr = 0;
		unsigned char temp = 0;
		int k;
		for (k = 0; k < 4; k++)
		{
			if (!fread(&temp, 1, 1, filein))         //fread 函数用于从文件流中读取数据  fread(void*buffer,size,count,FILE*stream)
													 //buffer为接收数据的地址，即数据保存的地址，size是读出内容的单字节数，
													 // count读出size字节的数据项的个数，stream为目标文件指针                                   
			{
				i = -1;
				break;
			}
			instr |= temp << (k << 3);  //但是这是用来干啥的
			//按位 OR 运算,并将结果赋值给instr，在这里由于前面的instr是0，这也就相当于赋值运算了
		}
		if (i == -1) break;
		mipsComupter.memory[i] = instr;
		i++;
	}//这里我修改了while的大括号的位置
		if (i > MAX_NUM_INSTRS) 
		{
			//fprintf函数打印输出到文件
			fprintf(stderr, "Program too big (%d instructions, only %d allowed).\n", i, MAX_NUM_INSTRS);
			exit(1);
		}
	

	mipsComupter.printingRegisters = printingRegisters;
	mipsComupter.printingMemory = printingMemory;
	mipsComupter.interactive = interactive;
	mipsComupter.debugging = debugging;
}


/*
 *  Return the contents of memory at the given address.
 */
unsigned int SimMipsComputer::GetMemoryContents(int addr) 
{
	int index = (addr - 0x00400000) / 4;
	if (((addr & 0x3) != 0) || (index < 0) 
		|| (index >= (MAX_NUM_INSTRS + MAX_NUM_DATA))) 
	{
		printf("Invalid Address: %8.8x", addr);
		exit(0);
	}

	return mipsComupter.memory[index];
}

/*
 *  Print relevant information about the state of the computer.
 *  changedReg is the index of the register changed by the instruction
 *  being simulated.  If changedReg is -1, no register was updated by
 *  the simulated instruction, i.e. it was a branch, a jump, or a store).
 *  changedMem is the address of the memory location changed by the
 *  simulated instruction.  For anything but a store, changedMem will be -1.
 *  Previously initialized flags indicate whether to print all the
 *  registers or just the one that changed, and whether to print
 *  all the nonzero memory or just the memory location that changed.
 */
void SimMipsComputer::PrintInfo(int changedReg, int changedMem) 
{
	int i, addr;
	printf("New pc = %8.8x\n", mipsComupter.pc);

	if (!mipsComupter.printingRegisters && changedReg == -1) 
		printf("No register was updated.\n");
	else if (!mipsComupter.printingRegisters) 
		printf("Updated r%d to %8.8x\n", changedReg, mipsComupter.registers[changedReg]);
	else 
	{
		for (i = 0; i < 32; i++) 
		{
			printf("r%2d: %8.8x  ", i, mipsComupter.registers[i]);
			if ((i + 1) % 4 == 0) printf("\n");
		}
	}

	if (!mipsComupter.printingMemory && changedMem == -1) 
		printf("No memory location was updated.\n");
	else if (!mipsComupter.printingMemory) 
		printf("Updated memory at address %8.8x to %8.8x\n", changedMem, GetMemoryContents(changedMem));
	else 
	{
		printf("Nonzero memory\n");
		printf("ADDR     CONTENTS\n");

		for (addr = 0x00400000 + (MAX_NUM_INSTRS << 2); //左移运算符表示MAX_NUM_INSTRS*2^2
			addr < 0x00400000 + ((MAX_NUM_INSTRS + MAX_NUM_DATA) << 2); 
			addr = addr + 4) 
		{
			if (GetMemoryContents(addr) != 0) 
				printf("%8.8x  %8.8x\n", addr, GetMemoryContents(addr));
		}
	}
	printf("\n");
}



/*
 *  Run the simulation.
 */
void SimMipsComputer::Simulate() 
{
	char s[10];  /* used for handling interactive input */
	unsigned int instr;
	int changedReg, changedMem;

	mipsComupter.pc = 0x00400000;
	while (1) 
	{
		if (mipsComupter.interactive) 
		{
			printf("> ");
			fgets(s, 10, stdin);
			if (s[0] == 'q') return;
		}

		instr = GetMemoryContents(mipsComupter.pc);
		printf("Executing instruction at %8.8x: %8.8x\n", mipsComupter.pc, instr);
		/*cout << "\t" <<*/ Disassemble(instr, mipsComupter.pc) ;
		SimulateInstr(instr, &changedReg, &changedMem);
		PrintInfo(changedReg, changedMem);
	}
}


/*
 *  Return a string containing the disassembled version of the given
 *  instruction.  You absolutely MUST follow the formatting instructions
 *  given in the assignment.  Note that the string your return should
 *  not include a tab at the beginning or a newline at the end since
 *  those are added in Simulate() function where this function is called.
 */


string SimMipsComputer::Disassemble(unsigned int instr, unsigned int pc) // 对机器语言指令进行反汇编，返回汇编语言指令字符串+
{
	// Your program must exit when an unsupported instruction is detected
	//if (/*instruction isn't supported */) exit (0); 

	unsigned int instr_ = instr;

	 unsigned int opcode= (instr_ >> 26);
	 unsigned int rs = (instr_ <<6)>>27;
	 unsigned int rt= (instr_ <<11)>>27;
	 unsigned int rd=(instr_ <<16)>>27;
	 unsigned int shamt=(instr_ <<21)>>27;

	 unsigned int func_ = 63;
	 unsigned int func = instr & func_;
	
	 int imm = (instr_ << 16) >> 16;
	 int imm_ = imm;//用于andi和ori的立即数，由于andi和ori是无符号扩展不是有符号扩展，所以与其他的要进行区分
	 int imm_jugde = (instr<<16)>>31;
	 if (imm_jugde != 0)
	 {
		 imm = imm | 0xffff0000;
	 }

	/* int imm_Array[16];
	 for (int i = 0; i < 16; i++)
	 {
		 imm_Array[i] = (instr<<(16+i))>>31;
	 }
	 if (imm_Array[0] == 1)
	 {
		  imm = 0;
		 for (int i = 0; i < 16; i++)
		 {
			 if(imm_Array[i]==1)
			 imm += pow(2, 15-i);
		 }
		 imm = ~(imm - 1);
	 }*/
	
	 unsigned int beq_add = pc+4+(4*imm)
		 ;
	 unsigned int address = (instr_ << 6) >> 4 | (pc&0xf0000000);

		if (opcode == 0)//R格式
		{
				if (func == 32)
				{
					cout << "\tadd\t$" << rd << ",$" << rs << ",$" << rt << endl;
				}
				else if (func == 33)
				{
					cout << "\taddu\t$" << rd << ",$" << rs << ",$" << rt << endl;
				}
				else if (func == 34)

					cout << "\tsub\t$" << rd << ",$" << rs << ",$" << rt << endl;
				else if (func == 35)
					cout << "\tsubu\t$" << rd << ",$" << rs << ",$" << rt << endl;
				else if (func == 36)
					cout << "\tand\t$" << rd << ",$" << rs << ",$" << rt << endl;
				else if (func == 37)
					cout << "\tor\t$" << rd << ",$" << rs << ",$" << rt << endl;
				else if (func == 38)
					cout << "\txor\t$" << rd << ",$" << rs << ",$" << rt << endl;
				else if (func == 39)
					cout << "\tnor\t$" << rd << ",$" << rs << ",$" << rt << endl;
				else if (func == 42)
					cout << "\tslt\t$" << rd << ",$" << rs << ",$" << rt << endl;
				else if (func == 43)
					cout << "\tsltu\t$" << rd << ",$" << rs << ",$" << rt << endl;
				else if (func == 4)
					cout << "\tsllv\t$" << rd << ",$" << rs << ",$" << rt << endl;
				else if (func == 6)
					cout << "\tsrlv\t$" << rd << ",$" << rs << ",$" << rt << endl;
				else if (func == 7)
					cout << "\tsrav\t$" << rd << ",$" << rs << ",$" << rt << endl;
				else if (func == 8)
					cout << "\tjr\t$" << rs << endl;
			    else if(rs==0)
			    {
					if (func == 0 && shamt != 0&&rt!=0&&rd!=0)
						cout << "\tsll\t$" << rd << ",$" << rt << "," << shamt << endl;
					else if (func == 2)
						cout << "\tsrl\t$" << rd << ",$" << rt << "," << shamt << endl;
					else if (func == 3)
						cout << "\tsra\t$" << rd << ",$" << rt << "," << shamt << endl;
					else
						exit(0);
			    }
		}
		else //I格式
		{
			if (opcode == 8)
				cout << "\taddi\t$" << rt << ",$" << rs << "," << imm << endl;
			else if (opcode == 9)
				cout << "\taddiu\t$" << rt << ",$" << rs << "," << imm << endl;
			else if (opcode == 12)
				printf("\t%s\t$%u,$%u,0x%x\n", "andi", rt, rs, imm_);
			else if (opcode == 13)
				printf("\t%s\t$%u,$%u,0x%x\n", "ori", rt, rs, imm_);
			else if (opcode == 14)
				printf("\t%s\t$%u,$%u,0x%x\n", "xori", rt, rs, imm_);
			else if (opcode == 15)
				printf("\t%s\t$%u,0x%x\n", "lui", rt, imm);
			else if (opcode == 35)
				cout << "\tlw\t$" << rt << "," << imm << "(" << "$"<<rs << ")" << endl;
			else if (opcode == 43)
				cout << "\tsw\t$" << rt << "," << imm << "(" << "$" << rs << ")" << endl;
			else if (opcode == 4)
				printf("\t%s\t$%u,$%u,0x%x\n","beq",rs,rt, beq_add);
			else if (opcode == 5)
				printf("\t%s\t$%u,$%u,0x%x\n", "bne", rs, rt, beq_add);
			else if (opcode == 10)
				cout << "\tslti\t$" << rt << ",$" << rs << "," << imm << endl;
			else if (opcode == 11)
				cout << "\tsltiu\t$" << rt << ",$" << rs << "," << imm << endl;
			else if (opcode == 2)
				printf("\t%s\t0x%x\n","j",address);
			else if (opcode == 3)
                printf("\t%s\t0x%x\n", "jal", address);

			else
				exit(0);
		}
	

	return "";
}

/*
 *  Simulate the execution of the given instruction, updating the
 *  pc appropriately.
 *
 *  If the instruction modified a register--i.e. if it was lw,
 *  addu, addiu, subu, sll, srl, and, andi, or, ori, lui, or slt
 *  to list a few examples-- return the index of the modified
 *  register in *changedReg, otherwise return -1 in *changedReg.
 *  Note that you should never return 0 in *changedReg, since
 *  $0 cannot be changed!  Note that even if the instruction
 *  changes the register back to it's old value
 *  (e.g. addu $3, $3, $0) the destination register ($3 in the
 *  example) should be marked changed!
 *
 *  If the instruction was sw, return the address of the
 *  updated memory location in *changedMem, otherwise return -1
 *  in *changedMem.
 */

 // 模拟一条MIPS指令的执行
void SimMipsComputer::SimulateInstr(unsigned int instr, int* changedReg, int* changedMem) {
	/* You replace this code by the right stuff. */
	mipsComupter.pc = mipsComupter.pc + 4;
	*changedReg = -1;
	*changedMem = -1;

	unsigned int opcode = (instr >> 26);
	unsigned int rs = (instr<< 6) >> 27;
	unsigned int rt = (instr << 11) >> 27;
	unsigned int rd = (instr << 16) >> 27;
	unsigned int shamt = (instr << 21) >> 27;

	unsigned int func_ = 63;
	unsigned int func = instr & func_;
	
	int imm = (instr << 16) >> 16;
	int imm_ = imm;//用于andi和ori的立即数，由于andi和ori是无符号扩展不是有符号扩展，所以与其他的要进行区分,这个函数里可能用不到
	int imm_jugde = (instr << 16) >> 31;
	if (imm_jugde != 0)
	{
		imm = imm | 0xffff0000;
	}
	
	unsigned int address = (instr << 6) >> 4 | (mipsComupter.pc & 0xf0000000);

	
	if (opcode == 0)
	{

		if (func == 32 || func == 33)//add,addu
		{
			if (rt == 0)
			{
				*changedReg = -1;
			}
			else 
			{
				*changedReg = rd;
				mipsComupter.registers[*changedReg] = mipsComupter.registers[rt] + mipsComupter.registers[rs];
			}
		}

		if (func == 34 || func == 35)//sub,subu
		{
			*changedReg = rd;
			mipsComupter.registers[*changedReg] = mipsComupter.registers[rs] - mipsComupter.registers[rt];
		}

		if (func == 36)//and
		{
			*changedReg = rd;
			mipsComupter.registers[*changedReg] = mipsComupter.registers[rs] & mipsComupter.registers[rt];
		}

		if (func == 37)//or
		{
			*changedReg = rd;
			mipsComupter.registers[*changedReg] = mipsComupter.registers[rs] | mipsComupter.registers[rt];
		}

		if (func == 38)//xor
		{
			*changedReg = rd;
			mipsComupter.registers[*changedReg] = mipsComupter.registers[rs] ^ mipsComupter.registers[rt];
		}

		if (func == 39)//nor
		{
			*changedReg = rd;
			mipsComupter.registers[*changedReg] = ~(mipsComupter.registers[rs] | mipsComupter.registers[rt]);
		}

		if (func == 42 || func == 43)//slt,sltu
		{
			*changedReg = rd;
			if (mipsComupter.registers[rs] < mipsComupter.registers[rt])
				mipsComupter.registers[rd] = 1;
			else
				mipsComupter.registers[rd] = 0;
		}

		if (func == 0)//sll
		{
			*changedReg = rd;
			mipsComupter.registers[rd] = mipsComupter.registers[rt] << 10;
		}

		if (func == 2|| func == 3)//srl,sra
		{
			*changedReg = rd;
			mipsComupter.registers[rd] = mipsComupter.registers[rt] >> 10;
		}

		if (func == 4)//sllv
		{
			*changedReg = rd;
			mipsComupter.registers[rd] = mipsComupter.registers[rs] << mipsComupter.registers[rt];
		}

		if (func == 6 || func == 7)//srlv,srav
		{
			*changedReg = rd;
			mipsComupter.registers[rd] = mipsComupter.registers[rs] >> mipsComupter.registers[rt];
		}

		if (func == 8)//jr
		{
			mipsComupter.pc = mipsComupter.registers[rs];
		}

	}
	else if (opcode == 8||opcode==9)//addi,addiu
	{
		if (rt == 0)
		{
			*changedReg = -1;
		}
		else
		{
			*changedReg = rt;
			mipsComupter.registers[rt] = mipsComupter.registers[rs] + imm;
		}
	}
	else if (opcode == 12)//andi
	{
		*changedReg = rt;
		mipsComupter.registers[rt] = mipsComupter.registers[rs] & imm;
	}
	else if (opcode == 13)//ori
	{
		*changedReg = rt;
		mipsComupter.registers[rt] = mipsComupter.registers[rs] | imm;
	}
	else if (opcode == 14)//xori
	{
		*changedReg = rt;
		mipsComupter.registers[rt] = mipsComupter.registers[rs] ^ 10;
	}
	else if (opcode == 15)//lui
	{
		*changedReg = rt;
		mipsComupter.registers[rt] = imm*65536;
	}
	else if (opcode == 10||opcode==11)//slti,sltiu
	{
		*changedReg = rt;
		if (mipsComupter.registers[rs] < imm)
			mipsComupter.registers[rt] = 1;
		else 
			mipsComupter.registers[rt] = 0;
	}
	else if (opcode == 35)//lw
	{
	*changedReg = rt;
	 int addr = mipsComupter.registers[rs] + imm;//保存地址的寄存器
	mipsComupter.registers[rt] = GetMemoryContents(addr);
	
     }
	else if (opcode == 43)//sw
	{
	 int addr= mipsComupter.registers[rs]+imm;
	*changedMem =addr;
	//相对地址
	int con_addr=(addr- 0x00400000) / 4;
	mipsComupter.memory[con_addr] = mipsComupter.registers[rt];
     }
	else if (opcode==4)//beq
	{
	if(mipsComupter.registers[rs] == mipsComupter.registers[rt])
	mipsComupter.pc += imm * 4;
    }
	else if (opcode == 5)//bne
	{
	if (mipsComupter.registers[rs] != mipsComupter.registers[rt])
		mipsComupter.pc += imm * 4;
    }
	else if (opcode == 2)//J格式
	{
	mipsComupter.pc = address;
     }
	else if (opcode == 3)//jal格式
	{
	*changedReg = 31;
	mipsComupter.registers[*changedReg] = mipsComupter.pc;
	mipsComupter.pc = address;
	return;
    }
	else
	exit(0);

}

