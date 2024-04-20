#include "emu.h"

#include "cpu/g65816/g65816.h"
#include "sound/ay8910.h"
#include "video/tms9928a.h"

#include "machine/rescap.h"  // for set_resistors_load

namespace
{

  class gtvip_state : public driver_device
  {
  public:
    gtvip_state(const machine_config &mconfig, device_type type, const char *tag)
        : driver_device(mconfig, type, tag),
          m_maincpu(*this, "maincpu"),
          m_ymsnd(*this, "ym2149"),
          m_vdp(*this, "tms9918")
    // m_mainmemory(*this, "main_ram", 0x8000, ENDIANNESS_LITTLE)
    // m_mainmemory(*this, "main_ram", 0x10000, ENDIANNESS_LITTLE)
    {
    }

    void init() {}
    void gtvip(machine_config &config);

  private:
    required_device<g65816_device> m_maincpu;
    required_device<ym2149_device> m_ymsnd;
    required_device<tms9918_device> m_vdp;

    // memory_share_creator<u8> m_mainmemory;

    void main_memmap(address_map &map);
  };

  void gtvip_state::gtvip(machine_config &config)
  {
    G65816(config, m_maincpu, XTAL(8'000'000));
    m_maincpu->set_addrmap(AS_PROGRAM, &gtvip_state::main_memmap);

    // TODO: figure out how to set up sound chip
    YM2149(config, m_ymsnd, XTAL(4'000'000));
    m_ymsnd->set_flags(AY8910_SINGLE_OUTPUT);
    // m_ymsnd->set_resistors_load(RES_K(1), 0, 0);
    // m_ymsnd->port_a_write_callback().set(FUNC(st_state::psg_pa_w));
    // m_ymsnd->port_b_write_callback().set("cent_data_out", FUNC(output_latch_device::write));

    // example of how atari does it
    // YM2149(config, m_ymsnd, Y2/16);
    // m_ymsnd->set_flags(AY8910_SINGLE_OUTPUT);
    m_ymsnd->set_resistors_load(RES_K(1), 0, 0);
    // m_ymsnd->port_a_write_callback().set(FUNC(st_state::psg_pa_w));
    // m_ymsnd->port_b_write_callback().set("cent_data_out", FUNC(output_latch_device::write));

    // display chip
    // referenced colecovision which uses TMS9928A: src/mame/coleco/coleco.cpp
    // 10.738633MHz clock frequency and 16K vram
    // not sure about the SCREEN part but just copied it over
    TMS9918(config, m_vdp, XTAL(10'738'635));
    m_vdp->set_screen("screen");
    m_vdp->set_vram_size(0x4000);
    SCREEN(config, "screen", SCREEN_TYPE_RASTER);
  }

  // see MAME docs on memory: https://docs.mamedev.org/techspecs/memory.html
  void gtvip_state::main_memmap(address_map &map)
  {
    // 32kB SRAM
    map(0x0000, 0x7FFF).ram();

    // 32kB flash memory
    // some of this range is used for I/O (below)
    map(0x8000, 0xffff).rom();

    // I/O

    // display
    // DFC0: TMS9118 VRAM Access (aliased to DFC2, DFC4, DFC6) (not $C000 like in datasheet)
    // DFC1: TMS9118 Register Access (aliased to DFC3, DFC5, DFC7) (not $C002 like in datasheet)
    // referenced Tomy Tutor(?)g: src/mame/tomy/tutor.cpp
    // but referencing it as a device instead of tag for consistency
    map(0xdfc0, 0xdfc0).rw(m_vdp, FUNC(tms9918_device::vram_read), FUNC(tms9918_device::vram_write));         /*VDP data*/
    map(0xdfc1, 0xdfc1).rw(m_vdp, FUNC(tms9918_device::register_read), FUNC(tms9918_device::register_write)); /*VDP status*/

    // sound
    // DF10: Data Send for YM2149 soundchip #1
    // DF11: Register Select for YM2149 soundchip #1
    // DF12: Data Send for YM2149 soundchip #2
    // DF13: Register Select for YM2149 soundchip #2
    // should use DF14 and 15 instead of 12 and 13 for now to match the soundtest binary

    // example of how atari maps their sound chip
    // map(0xff8800, 0xff8800).after_delay(NAME([](offs_t) { return 1; })).rw(m_ymsnd, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w)).mirror(0xfc);
    // map(0xff8802, 0xff8802).after_delay(NAME([](offs_t) { return 1; })).rw(m_ymsnd, FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w)).mirror(0xfc);
    // map(0xff8800, 0xff8800).rw(m_ymsnd, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
    // map(0xff8802, 0xff8802).w(m_ymsnd, FUNC(ay8910_device::data_w));

    // example from src/mame/bandai/sv8000.cpp
    map(0xdf10, 0xdf10).w(m_ymsnd, FUNC(ay8910_device::data_w));
    map(0xdf11, 0xdf11).w(m_ymsnd, FUNC(ay8910_device::address_w));
    // map(0xc0, 0xc0).w("ay8910", FUNC(ay8910_device::data_w));
    // map(0xc1, 0xc1).w("ay8910", FUNC(ay8910_device::address_w));

    // controllers
    // DF00-DF03: Controller 1 through 4 inputs (aliased to DF04-DF07)
    // When Controller Select Pin output (P51, pin 4, J4-P5x connector) is 1: [C B C B Right Left Down Up]
    // When Controller Select Pin output (P51, pin 4, J4-P5x connector) is 0: [Start A Start A 0 0 Down Up]
  }

  INPUT_PORTS_START(gtvip_inputs)

  INPUT_PORTS_END

  ROM_START(gtvip)
  ROM_REGION(0x10000, "maincpu", 0)

  // HACK: for now, uncomment the ROM_LOAD line for the corresponding program to run
  // need to figure out a better way to handle roms
  // currently only the textdemo works

  // display test
  ROM_LOAD("textdemo.bin", 0x8000, 0x8000, CRC(4cf363dc) SHA1(bed707ec2ebb3e6cddfc6db58d78e436af05961a))

  // sound test
  // ROM_LOAD("soundtest.bin", 0x8000, 0x8000, CRC(7a828be3) SHA1(3b6487dbec7407e628b900877aae976f706a4d51))

  // monitor rom test
  // ROM_LOAD("monitor.bin", 0xE000, 0x2000, CRC(9575d641) SHA1(56ca218c0ed3d8fd631ee03690c0815b1441d0d4))

  ROM_END

} // namespace

COMP(2023, gtvip, 0, 0, gtvip, gtvip_inputs, gtvip_state, init, "GT", "GT VIP",
     MACHINE_NOT_WORKING)