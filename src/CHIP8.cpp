#include "CHIP8.hpp"

#include <chrono>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>

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
	if(!stack.empty())
		std::cout << "stack=" << static_cast<int>(stack.top()) << std::endl;
	else
		std::cout << "stack=empty" << std::endl;
	std::cout << "delay=" << static_cast<int>(delayTimer) << std::endl;
	std::cout << "sound=" << static_cast<int>(soundTimer) << std::endl;
}
#endif

CHIP8::CHIP8(const char * romFile) : PC(0x200), I(fontAddr), instruction(0), delayTimer(0), soundTimer(0) {
	std::ifstream input(romFile, std::ios::binary);
	std::vector<uint8> buffer(std::istreambuf_iterator<char>(input), {});
	srand(static_cast<unsigned>(time(0)));
	memFill();
	displayFill();
	loadROM(Data::font, 0x0050);
	loadROM(buffer, 0x200);
#ifndef NDEBUG
	memDump(0x040, 0x0B0);
	std::cout << std::endl;
	memDump(0x1f0, 0x2a0);
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
	bool any = false;
#ifdef _WIN32
	if (GetKeyState('1') & 0x8000) { any = true; if (key == 0x1) return true; }
	else if (GetKeyState('2') & 0x8000) { any = true; if (key == 0x2) return true; }
	else if (GetKeyState('3') & 0x8000) { any = true; if (key == 0x3) return true; }
	else if (GetKeyState('4') & 0x8000) { any = true; if (key == 0xc) return true; }
	else if (GetKeyState('Q') & 0x8000) { any = true; if (key == 0x4) return true; }
	else if (GetKeyState('W') & 0x8000) { any = true; if (key == 0x5) return true; }
	else if (GetKeyState('E') & 0x8000) { any = true; if (key == 0x6) return true; }
	else if (GetKeyState('R') & 0x8000) { any = true; if (key == 0xd) return true; }
	else if (GetKeyState('A') & 0x8000) { any = true; if (key == 0x7) return true; }
	else if (GetKeyState('S') & 0x8000) { any = true; if (key == 0x8) return true; }
	else if (GetKeyState('D') & 0x8000) { any = true; if (key == 0x9) return true; }
	else if (GetKeyState('F') & 0x8000) { any = true; if (key == 0xe) return true; }
	else if (GetKeyState('Z') & 0x8000) { any = true; if (key == 0xa) return true; }
	else if (GetKeyState('X') & 0x8000) { any = true; if (key == 0x0) return true; }
	else if (GetKeyState('C') & 0x8000) { any = true; if (key == 0xb) return true; }
	else if (GetKeyState('V') & 0x8000) { any = true; if (key == 0xf) return true; }
#endif
	if(key == 16)
		return any;

	return false;
}

void CHIP8::setColour() const {
#ifdef _WIN32
	static char background = '0';
	static char pixel = '0';
	static bool deBounceBackground = false;
	static bool deBouncePixel = false;

	auto change = [](char key, char & val, bool & canPress) {
		if ((GetKeyState(key) & 0x8000)) {
			if (canPress) {
				if (val < '9' || val >= 'A') val++;
				else val = 'A';

				if (val > 'F') val = '0';

				std::string str = "Color ";
				str.push_back(background);
				str.push_back(pixel);
				system(str.c_str());
				canPress = false;
			}
		}
		else {
			canPress = true;
		}
	};

	change('M', background, deBounceBackground);
	change('N', pixel, deBouncePixel);
#endif
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
	uint8 X = (NNN & 0x0f00) >> 8;
	jump(NNN + V.at(X));
#else
	jump(NNN + V.at(0));
#endif
}

void CHIP8::random(uint8 X, uint8 NN) {
	const uint8 r = static_cast<uint8>(std::rand());
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
	const uint16 z = static_cast<unsigned>(V.at(X)) + static_cast<unsigned>(V.at(Y));
	if (z > 0xff) {
		V.at(0xf) = 1;
	}
	else {
		V.at(0xf) = 0;
	}
	V.at(X) += V.at(Y);
}

