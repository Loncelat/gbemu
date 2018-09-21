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
|02|boot_hwio-dmgABCXmgb|:white_check_mark:|
|03|boot_regs-dmgABCX|:white_check_mark:|
|04|call_cc_timing|:x:|
|05|call_cc_timing2|:x:|
|06|call timing|:x:|
|07|call_timing 2|:x:|
|08|di_timing-GS|:white_check_mark:|
|09|div timing|:white_check_mark:|
|10|ei_sequence|:white_check_mark:|
|11|ei_timing|:white_check_mark:|
|12|halt_ime0_ei|:white_check_mark:|
|13|halt_ime0_nointr_timing|:x:|
|14|halt_ime1_timing|:white_check_mark:|
|15|halt_ime1_timing2-GS|:white_check_mark:|
|16|if_ie_registers|:white_check_mark:|
|17|intr_timing|:x:|
|18|jp_cc_timing|:x:|
|19|jp timing|:x:|
|20|ld hl_sp_e timing|:x:|
|21|oam_dma_restart|:x:|
|22|oam_dma_start|:x:|
|23|oam_dma_timing|:x:|
|24|pop_timing|:x:|
|25|push_timing|:x:|
|26|rapid_di_ei|:white_check_mark:|
|27|ret_cc_timing|:x:|
|28|ret_timing|:x:|
|29|reti_intr_timing|:x:|
|30|reti_timing|:x:|
|31|rst_timing|:x:|

Gekkio's PPU Tests:

|#|name|state|
|-|-|-|
|01|hblank_ly_scx_timing-GS|:x:|
|02|intr_1_2_timing-GS|:white_check_mark:|
|03|intr_2_0_timing|:white_check_mark:|
|04|intr_2_mode0_timing|:x:|
|05|intr_2_mode0_timing_sprites|:x:|
|06|intr_2_mode3_timing|:x:|
|07|intr_2_oam_ok_timing|:x:|
|08|lcdon_timing-dmgABCmgbS|:x:|
|09|lcdon_write_timing-GS|:x:|
|10|stat_irq_blocking|:white_check_mark:|
|11|stat_lyc_onoff|:x:|
|12|vblank_stat_intr-GS|:white_check_mark:|

Gekkio's Bits Acceptance Tests:

|#|name|state|
|-|-|-|
|01|mem_oam|:white_check_mark:|
|02|reg_f|:white_check_mark:|
|03|unused_hwio-GS|:x:|