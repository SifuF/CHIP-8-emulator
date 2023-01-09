#include "CPU.hpp"
#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

#ifndef NDEBUG
void CPU::memDump(unsigned begin, unsigned end) const {
	for (unsigned i = begin; i < end; i++) {
		if (!(i % 16))
			std::cout << std::endl << "offset " << std::hex << i << ": ";

		if (memory.at(i) < 16)
			std::cout << 0;

		std::cout << std::hex << static_cast<int>(memory.at(i)) << " ";
	}
	std::cout << std::endl;
}

void CPU::printRegisters() const {
	std::cout << std::hex << "PC=" << PC << " I=" << I;
	std::cout << " ins=" << static_cast<int>(instruction) << std::endl;
	for (unsigned i = 0; i < 16; i++) {
		std::cout << "V" << i << "=" << static_cast<int>(V.at(i)) << " ";
	}
	std::cout << std::endl;
}
#endif

CPU::CPU(const char * romFile) : PC(0x200), I(0x50), instruction(0), delayTimer(0), soundTimer(0) {
	std::ifstream input(romFile, std::ios::binary);
	std::vector<uint8> buffer(std::istreambuf_iterator<char>(input), {});

	memFill();
	displayFill();
	loadROM(Data::font, 0x0050);
	loadROM(buffer, 0x200);
#ifndef NDEBUG
	memDump(0x040, 0x0B0);
	std::cout << std::endl;
	memDump(0x1F0, 0x2A0);
	screenUpdate();
#endif
}

template <class Container>
void CPU::loadROM(Container rom, uint16 addr) {
	std::memcpy(memory.data() + addr, rom.data(), rom.size());
}

void CPU::memFill(uint8 set) {
	std::memset(memory.data(), set, memory.size());
}

void CPU::displayFill(bool set) {
	std::memset(display.data(), set, display.size());
}

void CPU::decrementTimers() {
	if (delayTimer > 0) delayTimer--;
	if (soundTimer > 0) soundTimer--;
}

int CPU::getKeys() {
	return 0;
}

void CPU::jump(uint16 addr) {
	PC = addr;
}

void CPU::setRegister(uint8 reg, uint8 val) {
	V.at(static_cast<int>(reg)) = val;
}

void CPU::addRegister(uint8 reg, uint8 val) {
	V.at(static_cast<int>(reg)) += val;
}

void CPU::setIndexRegister(uint16 val) {
	I = val;
}

void CPU::draw(uint8 X, uint8 Y, uint8 N) {
	uint8 x = V.at(X) % 64;
	uint8 y = V.at(Y) % 32;
	V.at(15) = 0;
	for (unsigned i = 0; i < N; i++) {
		uint8 sprite = memory.at(I + i);
		for (unsigned j = 0; j < 8; j++) {
			uint8 mask = 1 << (7-j);
			uint8 bit = sprite & mask;
			if (bit) {
				display.at(x + j + (i + y) * 64) = !display.at(x + j + (i + y) * 64);
				V.at(15) = 1;
			}
	    }
	}
}

void CPU::screenUpdate() const {
	for (unsigned i = 0; i < display.size(); i++) {
		if (!(i % 64))
			std::cout << std::endl;

		if (display.at(i))
			std::cout << '@';
		else
			std::cout << ' ';
	}
	std::cout << std::endl;
}

void CPU::screenClear() const {
#ifdef _WIN32
	system("CLS");
#else
	cout << "\033[2J\033[1;1H";
#endif
}

void CPU::fetchDecodeExecute() {
	uint16 msb = memory.at(PC++) << 8;
	uint8 lsb = memory.at(PC++);
	instruction = msb | lsb;

	uint8 F = (instruction & 0xF000) >> 12;  // FXYN - 4 bits each
	uint8 X = (instruction & 0x0F00) >> 8;
	uint8 Y = (instruction & 0x00F0) >> 4;
	uint8 N = (instruction & 0x000F);
	uint8 NN = (instruction & 0x00FF);       // FXNN - 8 bits
	uint16 NNN = (instruction & 0x0FFF);     // FNNN - 12 bits

	switch (F) {
		case 0x0: {
			if (NNN == 0x0E0) {
				displayFill();
			}
			break;
		}
		case 0x1: {
			jump(NNN);
			break;
		}
		case 0x2: {
			break;
		}
		case 0x3: {
			break;
		}
		case 0x4: {
			break;
		}
		case 0x5: {
			break;
		}
		case 0x6: {
			setRegister(X, NN);
			break;
		}
		case 0x7: {
			addRegister(X, NN);
			break;
		}
		case 0x8: {
			break;
		}
		case 0x9: {
			break;
		}
		case 0xa: {
			setIndexRegister(NNN);
			break;
		}
		case 0xb: {
			break;
		}
		case 0xc: {
			break;
		}
		case 0xd: {
#ifdef NDEBUG
			screenClear();
#endif
			draw(X, Y, N);
#ifdef NDEBUG
			screenUpdate();
#endif
			break;
		}
		case 0xe: {
			break;
		}
		case 0xf: {
			break;
		}
		default: {
			std::cout << "Error - unknown instruction";
			break;
		}
	}
}

void CPU::step() {
#ifndef NDEBUG
	screenClear();
#endif
	decrementTimers();
	getKeys();
#ifndef NDEBUG
	printRegisters();
#endif
	fetchDecodeExecute();
#ifndef NDEBUG
	screenUpdate();
#endif
}

int CPU::run() {
	auto timePrev = std::chrono::steady_clock::now();
	double frameTime = 1/60.0;
	while (running) {
		auto timeNow = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = timeNow - timePrev;
		if (elapsed_seconds.count() > frameTime) {
			step();
			timePrev = timeNow;
		}
	}
	return 0;
}
