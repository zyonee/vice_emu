//  ---------------------------------------------------------------------------
//  This file is part of reSID, a MOS6581 SID emulator engine.
//  Copyright (C) 2004  Dag Lem <resid@nimrod.no>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  ---------------------------------------------------------------------------

#define __FILTER_CC__
#include "filter.h"

// Maximum cutoff frequency is specified as
// FCmax = 2.6e-5/C = 2.6e-5/2200e-12 = 11818.
//
// Measurements indicate a cutoff frequency range of approximately
// 220Hz - 18kHz on a MOS6581 fitted with 470pF capacitors. The function
// mapping FC to cutoff frequency has the shape of the tanh function, with
// a discontinuity at FCHI = 0x80.
// In contrast, the MOS8580 almost perfectly corresponds with the
// specification of a linear mapping from 30Hz to 12kHz.
// 
// The mappings have been measured by feeding the SID with an external
// signal since the chip itself is incapable of generating waveforms of
// higher fundamental frequency than 4kHz. It is best to use the bandpass
// output at full resonance to pick out the cutoff frequency at any given
// FC setting.
//
// The mapping function is specified with spline interpolation points and
// the function values are retrieved via table lookup.
//
// NB! Cutoff frequency characteristics may vary, we have modeled two
// particular Commodore 64s.

// ----------------------------------------------------------------------------
// Constructor.
// ----------------------------------------------------------------------------
Filter::Filter()
{
  reset();
}

// ----------------------------------------------------------------------------
// SID reset.
// ----------------------------------------------------------------------------
void Filter::reset()
{
  fc = 0;
  res = 0;
  filt = 0;
  voice3off = 0;
  hp_bp_lp = 0;
  vol = 0;
  Vnf = 0;
}

// ----------------------------------------------------------------------------
// Register functions.
// ----------------------------------------------------------------------------
void Filter::writeFC_LO(reg8 fc_lo)
{
  fc = fc & 0x7f8 | fc_lo & 0x007;
}

void Filter::writeFC_HI(reg8 fc_hi)
{
  fc = (fc_hi << 3) & 0x7f8 | fc & 0x007;
}

void Filter::writeRES_FILT(reg8 res_filt)
{
  res = (res_filt >> 4) & 0x0f;
  filt = res_filt & 0x0f;
}

void Filter::writeMODE_VOL(reg8 mode_vol)
{
  voice3off = mode_vol & 0x80;
  hp_bp_lp = (mode_vol >> 4) & 0x07;
  vol = mode_vol & 0x0f;
}
