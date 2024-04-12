#include "emu.h"

#include "cpu/g65816/g65816.h"
#include "sound/ay8910.h"
#include "video/tms9928a.h"

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

    YM2149(config, m_ymsnd, XTAL(4'000'000));
    m_ymsnd->set_flags(AY8910_SINGLE_OUTPUT);
    // m_ymsnd->set_resistors_load(RES_K(1), 0, 0);
    // m_ymsnd->port_a_write_callback().set(FUNC(st_state::psg_pa_w));
    // m_ymsnd->port_b_write_callback().set("cent_data_out", FUNC(output_latch_device::write));

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

    // controllers
    // DF00-DF03: Controller 1 through 4 inputs (aliased to DF04-DF07)
    // When Controller Select Pin output (P51, pin 4, J4-P5x connector) is 1: [C B C B Right Left Down Up]
    // When Controller Select Pin output (P51, pin 4, J4-P5x connector) is 0: [Start A Start A 0 0 Down Up]
    

  }

  INPUT_PORTS_START(gtvip_inputs)

  INPUT_PORTS_END

  ROM_START(gtvip)
  ROM_REGION(0x10000, "maincpu", 0)
  ROM_LOAD("textdemo.bin", 0x8000, 0x8000, CRC(4cf363dc) SHA1(bed707ec2ebb3e6cddfc6db58d78e436af05961a))
  ROM_END

} // namespace

COMP(2023, gtvip, 0, 0, gtvip, gtvip_inputs, gtvip_state, init, "GT", "GT VIP",
     MACHINE_NOT_WORKING)