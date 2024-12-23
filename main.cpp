#include <iostream>
#include <stdio.h>
#include <stdlib.h>


using Byte = unsigned char;
using Word = unsigned short;

using u32 = unsigned int;
struct Mem{
    static constexpr u32 MAX_MEM = 1024*64;
    Byte Data[MAX_MEM];

    void Initialise(){
        for (u32 i = 0; i < MAX_MEM; i++){
            Data[i] = 0;
        }
    }
    // Read 1 byte
    Byte operator[](u32 Address) const{
        return Data[Address];
    }
    Byte& operator[](u32 Address){
        return Data[Address];
    }

    //2 bytes
    void WriteWord( Word Value, u32 Address,u32 Cycles){
        // Little endian
        Data[Address] = Value & 0xFF;
        Data[Address + 1] = (Value >> 8);
        Cycles -=2;
    }
};
struct CPU{


    Word PC; // Program Counter
    Word SP; // Stack Pointer

    Byte A, X, Y; // Registers (Acumulator, Index, Index)

    Byte C: 1; // Processor Status
    Byte Z: 1; // Zero
    Byte I: 1; // Disable Interrupts
    Byte D: 1; // Decimal Mode
    Byte B: 1; // Break
    Byte V: 1; // Overflow
    Byte N: 1; // Negative

    void Reset(Mem& memory){
        PC = 0xFFFC;
        SP = 0x0100;
        C = Z = I = D = B = V = N = 0;
        A = X = Y = 0;
        memory.Initialise();

    }

    Byte FetchByte(u32& Cycles, Mem& memory){
        Byte Data = memory[PC];
        PC++;
        Cycles--;
        return Data;
    }

    Word FetchWord(u32& Cycles, Mem& memory){
        //Little endian
        Word Data = memory[PC];
        PC++;
        Cycles--;

        Data |= (memory[PC] << 8);
        PC++;
        Cycles -=  2;
        return Data;

        // Big endian
    }

    Byte ReadByte(u32& Cycles, Byte Address, Mem& memory){
        Byte Data = memory[Address];
        Cycles--;
        return Data;
    }

    //opcodes
    static constexpr Byte
        INS_LDA_IM = 0xA9,
        INS_LDA_ZP = 0xA5,
        INS_LDA_ZPX = 0xB5,
        INS_JSR = 0x20;

    void LDASetStatus(){
        Z = A == 0;
        N = (A & 0b10000000) > 0;
    }

    void Execute( u32 Cycles, Mem& memory){
        while (Cycles > 0){
            Byte Ins = FetchByte(Cycles, memory);
            switch (Ins){

                case INS_LDA_IM:{ // 2 cycles
                    Byte Value = FetchByte(Cycles, memory);
                    A = Value;
                    LDASetStatus(); break;
                }

                case INS_LDA_ZP:{ // 3 cycles
                    Byte ZeroPageAddress = FetchByte(Cycles, memory);
                    A = ReadByte(Cycles, ZeroPageAddress, memory);
                    LDASetStatus(); break;
                }

                case INS_LDA_ZPX:{ // 4 cycles
                    Byte ZeroPageAddress = FetchByte(Cycles, memory);
                    ZeroPageAddress += X;
                    Cycles--;
                    A = ReadByte(Cycles, ZeroPageAddress, memory);
                    LDASetStatus(); break;
                }

                case INS_JSR:{ // 6 cycles
                    Word SubAdress = FetchWord(Cycles, memory);
                    SP -= 2;
                    memory.WriteWord(PC - 1, SP, Cycles);
                    PC = SubAdress;
                    Cycles -= 2;
                    break;
                }

                default:{
                    printf("Instruction not handled %d\n", Ins); break;
                }
            }
        }
    }
};


int main(){
    Mem mem;
    CPU cpu;
    cpu.Reset(mem);

    // start test instructions
    mem[0xFFFC] = CPU::INS_JSR; // Jump to Subroutine
    mem[0xFFFD] = 0x42; // Low byte of subroutine address
    mem[0xFFFE] = 0x00; // High byte of subroutine address
    mem[0x0042] = CPU::INS_LDA_IM; // Load Immediate
    mem[0x0043] = 0x84; // Value to load
    // end test instructions
    cpu.Execute(9,mem);
    return 0;
}
