# gbemu

gbemu (name subject to change) is a very work-in-progress emulator for the Nintendo Game Boy.

#### Test status:

Blargg's CPU Tests:

|#|name|state|
|-|-|-|
|01|special|:white_check_mark:|
|02|interrupts|:white_check_mark:|
|03|op sp,hl|:white_check_mark:|
|04|op r,imm|:white_check_mark:|
|05|op rp|:white_check_mark:|
|06|ld r,r|:white_check_mark:|
|07|jr,jp,call,ret,rst|:white_check_mark:|
|08|misc instrs|:white_check_mark:|
|09|op r,r|:white_check_mark:|
|10|bit ops|:white_check_mark:|
|11|op a,(hl)|:white_check_mark:|
|12|cpu_instrs|:white_check_mark:|

Blargg's Miscellaneous Tests:

|#|name|state|
|-|-|-|
|01|halt_bug|:white_check_mark:|
|02|instr_timing|:white_check_mark:|
|03|interrupt_time|:x:|
|04|mem_timing|:x:|
|05|mem_timing-2|:x:|
|06|oam_bug|:x:|

Gekkio's MBC1 Tests:

|#|name|state|
|-|-|-|
|01|bits_ram_en|:white_check_mark:|
|02|multicart_rom_8Mb|:x:|
|03|ram_64Kb|:white_check_mark:|
|04|ram_256Kb|:white_check_mark:|
|05|rom_512Kb|:white_check_mark:|
|06|rom_1MB|:white_check_mark:|
|07|rom_2Mb|:white_check_mark:|
|08|rom_4Mb|:white_check_mark:|
|09|rom_8Mb|:white_check_mark:|
|10|rom_16Mb|:white_check_mark:|

Gekkio's Acceptance Tests:

|#|name|state|
|-|-|-|
|01|add_sp_e_timing|:x:|
|02|boot_hwio-dmg0|:x:|
|03|boot_hwio-dmg ABCXmgb|:x:|
|04|boot_hwio-S|:x:|
|05|boot_regs-dmg0|:x:|
|06|boot_regs-dmgABCX|:white_check_mark:|
|07|call_cc_timing|:x:|
|08|call_cc_timing2|:x:|
|09|call timing|:x:|
|10|call_timing 2|:x:|
|11|di_timing-GS|:x:|
|12|div timing|:white_check_mark:|
|13|ei_sequence|:x:|
|14|ei_timing|:x:|
|15|halt_ime0_ei|:white_check_mark:|
|16|halt_ime0_nointr_timing|:x:|
|17|halt_ime1_timing|:white_check_mark:|
|18|halt_ime1_timing2-GS|:white_check_mark:|
|19|if_ie_registers|:white_check_mark:|
|20|intr_timing|:white_check_mark:|
|21|jp_cc_timing|:x:|
|22|jp timing|:x:|
|23|ld hl_sp_e timing|:x:|
|24|oam_dma_restart|:x:|
|25|oam_dma_start|:x:|
|26|oam_dma_timing|:x:|
|27|pop_timing|:x:|
|28|push_timing|:x:|
|29|rapid_di_ei|:x:|
|30|ret_cc_timing|:x:|
|31|ret_timing|:x:|
|32|reti_intr_timing|:x:|
|33|reti_timing|:x:|
|34|rst_timing|:x:|

Gekkio's Bits Acceptance Tests:

|#|name|state|
|-|-|-|
|01|mem_oam|:white_check_mark:|
|02|reg_f|:white_check_mark:|
|03|unused_hwio-GS|:x:|