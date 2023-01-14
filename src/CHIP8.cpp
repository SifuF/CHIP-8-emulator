#include "CHIP8.hpp"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

//#define OP_BXNN
#define CHIP48
//#define OLDMEM

#ifndef NDEBUG
void CHIP8::memDump(unsigned begin, unsigned end) const {
	for (unsigned i = begin; i < end; i++) {
		if (!(i % 16))
			std::cout << std::endl << "offset " << std::hex << i << ": ";

		if (memory.at(i) < 16)
			std::cout << 0;

		std::cout << std::hex << static_cast<int>(memory.at(i)) << " ";
	}
	std::cout << std::endl;
}

void CHIP8::printRegisters() const {
	std::cout << std::hex << "PC=" << PC << " I=" << I;
	std::cout << " ins=" << static_cast<int>(instruction) << std::endl;
	for (unsigned i = 0; i < 16; i++) {
		std::cout << "V" << i << "=" << static_cast<int>(V.at(i)) << " ";
	}
	std::cout << std::endl;
	std::cout << "delay=" << static_cast<int>(delayTimer) << std::endl;
	std::cout << "sound=" << static_cast<int>(soundTimer) << std::endl;
}
#endif

CHIP8::CHIP8(const char * romFile) : PC(0x200), I(0x50), instruction(0), delayTimer(0), soundTimer(0) {
	std::ifstream input(romFile, std::ios::binary);
	std::vector<uint8> buffer(std::istreambuf_iterator<char>(input), {});
	srand(time(NULL));
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
void CHIP8::loadROM(Container rom, uint16 addr) {
	std::memcpy(memory.data() + addr, rom.data(), rom.size());
}

void CHIP8::memFill(uint8 set) {
	std::memset(memory.data(), set, memory.size());
}

void CHIP8::displayFill(bool set) {
	std::memset(display.data(), set, display.size());
}

void CHIP8::decrementTimers() {
	if (delayTimer > 0)
		delayTimer--;

	if (soundTimer > 0) {
		playSound();
		soundTimer--;
	}
}

bool CHIP8::getKey(uint8 key) {
	//get keys
	return false;
}

void CHIP8::jump(uint16 NNN) {
	PC = NNN;
}

void CHIP8::callSubroutine(uint16 NNN) {
	stack.push(PC);
	jump(NNN);
}

void CHIP8::returnFromSubroutine() {
	PC = stack.top();
	stack.pop();
}

void CHIP8::setRegister(uint8 X, uint8 NN) {
	V.at(X) = NN;
}

void CHIP8::addRegister(uint8 X, uint8 NN) {
	V.at(X) += NN;
}

void CHIP8::setIndexRegister(uint16 NNN) {
	I = NNN;
}

void CHIP8::skipIfVxEqualNn(uint8 X, uint8 NN) {
	if (V.at(X) == NN) PC += 2;
}

void CHIP8::skipIfVxNotEqualNn(uint8 X, uint8 NN) {
	if (V.at(X) != NN) PC += 2;
}

void CHIP8::skipIfVxEqualVy(uint8 X, uint8 Y) {
	if (V.at(X) == V.at(Y)) PC += 2;
}

void CHIP8::skipIfVxNotEqualVy(uint8 X, uint8 Y) {
	if (V.at(X) != V.at(Y)) PC += 2;
}

void CHIP8::jumpOffset(uint16 NNN) {
#ifdef OP_BXNN
	uint8 X = (NNN & 0x0F00) >> 8;
	jump(NNN + V.at(X));
#else
	jump(NNN + V.at(0));
#endif
}

void CHIP8::random(uint8 X, uint8 NN) {
	uint8 r = static_cast<uint8>(std::rand());
	V.at(X) = r & NN;
}

void CHIP8::skipIfKey(uint8 X) {
	if (getKey(V.at(X))) PC += 2;
}

void CHIP8::skipIfNotKey(uint8 X) {
	if (!getKey(V.at(X))) PC += 2;
}

void CHIP8::setV(uint8 X, uint8 Y) {
	V.at(X) = V.at(Y);
}

void CHIP8::orV(uint8 X, uint8 Y) {
	V.at(X) = V.at(X) | V.at(Y);
}

void CHIP8::andV(uint8 X, uint8 Y) {
	V.at(X) = V.at(X) & V.at(Y);
}

void CHIP8::xorV(uint8 X, uint8 Y) {
	V.at(X) = V.at(X) ^ V.at(Y);
}

void CHIP8::addV(uint8 X, uint8 Y) {
	uint16 z = static_cast<uint16>(V.at(X)) + static_cast<uint16>(V.at(Y));
	if (z > 0xFF) {
		V.at(0xf) = 1;
	}
	else {
		V.at(0xf) = 0;
	}
	V.at(X) = static_cast<uint8>(z);
}

void CHIP8::subV(uint8 X, uint8 Y, bool swap) {
	if (swap) {
		if (V.at(Y) >= V.at(X)) {
			V.at(0xf) = 1;
		}
		else {
			V.at(0xf) = 0;
		}
		V.at(X) = V.at(Y) - V.at(X);
	}
	else {
		if (V.at(X) >= V.at(Y)) {
			V.at(0xf) = 1;
		}
		else {
			V.at(0xf) = 0;
		}
		V.at(X) = V.at(X) - V.at(Y);
	}
}

void CHIP8::shiftV(uint8 X, uint8 Y, bool left) {
#ifndef CHIP48
	V.at(X) = V.at(Y);
#endif
	uint8 x_tmp = V.at(X);
	uint8 mask;
	if (left) {
		mask = 0b10000000;
		V.at(X) = V.at(X) << 1;
	}
	else {
		mask = 0b00000001;
		V.at(X) = V.at(X) >> 1;
	}
	V.at(0xF) = (mask & x_tmp) ? 1 : 0;
}

void CHIP8::setVxFromTimer(uint8 X) {
	V.at(X) = delayTimer;
}

void CHIP8::setTimerFromVx(uint8 X) {
	delayTimer = V.at(X);
}

void CHIP8::setSoundFromVx(uint8 X) {
	soundTimer = V.at(X);
}

void CHIP8::addIndexRegister(uint8 X) {
	if ((static_cast<unsigned>(I) + static_cast<unsigned>(V.at(X))) > 0xFFF) {
		V.at(0xf) = 1;
	}
	I += V.at(X);
}

void CHIP8::getKeyBlocking(uint8 key) {
	if (getKey()) return;
	else PC -= 2;
}

void CHIP8::fontCharacter(uint8 X) {
	I = 0x50 + 5 * (V.at(X) & 0x0F);
}

void CHIP8::bcdConversion(uint8 X) {
	memory.at(I) = (V.at(X) / 100) % 10;
	memory.at(I + 1) = (V.at(X) / 10) % 10;
	memory.at(I + 2) = V.at(X) % 10;
}

void CHIP8::storeMem(uint8 X) {
	memcpy(memory.data() + I, V.data(), X + 1);
#ifdef OLDMEM
	I += X + 1;
#endif
}

void CHIP8::loadMem(uint8 X) {
	memcpy(V.data(), memory.data() + I, X + 1);
#ifdef OLDMEM
	I += X + 1;
#endif
}

void CHIP8::playSound() const {
	std::cout << '\a' << std::flush;
}

void CHIP8::draw(uint8 X, uint8 Y, uint8 N) {
	uint8 x = V.at(X) % 64;
	uint8 y = V.at(Y) % 32;
	V.at(0xf) = 0;
	for (unsigned i = 0; i < N; i++) {
		uint8 sprite = memory.at(I + i);
		for (unsigned j = 0; j < 8; j++) {
			uint8 mask = 1 << (7-j);
			uint8 bit = sprite & mask;
			if (bit) {
				if (display.at(x + j + (i + y) * 64)) {
					V.at(15) = 1;
				}
				display.at(x + j + (i + y) * 64) = !display.at(x + j + (i + y) * 64);
			}
	    }
	}
}

void CHIP8::screenUpdate() const {
	std::string buffer = "";
	for (unsigned i = 0; i < display.size(); i++) {
		if (!(i % 64))
			buffer += '\n';

		if (display.at(i))
			buffer += '@';
		else
			buffer += ' ';
	}
	std::cout << buffer << std::endl;
}

void CHIP8::screenClear() const {
#ifdef _WIN32
	system("CLS");
#else
	std::cout << "\033[2J\033[1;1H";
#endif
}

void CHIP8::fetchDecodeExecute() {
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
			else if (NNN == 0x0EE) {
				returnFromSubroutine();
			}
			else {
				std::cout << "Error - Attempting to execute at: " << std::hex << static_cast<int>(NNN) << std::endl;
			}
			break;
		}
		case 0x1: {
			jump(NNN);
			break;
		}
		case 0x2: {
			callSubroutine(NNN);
			break;
		}
		case 0x3: {
			skipIfVxEqualNn(X, NN);
			break;
		}
		case 0x4: {
			skipIfVxNotEqualNn(X, NN);
			break;
		}
		case 0x5: {
			if (N == 0) {
				skipIfVxEqualVy(X, Y);
			}
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
			switch (N) {
			    case 0x0: {
					setV(X, Y);
				    break;
			    }
				case 0x1: {
					orV(X, Y);
					break;
				}
				case 0x2: {
					andV(X, Y);
					break;
				}
				case 0x3: {
					xorV(X, Y);
					break;
				}
				case 0x4: {
					addV(X, Y);
					break;
				}
				case 0x5: {
					subV(X, Y);
					break;
				}
				case 0x6: {
					shiftV(X, Y);
					break;
				}
				case 0x7: {
					subV(X, Y, true);
					break;
				}
				case 0xe: {
					shiftV(X, Y, true);
					break;
				}
				default: {
					std::cout << "Error - Unknown ALU instruction";
					break;
				}
			}
			break;
		}
		case 0x9: {
			if (N == 0) {
				skipIfVxNotEqualVy(X, Y);
			}
			break;
		}
		case 0xa: {
			setIndexRegister(NNN);
			break;
		}
		case 0xb: {
			jumpOffset(NNN);
			break;
		}
		case 0xc: {
			random(X, NN);
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
			if (NN == 0x9e) {
				skipIfKey(X);
			}
			else if (NN == 0xa1) {
				skipIfNotKey(X);
			}
			break;
		}
		case 0xf: {
			if (NN == 0x07) {
				setVxFromTimer(X);
			}
			else if (NN == 0x15) {
				setTimerFromVx(X);
			}
			else if (NN == 0x18) {
				setSoundFromVx(X);
			}
			else if (NN == 0x1E) {
				addIndexRegister(X);
			}
			else if (NN == 0x0A) {
				getKeyBlocking(X);
			}
			else if (NN == 0x29) {
				fontCharacter(X);
			}
			else if (NN == 0x33) {
				bcdConversion(X);
			}
			else if (NN == 0x55) {
				storeMem(X);
			}
			else if (NN == 0x65) {
				loadMem(X);
			}
			break;
		}
		default: {
			std::cout << "Error - Unknown instruction";
			break;
		}
	}
}

void CHIP8::step() {
#ifndef NDEBUG
	screenClear();
#endif
	decrementTimers();
#ifndef NDEBUG
	printRegisters();
#endif
	fetchDecodeExecute();
#ifndef NDEBUG
	screenUpdate();
#endif
}

int CHIP8::run() {
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
