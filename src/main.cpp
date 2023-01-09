// CHIP-8 emulator by SifuF

#include "CPU.hpp"

int main(int argc, char** argv) {
	CPU cpu("roms/ibm.ch8");
	return cpu.run();
}
