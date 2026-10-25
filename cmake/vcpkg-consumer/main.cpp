// Minimal vcpkg consumer: encode an RS(255,223) codeword, inject a single
// error, and confirm the decoder corrects it.
#include <ezpwd/rs>

#include <cstdint>
#include <iostream>
#include <vector>

int main() {
    ezpwd::RS<255, 223> rs;

    std::vector<uint8_t> data(223);
    for (int i = 0; i < 223; i++)
        data[i] = (uint8_t)(i * 3 + 1);

    rs.encode(data);
    data[5] ^= 0xFF;               // inject a single-symbol error
    const int fixed = rs.decode(data);

    std::cout << "vcpkg consumer: fixed=" << fixed << std::endl;
    return fixed == 1 ? 0 : 1;
}
