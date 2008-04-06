/*
 * resid.cc - reSID interface code.
 *
 * Written by
 *  Teemu Rantanen <tvr@cs.hut.fi>
 *  Dag Lem <resid@nimrod.no>
 *  Andreas Boose <viceteam@t-online.de>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "resid/sid.h"

extern "C" {

#include <string.h>

#include "sid/sid.h" /* sid_engine_t */
#include "lib.h"
#include "log.h"
#include "resid.h"
#include "resources.h"
#include "sid-snapshot.h"
#include "sound.h"
#include "types.h"

struct sound_s
{
    /* resid sid implementation */
    SID	sid;
};


static sound_t *resid_open(BYTE *sidstate)
{
    sound_t *psid;
    int	i;

    psid = new sound_t;

    for (i = 0x00; i <= 0x18; i++) {
	psid->sid.write(i, sidstate[i]);
    }

    return psid;
}

static int resid_init(sound_t *psid, int speed, int cycles_per_sec)
{
    sampling_method method;
    char method_text[100];
    double passband;
    int filters_enabled, model, sampling, passband_percentage;

    if (resources_get_value("SidFilters", (void *)&filters_enabled) < 0)
        return 0;

    if (resources_get_value("SidModel", (void *)&model) < 0)
        return 0;

    if (resources_get_value("SidResidSampling", (void *)&sampling) < 0)
        return 0;

    if (resources_get_value("SidResidPassband",
        (void *)&passband_percentage) < 0)
        return 0;

    passband = speed * passband_percentage / 200.0;

    psid->sid.set_chip_model(model == 0 ? MOS6581 : MOS8580);

    /* 8580 + digi boost. */
    psid->sid.input(model == 2 ? -32768 : 0);

    psid->sid.enable_filter(filters_enabled ? true : false);
    psid->sid.enable_external_filter(filters_enabled ? true : false);

    switch (sampling) {
      default:
      case 0:
        method = SAMPLE_FAST;
	strcpy(method_text, "fast");
	break;
      case 1:
        method = SAMPLE_INTERPOLATE;
	strcpy(method_text, "interpolating");
	break;
      case 2:
        method = SAMPLE_RESAMPLE_INTERPOLATE;
	sprintf(method_text, "resampling, pass to %dHz", (int)passband);
	break;
      case 3:
        method = SAMPLE_RESAMPLE_FAST;
	sprintf(method_text, "resampling, pass to %dHz", (int)passband);
	break;
    }

    if (!psid->sid.set_sampling_parameters(cycles_per_sec, method,
					   speed, passband)) {
        log_warning(LOG_DEFAULT,
                    "reSID: Out of spec, increase sampling rate or decrease maximum speed");
	return 0;
    }

    log_message(LOG_DEFAULT, "reSID: %s, filter %s, sampling rate %dHz - %s",
		model == 0 ? "MOS6581" : "MOS8580",
		filters_enabled ? "on" : "off",
		speed, method_text);

    return 1;
}

static void resid_close(sound_t *psid)
{
    delete psid;
}

static BYTE resid_read(sound_t *psid, WORD addr)
{
    return psid->sid.read(addr);
}

static void resid_store(sound_t *psid, WORD addr, BYTE byte)
{
    psid->sid.write(addr, byte);
}

static void resid_reset(sound_t *psid, CLOCK cpu_clk)
{
    psid->sid.reset();
}

static int resid_calculate_samples(sound_t *psid, SWORD *pbuf, int nr,
                                   int interleave, int *delta_t)
{
    return psid->sid.clock(*delta_t, pbuf, nr, interleave);
}

static void resid_prevent_clk_overflow(sound_t *psid, CLOCK sub)
{
}

static char *resid_dump_state(sound_t *psid)
{
    return lib_stralloc("");
}

static void resid_state_read(sound_t *psid, sid_snapshot_state_t *sid_state)
{
    SID::State state;
    unsigned int i;

    state = psid->sid.read_state();

    for (i = 0; i < 0x20; i++) {
        sid_state->sid_register[i] = (BYTE)state.sid_register[i];
    }

    sid_state->bus_value = (BYTE)state.bus_value;
    sid_state->bus_value_ttl = (DWORD)state.bus_value_ttl;
    for (i = 0; i < 3; i++) {
        sid_state->accumulator[i] = (DWORD)state.accumulator[i];
        sid_state->shift_register[i] = (DWORD)state.shift_register[i];
        sid_state->rate_counter[i] = (WORD)state.rate_counter[i];
        sid_state->rate_counter_period[i] = (WORD)state.rate_counter_period[i];
        sid_state->exponential_counter[i] = (WORD)state.exponential_counter[i];
        sid_state->exponential_counter_period[i] = (WORD)state.exponential_counter_period[i];
        sid_state->envelope_counter[i] = (BYTE)state.envelope_counter[i];
        sid_state->envelope_state[i] = (BYTE)state.envelope_state[i];
        sid_state->hold_zero[i] = (BYTE)state.hold_zero[i];
    }
}

static void resid_state_write(sound_t *psid, sid_snapshot_state_t *sid_state)
{
    SID::State state;
    unsigned int i;

    for (i = 0; i < 0x20; i++) {
        state.sid_register[i] = (char)sid_state->sid_register[i];
    }

    state.bus_value = (reg8)sid_state->bus_value;
    state.bus_value_ttl = (cycle_count)sid_state->bus_value_ttl;
    for (i = 0; i < 3; i++) {
        state.accumulator[i] = (reg24)sid_state->accumulator[i];
        state.shift_register[i] = (reg24)sid_state->shift_register[i];
        state.rate_counter[i] = (reg16)sid_state->rate_counter[i];
	if (sid_state->rate_counter_period[i])
            state.rate_counter_period[i] = (reg16)sid_state->rate_counter_period[i];
        state.exponential_counter[i] = (reg16)sid_state->exponential_counter[i];
	if (sid_state->exponential_counter_period[i])
            state.exponential_counter_period[i] = (reg16)sid_state->exponential_counter_period[i];
        state.envelope_counter[i] = (reg8)sid_state->envelope_counter[i];
        state.envelope_state[i] = (EnvelopeGenerator::State)sid_state->envelope_state[i];
        state.hold_zero[i] = (sid_state->hold_zero[i] != 0);
    }

    psid->sid.write_state((const SID::State)state);
}

sid_engine_t resid_hooks =
{
    resid_open,
    resid_init,
    resid_close,
    resid_read,
    resid_store,
    resid_reset,
    resid_calculate_samples,
    resid_prevent_clk_overflow,
    resid_dump_state,
    resid_state_read,
    resid_state_write
};

} // extern "C"