void CHIP8::subV(uint8 X, uint8 Y, bool swap) {
	uint8 A, B;
	if (swap) {
		A = Y;
		B = X;
	}
	else {
		A = X;
		B = Y;
	}

	if (V.at(A) >= V.at(B)) {
		V.at(0xf) = 1;
	}
	else {
		V.at(0xf) = 0;
	}
	V.at(X) = V.at(A) - V.at(B);
}

void CHIP8::shiftV(uint8 X, uint8 Y, bool left) {
#ifndef CHIP48
	V.at(X) = V.at(Y);
#endif
	const uint8 vx = V.at(X);
	uint8 mask;
	if (left) {
		mask = 0b10000000;
		V.at(X) = V.at(X) << 1;
	}
	else {
		mask = 0b00000001;
		V.at(X) = V.at(X) >> 1;
	}
	V.at(0xF) = (mask & vx) ? 1 : 0;
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
	if ((static_cast<unsigned>(I) + static_cast<unsigned>(V.at(X))) > 0xfff) {
		V.at(0xf) = 1;
	}
	I += V.at(X);
}

void CHIP8::getKeyBlocking(uint8 key) {
	if (getKey(16)) return;

	PC -= 2;
}

void CHIP8::fontCharacter(uint8 X) {
	I = fontAddr + 5 * (V.at(X) & 0x0f);
}

void CHIP8::bcdConversion(uint8 X) {
	memory.at(I) = (V.at(X) / 100) % 10;
	memory.at(I + 1) = (V.at(X) / 10) % 10;
	memory.at(I + 2) = V.at(X) % 10;
}

void CHIP8::storeMem(uint8 X) {
	std::memcpy(memory.data() + I, V.data(), X + 1);
#ifdef OLDMEM
	I += X + 1;
#endif
}

void CHIP8::loadMem(uint8 X) {
	std::memcpy(V.data(), memory.data() + I, X + 1);
#ifdef OLDMEM
	I += X + 1;
#endif
}

void CHIP8::playSound() const {
	std::cout << '\a' << std::flush;
}

void CHIP8::draw(uint8 X, uint8 Y, uint8 N) {
	const uint8 x = V.at(X) % 64;
	const uint8 y = V.at(Y) % 32;
	V.at(0xf) = 0;
	for (unsigned i = 0; i < N; i++) {
		if (y + i >= 32) break;

		const uint8 sprite = memory.at(I + i);
		for (unsigned j = 0; j < 8; j++) {
			const uint8 mask = 1 << (7 - j);
			const uint8 bit = sprite & mask;
			if (bit) {
				if (x + j >= 64) break;

				const unsigned index = x + j + (i + y) * 64;
				if (display.at(index)) {
					V.at(0xf) = 1;
				}
				display.at(index) = !display.at(index);
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
	const uint16 msb = memory.at(PC++) << 8;
	const uint8 lsb = memory.at(PC++);
	instruction = msb | lsb;

	const uint8 F = (instruction & 0xf000) >> 12;  // FXYN - 4 bits each
	const uint8 X = (instruction & 0x0f00) >> 8;
	const uint8 Y = (instruction & 0x00f0) >> 4;
	const uint8 N = (instruction & 0x000f);
	const uint8 NN = (instruction & 0x00ff);       // FXNN - 8 bits
	const uint16 NNN = (instruction & 0x0fff);     // FNNN - 12 bits

	switch (F) {
		case 0x0: {
			if (NNN == 0x0e0) {
				displayFill();
			}
			else if (NNN == 0x0ee) {
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
			else if (NN == 0x1e) {
				addIndexRegister(X);
			}
			else if (NN == 0x0a) {
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
	setColour();
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
	const double frameTime = 1/60.0;
	while (running) {
		const auto timeNow = std::chrono::steady_clock::now();
		const std::chrono::duration<double> elapsed_seconds = timeNow - timePrev;
		if (elapsed_seconds.count() > frameTime) {
			step();
			timePrev = timeNow;
		}
	}
	return 0;
}
