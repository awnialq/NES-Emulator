# Comprehensive Technical Summary - CPU Instruction Fixes & PPU Pipeline Implementation

This document provides a highly thorough, exhaustively detailed explanation of both the CPU instruction bugs and the PPU (Picture Processing Unit) rendering pipeline upgrades implemented in the NES emulator. It documents what was changed, why it was changed, the theoretical background, and the engineering details.

---

# PART 1: CPU Component Fixes & Controller Input

## 1. Background: NES Controller Hardware & Polling Protocols

To understand why the CPU bugs caused controller inputs to fail completely, it is essential to understand how standard NES games poll controller buttons.

### Serial Protocol
Unlike modern parallel controllers, the standard NES controller (Standard Controller / Joypad) operates using a shift-register chip (CD4021) to transmit button states serially over a single data line. 
- The controller buttons are ordered sequentially: **A, B, Select, Start, Up, Down, Left, Right**.
- The NES console has two memory-mapped registers for inputs: `$4016` (Controller 1) and `$4017` (Controller 2).

### Controller Strobing State Machine
To query the controller, the NES CPU performs a three-step sequence:
1. **Write `1` to `$4016`**: This raises the strobe line high. While high, the shift register is continually loaded with the current instantaneous button states.
2. **Write `0` to `$4016`**: This lowers the strobe line, latching/freezing the button states into the shift register.
3. **Read `$4016` (8 times)**: Each read shifts out one bit on data line D0 (`$01`), starting with button **A** and ending with button **Right**.

### Joypad Polling Assembly Loop
A typical production-grade NES game handles this sequence using an optimized CPU assembly loop similar to the following:

```assembly
ReadControllers:
    LDA #$01
    STA $4016       ; Strobe high (latch state)
    LDA #$00
    STA $4016       ; Strobe low (begin shifting)
    LDX #$08        ; Set counter for 8 buttons
ReadLoop:
    LDA $4016       ; Read next button bit into D0 of Accumulator (A)
    LSR A           ; Shift D0 into the Carry flag (C)
    ROL buttons     ; Rotate Carry flag into the game's RAM variable
    DEX             ; Decrement index
    BNE ReadLoop    ; Loop 8 times
    RTS
```

Note how the loop relies entirely on two core CPU instructions:
- **`LSR A`**: Shifting the accumulator right to move the controller bit (bit 0) into the Carry flag.
- **`ROL memory`**: Rotating the Carry flag into a RAM variable to pack the 8 button bits into a single byte.

If either `LSR A` or `ROL` behaves incorrectly, the button bits are completely scrambled or lost, making the game think no inputs (or junk inputs) are being pressed.

---

## 2. Technical Breakdown of the Discovered CPU Bugs

