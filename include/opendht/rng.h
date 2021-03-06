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

#include <random>

namespace dht {
namespace crypto {

#ifdef _WIN32

/**
 * Hardware random rumber generator using Intel RDRAND/RDSEED,
 * API-compatible with std::random_device.
 */
class random_device {
public:
    using result_type = std::random_device::result_type;
    using pseudo_engine = std::mt19937_64;

    /**
     * Current implementation assumption : result_type must be of a size
     * supported by Intel RDRAND/RDSEED.
     * result_type is unsigned int so this is currently safe.
     */
    static_assert(
        sizeof(result_type) == 2 || 
        sizeof(result_type) == 4 || 
        sizeof(result_type) == 8,
        "result_type must be 16, 32 or 64 bits");

    random_device();

    result_type operator()();

    static constexpr result_type min() {
        return std::numeric_limits<result_type>::lowest();
    }

    static constexpr result_type max() {
        return std::numeric_limits<result_type>::max();
    }

    double entropy() const {
        if (has_rdrand or has_rdseed)
            return 1.;
        return 0.;
    }

private:
    random_device& operator=(random_device&) = delete;

    const bool has_rdrand;
    const bool has_rdseed;
    pseudo_engine gen;
    std::uniform_int_distribution<result_type> dis {};

    struct CPUIDinfo {
        unsigned int EAX;
        unsigned int EBX;
        unsigned int ECX;
        unsigned int EDX;
        void get(const unsigned int func, const unsigned int subfunc);
    };
    static bool hasIntelCpu();
    static bool hasRdrand();
    static bool hasRdseed();
    bool rdrandStep(result_type* r);
    bool rdrand(result_type* r);
    bool rdseedStep(result_type* r);
    bool rdseed(result_type* r);
};

#else

using random_device = std::random_device;

#endif

}} // dht::crypto
