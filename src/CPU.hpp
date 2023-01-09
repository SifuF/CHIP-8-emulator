#pragma once

#include "defs.hpp"
#include "font.hpp"
#include <stack>
#include <vector>

class CPU {
public:
	CPU(const char * romFile = "roms/rom.ch8");
	~CPU() {};

	int run();
	void step();

	void displayFill(bool set = false);
	void decrementTimers();
	int getKeys();
	void fetchDecodeExecute();
	template <class Container>
	void loadROM(Container rom, uint16 addr);
	void memFill(uint8 set = 0);
	void screenClear() const;
	void screenUpdate() const;

	void draw(uint8 X, uint8 Y, uint8 N);
	void jump(uint16 addr);
	void setRegister(uint8 X, uint8 NN);
	void addRegister(uint8 X, uint8 NN);
	void setIndexRegister(uint16 val);

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
};
