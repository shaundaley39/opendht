/*
 *  Copyright (C) 2014-2015 Savoir-Faire Linux Inc.
 *  Author : Adrien Béraud <adrien.beraud@savoirfairelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.
 *
 *  Additional permission under GNU GPL version 3 section 7:
 *
 *  If you modify this program, or any covered work, by linking or
 *  combining it with the OpenSSL project's OpenSSL library (or a
 *  modified version of that library), containing parts covered by the
 *  terms of the OpenSSL or SSLeay licenses, Savoir-Faire Linux Inc.
 *  grants you additional permission to convey the resulting work.
 *  Corresponding Source for a non-source form of such a combination
 *  shall include the source code for the parts of OpenSSL used as well
 *  as that of the covered work.
 */

#include "rng.h"

#include <chrono>
#include <cstring>

namespace dht {
namespace crypto {

random_device::random_device() :
   has_rdrand(hasRdrand()),
   has_rdseed(hasRdseed()),
   gen(std::chrono::system_clock::now().time_since_epoch().count())
{}

random_device::result_type
random_device::operator()()
{
    result_type prand = dis(gen);
    result_type hwrand;
    if (has_rdseed and rdseed(&hwrand))
        prand ^= hwrand;
    else if (has_rdrand and rdrand(&hwrand))
        prand ^= hwrand;
    return prand;
}

void
random_device::CPUIDinfo::get(const unsigned int func, const unsigned int subfunc)
{
    __asm__ __volatile__ (
        "cpuid"
        : "=a"(EAX), "=b"(EBX), "=c"(ECX), "=d"(EDX)
        : "a"(func), "c"(subfunc)
    );
}

bool
random_device::hasIntelCpu()
{
    CPUIDinfo info;
    info.get(0, 0);
    if (memcmp((char *) (&info.EBX), "Genu", 4) == 0
     && memcmp((char *) (&info.EDX), "ineI", 4) == 0
     && memcmp((char *) (&info.ECX), "ntel", 4) == 0) {
        return true;
    }
    return false;
}

bool
random_device::hasRdrand()
{
    if (!hasIntelCpu())
        return false;

    CPUIDinfo info;
    info.get(1, 0);
    static const constexpr unsigned int RDRAND_FLAG = (1 << 30);
    if ((info.ECX & RDRAND_FLAG) == RDRAND_FLAG)
        return true;
    return false;
}

bool
random_device::hasRdseed()
{
    if (!hasIntelCpu())
        return false;

    CPUIDinfo info;
    info.get(7, 0);
    static const constexpr unsigned int RDSEED_FLAG = (1 << 18);
    if ((info.ECX & RDSEED_FLAG) == RDSEED_FLAG)
        return true;
    return false;
}

bool
random_device::rdrandStep(result_type* r)
{
    unsigned char ok;
    asm volatile ("rdrand %0; setc %1"
        : "=r" (*r), "=qm" (ok));
    return ok;
}

bool
random_device::rdrand(result_type* r)
{
    result_type res;
    unsigned retries = 8;
    while (retries--)
        if (rdrandStep(&res)) {
            *r = res;
            return true;
        }
    return false;
}

bool
random_device::rdseedStep(result_type* r)
{
    unsigned char ok;
    asm volatile ("rdseed %0; setc %1"
        : "=r" (*r), "=qm" (ok));
    return ok;
}

bool
random_device::rdseed(result_type* r)
{
    result_type res;
    unsigned retries = 256;
    while (retries--)
        if (rdseedStep(&res)) {
            *r = res;
            return true;
        }
    return false;
}

}}