We discovered three critical bugs in the instruction implementations within [cpu6502.cpp](file:///Users/awni/Documents/GitHub/NES-Emulator/src/cpu6502.cpp):

### Bug A: Accumulator Address Mode (`IMP`) Stale-Operand Bug
In our emulator's CPU instruction lookup table, instructions that operate directly on the Accumulator (such as `ASL A`, `LSR A`, `ROL A`, `ROR A`) are mapped with the implicit (`IMP`) addressing mode.

Prior to the fix, the instructions were implemented like this (using `LSR` as an example):
```cpp
uint8_t cpu::LSR(){
    fetch(); // <-- BUG
    setFlag(C, fetched & 0x0001);
    fetched = fetched >> 1;
    ...
    if(lookup[opcode].addrmode == &cpu::IMP){
        accum = fetched;
    }
```

#### The Problem:
1. **`fetch()` is a no-op**: Under the implicit (`IMP`) addressing mode, the `fetch()` helper does not read from memory and returns the existing value of the member variable `fetched` unchanged.
2. **Stale Data Corruption**: `fetched` holds whatever operand byte was processed by the *previous* instruction (e.g. an address byte, immediate value, or status flags from a completely unrelated calculation).
3. **Execution**: The instruction shifted/rotated this stale value, updated the CPU flags, and overwrote the Accumulator (`accum`) with the resulting garbage data!
4. **Input Loss**: When the game did `LDA $4016` (putting the controller bit into the accumulator) followed by `LSR A`, the `LSR` instruction ignored the accumulator, shifted stale data instead, and trashed the accumulator. The game never received the controller state.

### Bug B: `ROL` Carry-Flag Overwrite Bug
In the `ROL` (Rotate Left) instruction, the bitwise behavior should shift the *original* carry flag state into the LSB (bit 0) of the target, and shift the MSB (bit 7) of the target into the *new* carry flag.

Prior to the fix, `ROL` was implemented as:
```cpp
uint8_t cpu::ROL(){
    fetch();
    setFlag(C, fetched >> 7);                  // <-- Sets NEW carry
    fetched = fetched << 1;
    if(getFlag(C) == 1){fetched |= 0x01;}      // <-- Checks NEW carry (MSB of old)
    ...
```

#### The Problem:
By calling `setFlag(C, fetched >> 7)` *before* checking the carry flag, the original Carry state was completely overwritten. 
- If the original carry was `0` but the MSB of the fetched value was `1`, the new carry became `1`.
- The code then checked `getFlag(C) == 1` (which was the MSB) and set bit 0 to `1`.
- This essentially duplicated the MSB (bit 7) into the LSB (bit 0), losing the actual Carry state entirely!

---

## 3. CPU Changes: Implementation Detail

To fix these issues, we modified [cpu6502.cpp](file:///Users/awni/Documents/GitHub/NES-Emulator/src/cpu6502.cpp) to isolate the data source and target based on the addressing mode dynamically, and buffered status flags to prevent premature overwrites.

### ASL (Arithmetic Shift Left) Fix
- **What Changed**: We introduced a local variable `val`. If the instruction's addressing mode is `IMP` (Accumulator mode), `val` is bound directly to `accum`. Otherwise, `fetch()` is executed to load the operand into `fetched` from memory, and `val` is bound to `fetched`.
- **Code Change**:
  ```cpp
  uint8_t cpu::ASL(){
      uint8_t val;
      if(lookup[opcode].addrmode == &cpu::IMP){
          val = accum;
      }
      else{
          fetch();
          val = fetched;
      }
      setFlag(C, (val >> 7) != 0);
      val = val << 1;
      setFlag(Z, val == 0x00);
      setFlag(N, (val >> 7) != 0);
      if(lookup[opcode].addrmode == &cpu::IMP){
          accum = val;
      }
      else{
          write(addr_absolute, val);
      }
      return 0;
  }
  ```

### LSR (Logical Shift Right) Fix
- **What Changed**: Similar to `ASL`, we read the operand into a local `val` from `accum` in implicit mode, shift it right by 1, and write the output back to `accum` (or memory).
- **Code Change**:
  ```cpp
  uint8_t cpu::LSR(){
      uint8_t val;
      if(lookup[opcode].addrmode == &cpu::IMP){
          val = accum;
      }
      else{
          fetch();
          val = fetched;
      }
      setFlag(C, (val & 0x01) != 0);
      val = val >> 1;
      setFlag(Z, val == 0);
      setFlag(N, 0);
      if(lookup[opcode].addrmode == &cpu::IMP){
          accum = val;
      }
      else{
          write(addr_absolute, val);
      }
      return 0;
  }
  ```

### ROL (Rotate Left) Fix
- **What Changed**: In addition to isolating the source (`accum` vs. `fetched`), we buffered the original carry flag state (`originalCarry = getFlag(C)`) *before* updating `C` with the MSB (`val >> 7`). We then used `originalCarry` to shift into bit 0.
- **Code Change**:
  ```cpp
  uint8_t cpu::ROL(){
      uint8_t val;
      if(lookup[opcode].addrmode == &cpu::IMP){
          val = accum;
      }
      else{
          fetch();
          val = fetched;
      }
      uint8_t originalCarry = getFlag(C);
      setFlag(C, (val >> 7) != 0);
      val = val << 1;
      if(originalCarry == 1){
          val |= 0x01;
      }
      setFlag(Z, val == 0);
      setFlag(N, (val >> 7) != 0);
      if(lookup[opcode].addrmode == &cpu::IMP){
          accum = val;
      }
      else{
          write(addr_absolute, val);
      }
      return 0;
  }
  ```

### ROR (Rotate Right) Fix
- **What Changed**: Buffered the original carry flag state (`originalCarry = getFlag(C)`), set the new carry based on the LSB (`val & 0x01`), shifted `val` right by 1, and packed `originalCarry` into bit 7 (`originalCarry << 7`).
- **Code Change**:
  ```cpp
  uint8_t cpu::ROR(){
      uint8_t val;
      if(lookup[opcode].addrmode == &cpu::IMP){
          val = accum;
      }
      else{
          fetch();
          val = fetched;
      }
      uint8_t originalCarry = getFlag(C);
      setFlag(C, (val & 0x01) != 0);
      val = (val >> 1) | (originalCarry << 7);
      setFlag(Z, val == 0x00);
      setFlag(N, (val >> 7) != 0);
      if(lookup[opcode].addrmode == &cpu::IMP){
          accum = val;
      }
      else{
          write(addr_absolute, val);
      }
      return 0;
  }
  ```

---
---

# PART 2: PPU Component & Graphics Pipeline Upgrades

In addition to fixing the CPU instructions, we fully implemented foreground sprite rendering, scrolling memory registers, OAM DMA, and dynamic screen pixel merging.

## 1. Complete Background Scrolling & Nametable State Machine

In the NES, horizontal and vertical scrolling are not simple coordinate offsets. The PPU implements scrolling by manipulating two internal 15-bit address registers: `vramAddr` (current VRAM address, register `v`) and `tramAddr` (temporary VRAM address, register `t`).

### Bit Structure of `vramAddr` and `tramAddr`
Both registers are structured as follows:
```
yyy NN YYYYY XXXXX
||| || ||||| +++++-- Coarse X scroll (0-31 tiles)
||| || +++++-------- Coarse Y scroll (0-31 tiles)
||| ++-------------- Nametable select (0-3: $2000, $2400, $2800, $2C00)
+++----------------- Fine Y scroll (0-7 pixels within a tile)
```

### Scrolling Implementations ([ppu2C02.cpp](file:///Users/awni/Documents/GitHub/NES-Emulator/src/ppu2C02.cpp))

We implemented the precise hardware scrolling logic using three timing-safe state-machine updates:

1. **`incrementScrollY()`**: Called at the end of each active scanline to advance the rendering address to the next row of pixels.
   - **Logic**: Increments the Fine Y bits (`0x7000`). If Fine Y rolls over from `7` to `0`, it resets Fine Y and increments the Coarse Y bits (`0x03E0`).
   - **Boundary wrapping**: If Coarse Y reaches `29` (the 30th tile row of a nametable), it wraps back to `0` and toggles the vertical nametable select bit (`0x0800`). If it hits `31`, it wraps to `0` without toggling.
   ```cpp
   void ppu::incrementScrollY() {
       if (mask & 0x18) { // If rendering is enabled
           if ((vramAddr & 0x7000) != 0x7000) {
               vramAddr += 0x1000;        // Increment fine Y
           } else {
               vramAddr &= ~0x7000;       // Fine Y = 0
               int y = (vramAddr >> 5) & 0x001F;
               if (y == 29) {
                   y = 0;
                   vramAddr ^= 0x0800;    // Switch vertical nametable
               } else if (y == 31) {
                   y = 0;                 // Coarse Y = 0, no nametable switch
               } else {
                   y++;                   // Increment coarse Y
               }
               vramAddr = (vramAddr & ~0x03E0) | (y << 5);
           }
       }
   }
   ```

2. **`transferAddressX()`**: Called at PPU cycle 257 of each scanline. It restores/copies the horizontal scrolling parameters (Coarse X and horizontal nametable bit) from the temporary register `tramAddr` into `vramAddr`, preparing the PPU to scan the next line from the same horizontal start coordinate.
   ```cpp
   void ppu::transferAddressX() {
       if (mask & 0x18) {
           vramAddr = (vramAddr & 0xFBE0) | (tramAddr & 0x041F);
       }
   }
   ```

3. **`transferAddressY()`**: Called on the pre-render scanline (-1) at cycle 304. It restores/copies all vertical scrolling parameters (Coarse Y, Fine Y, and vertical nametable bit) from `tramAddr` into `vramAddr`, resetting the vertical scroll position to the top of the viewport for the upcoming frame.
   ```cpp
   void ppu::transferAddressY() {
       if (mask & 0x18) {
           vramAddr = (vramAddr & 0x041F) | (tramAddr & 0x7BE0);
       }
   }
   ```

---

## 2. Foreground Sprite Rendering & Pipeline Integration

To render actors and cursors, we implemented a full sprite evaluation and rendering pipeline, supporting standard 8x8 sprites and high-resolution 8x16 sprites.

### Sprite Evaluation (`evaluateSprites()`)
NES hardware evaluates which sprites to draw on a per-scanline basis to mimic a scanline-oriented renderer.
- **Scanline intersection**: The PPU reads OAM (Object Attribute Memory) and checks which sprites intersect the current scanline (`y_coordinate <= scanline < y_coordinate + sprite_height`).
- **Standard Limitations**: Up to 8 active sprites are selected per scanline. If more than 8 intersecting sprites are found, the **Sprite Overflow** flag (bit 5 of PPU Status register `$2002`) is raised.
- **Sizing**: Supports 8x8 sprites (pattern tables selected via bit 3 of PPUCTRL) and 8x16 sprites (pattern base specified dynamically by bit 0 of the sprite's tile ID).
- **Flips**: Evaluates vertical flips (`attributes & 0x80`) by reversing the pattern row calculation.

### Screen Pixel Merging (`renderPixel()`)
At each PPU clock cycle (representing a screen pixel `x`), the background and foreground sprite pipelines are evaluated and merged:

1. **Background Pixel Fetching**:
   - Calculates the absolute scrolled coordinates `absoluteX` using the current `vramAddr` (horizontal offset) and `fineX` scroll registers.
   - Evaluates the nametable ID, attributes, pattern LSB/MSB bits, and attribute shift values to fetch the base color index (0-3) and background palette ID (0-3).
2. **Foreground Sprite Processing**:
   - Loops through evaluated active sprites. If the current screen coordinate `x` falls within a sprite's horizontal span (`sprite.x <= x < sprite.x + 8`), the sprite's pattern bits are shifted (handling horizontal flips via `attributes & 0x40`) to retrieve the sprite's color index (0-3) and palette ID.
3. **Merging & Priority Resolution**:
   - If both background and sprite pixels are non-transparent (`index != 0`), the sprite's priority bit (`attributes & 0x20`) is checked:
     - **Priority = 0**: The sprite is drawn in front of the background.
     - **Priority = 1**: The sprite is drawn behind the background (showing the background instead).
   - **Sprite Zero Hit**: If a non-transparent background pixel overlaps a non-transparent pixel of Sprite 0 (the first sprite in OAM), the **Sprite Zero Hit** flag (bit 6 of PPU Status `$2002`) is raised. This is commonly used by games to sync CPU instructions with screen raster positions (e.g. for split-screen scrolling).

---

## 3. OAM DMA and Mirrored RAM Memory Correction

To support efficient OAM updates and robust memory behavior, we corrected the memory writes on the Bus.

### Mirrored RAM Capacity Fix ([Bus.cpp](file:///Users/awni/Documents/GitHub/NES-Emulator/src/Bus.cpp))
Previously, the emulator's main memory array `cpuMem` was 2048 bytes (2KB), but CPU memory writes within the `$0000` to `$1FFF` range did not apply correct mirroring bounds. This resulted in out-of-bounds array writes for addresses above `$07FF`. We corrected this by applying a mirroring mask of `0x07FF` to mirror the full 8KB space into the physical 2KB RAM safely:
```cpp
if(addr >= 0x0000 && addr <= 0x1FFF){
    cpuMem[addr & 0x07FF] = data; // Implemented address mirroring by anding with 2KB
}
```

### OAM DMA Transfer (`0x4014`)
Updating 256 bytes of sprite memory manually using single CPU writes is too slow. The NES provides a dedicated Direct Memory Access (DMA) channel on register `$4014`.
- When a page number `XX` is written to `$4014`, the CPU is suspended, and the DMA chip copies 256 bytes of memory from `$XX00`-`$XXFF` directly into the PPU's internal OAM array, auto-incrementing `ppu.oamAddr` sequentially.
- We implemented this hardware transfer natively inside `Bus::cpuWrite`:
  ```cpp
  else if(addr == 0x4014){
      // OAM DMA transfer
      uint16_t dmaSrc = static_cast<uint16_t>(data) << 8;
      for(int i = 0; i < 256; i++){
          ppu.oam[ppu.oamAddr] = cpuRead(dmaSrc + i);
          ppu.oamAddr++;
      }
  }
  ```

---

## 4. Overall Outcomes & System Harmony

These changes combine to form a fully operational NES console emulator:
- **Scrolling** enables dynamic viewport scrolling in games without trashing nametables.
- **Sprite Evaluation & Merging** displays actors, cursors, and player characters with accurate transparency and overlapping layering.
- **OAM DMA** provides super-fast rendering updates for active sprites.
- **CPU Instruction Fixes** ensure the serial strobe shifts are processed correctly by the controller polling loops, allowing you to control and interact with the software using your keyboard!
