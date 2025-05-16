#include "ppu2C02.h"
using ppu = ppu2C02;

void ppu::connectCart(const std::shared_ptr<cartridge>& cart){
    this->game = cart;
}