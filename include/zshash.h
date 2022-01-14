#ifndef ZSYNC2_ZSHASH_H
#define ZSYNC2_ZSHASH_H

// system headers
#include <assert.h>
#include <memory>
#include <sstream>

// library headers
#include <gcrypt.h>

// own headers
#include "zsutil.h"

namespace zsync2 {
    /**
     * Minimal idiomatic C++ wrapper around libgcrypt's hashing library.
     */
    template<gcry_md_algos _algorithm>
    class ZSyncHash {
    private:
        static void _assertGcryptNoError(gcry_error_t error) {
            if (error != 0) {
                throw std::runtime_error(std::to_string(error));
            }
        }

    public:
        ZSyncHash() {
            _assertGcryptNoError(gcry_md_open(&_handle, _algorithm, 0));
        };

        explicit ZSyncHash(const std::string& data) : ZSyncHash() {
            add(data);
        }

        template<typename Vector>
        void add(const Vector& vector) {
            gcry_md_write(_handle, vector.data(), vector.size());
        }

        // this function may be called only once, the result will not change once the digest has been calculated
        std::string getHash() {
            const auto* buffer = gcry_md_read(_handle, _algorithm);
            const auto len = gcry_md_get_algo_dlen(_algorithm);

            assert(buffer != nullptr);

            return bytesToHex(buffer, len);
        }

        ~ZSyncHash() noexcept {
            gcry_md_close(_handle);
        }

    private:
        gcry_md_hd_t _handle{};
    };
}

#endif //ZSYNC2_ZSHASH_H
