#pragma once
#include <cstdint>

class apu2A03{

    public:
        apu2A03();
        ~apu2A03();
    
    public:
        void cpuWrite(uint16_t addr, uint8_t data);
        uint8_t cpuRead(uint16_t addr);
        void clock();
        void reset();
        double get_sample();

        inline static constexpr std::array<uint8_t, 32> length_counter_lookup = {
            10, 254, 20, 2,  40, 4,  80, 6,  160, 8,  60, 10, 14, 12, 26, 14,
            12, 16,  24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30
        };   

        inline static constexpr std::array<uint16_t, 16> noise_period_lookup = {
            4, 8, 16, 32, 64, 96, 128, 160,
            202, 254, 380, 508, 762, 1016, 2034, 4068
        };

    private:
        struct pulse{
            uint8_t duty = 0x00;
            uint8_t volume = 0x00;
            bool const_volume = false;
            bool length_counter_halt = false;
            uint16_t timer = 0x0000;

            bool sweep_active = false;
            uint8_t sweep_period = 0x00;
            uint8_t sweep_shift = 0x00;
            bool sweep_negate = false;
            
            uint16_t current_timer = 0x0000;
            uint8_t sequence_step = 0x00;
            
            uint8_t length_counter = 0x00;

            bool envelope_start = false;
            uint8_t envelope_divider = 0x00;
            uint8_t decay_level = 0x00;

            uint8_t sweep_divider = 0x00;
            bool sweep_reload = false;
        };

        struct triangle_wave{
            bool linear_count_control = false;
            uint8_t linear_count_load = 0x00;
            uint16_t timer = 0x0000;
            uint8_t length_counter_load = 0x00;

            uint16_t current_timer = 0x0000;
            uint8_t sequence_step = 0x00;

            uint8_t length_counter = 0x00;
            uint8_t linear_counter = 0x00;
            bool linear_count_reload = false;
        };

        struct noise_channel{
            bool length_counter_halt = false;
            bool const_volume = false;
            uint8_t volume = 0x00;
            bool noise_mode = false;
            uint8_t noise_period = 0x00;
            uint8_t length_counter_load = 0x00;

            uint16_t current_timer = 0x0000;
            uint8_t length_counter = 0x00;
            uint16_t shift_register = 0x0001; // 1 b/c it will stay silent forever b/c of xor op

            bool envelope_start = false;
            uint8_t envelope_divider = 0x00;
            uint8_t decay_level = 0x00;
        };

        

        bool active_pulse1 = false;
        struct pulse pulse1;
        struct pulse pulse2;
        struct triangle_wave triangle;
        struct noise_channel noise;


};