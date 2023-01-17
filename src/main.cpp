// CHIP-8 emulator by SifuF

#include "CHIP8.hpp"

int main(int argc, char** argv) {
	CHIP8 chip8("roms/Breakout (Brix hack) [David Winter, 1997].ch8");
	//CHIP8 chip8("roms/INVADERS");
	return chip8.run();
}
