#pragma once

#include "defs.hpp"
#include "font.hpp"

#include <stack>
#include <vector>

class CHIP8 {
public:
	CHIP8(const char * romFile = "roms/rom.ch8");
	~CHIP8() {};

	int run();
	void step();

	void displayFill(bool set = false);
	void decrementTimers();
	bool getKey(uint8 key = 16);
	void fetchDecodeExecute();
	template <class Container>
	void loadROM(Container rom, uint16 addr);
	void memFill(uint8 set = 0);
	void screenClear() const;
	void screenUpdate() const;
	void playSound() const;
	void setColour() const;

	void draw(uint8 X, uint8 Y, uint8 N);             //DXYN
	void jump(uint16 NNN);                            //1NNN
	void setRegister(uint8 X, uint8 NN);              //6XNN
	void addRegister(uint8 X, uint8 NN);              //7XNN
	void setIndexRegister(uint16 NNN);                //ANNN
	void callSubroutine(uint16 NNN);                  //2NNN
	void returnFromSubroutine();                      //00EE
	void skipIfVxEqualNn(uint8 X, uint8 NN);          //3XNN
	void skipIfVxNotEqualNn(uint8 X, uint8 NN);       //4XNN
	void skipIfVxEqualVy(uint8 X, uint8 Y);           //5XY0
	void skipIfVxNotEqualVy(uint8 X, uint8 Y);        //9XY0
	void jumpOffset(uint16 NNN);                      //BNNN
	void random(uint8 X, uint8 NN);                   //CXNN
	void skipIfKey(uint8 X);                          //EX9E
	void skipIfNotKey(uint8 X);                       //EXA1
	void setV(uint8 X, uint8 Y);                      //8XY0
	void orV(uint8 X, uint8 Y);                       //8XY1
	void andV(uint8 X, uint8 Y);                      //8XY2
	void xorV(uint8 X, uint8 Y);                      //8XY3
	void addV(uint8 X, uint8 Y);                      //8XY4
	void subV(uint8 X, uint8 Y, bool swap = false);   //8XY5 and 8XY7
	void shiftV(uint8 X, uint8 Y, bool left = false); //8XY6 and 8XYE
	void setVxFromTimer(uint8 X);                     //FX07
	void setTimerFromVx(uint8 X);                     //FX15
	void setSoundFromVx(uint8 X);                     //FX18
	void addIndexRegister(uint8 X);                   //FX1E
	void getKeyBlocking(uint8 key);                   //FX0A
	void fontCharacter(uint8 X);                      //FX29
	void bcdConversion(uint8 X);                      //FX33
	void storeMem(uint8 X);                           //FX55
	void loadMem(uint8 X);                            //FX65
#ifndef NDEBUG
	void memDump(unsigned begin, unsigned end) const;
	void printRegisters() const;
#endif
private:
	std::array<uint8, 4096> memory;
	std::array<bool, 64*32> display;
	std::stack<uint16> stack;
	uint16 PC;
	uint16 instruction;
	uint16 I;
	std::array<uint8, 16> V;
	uint8 delayTimer;
	uint8 soundTimer;

	bool running = true;
	static constexpr uint16 fontAddr = 0x50;
};
