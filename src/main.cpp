// CHIP-8 emulator by SifuF

#include"CHIP8.hpp"

#include<memory>

int main(int argc, char** argv) {
	const auto * filename = "../roms/sifuf.ch8";
	auto chip8 = std::make_unique<CHIP8>(filename);
	return chip8->run();
}
