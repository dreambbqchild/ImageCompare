#include "InstructionSet.h"

#include <iostream>
#include <vector>
#include <bitset>
#include <array>
#include <string>
#include <intrin.h>

class InstructionSet_Internal
{
public:
    InstructionSet_Internal()
        : nIds_{ 0 },
        nExIds_{ 0 },
        isIntel_{ false },
        isAMD_{ false },
        f_1_ECX_{ 0 },
        f_1_EDX_{ 0 },
        f_7_EBX_{ 0 },
        f_7_ECX_{ 0 },
        f_81_ECX_{ 0 },
        f_81_EDX_{ 0 },
        data_{},
        extdata_{}
    {
        std::array<int, 4> cpui;

        // Calling __cpuid with 0x0 as the function_id argument
        // gets the number of the highest valid function ID.
        __cpuid(cpui.data(), 0);
        nIds_ = cpui[0];

        for (int i = 0; i <= nIds_; ++i)
        {
            __cpuidex(cpui.data(), i, 0);
            data_.push_back(cpui);
        }

        // Capture vendor string
        char vendor[0x20];
        memset(vendor, 0, sizeof(vendor));
        *reinterpret_cast<int*>(vendor) = data_[0][1];
        *reinterpret_cast<int*>(vendor + 4) = data_[0][3];
        *reinterpret_cast<int*>(vendor + 8) = data_[0][2];
        vendor_ = vendor;
        if (vendor_ == "GenuineIntel")
        {
            isIntel_ = true;
        }
        else if (vendor_ == "AuthenticAMD")
        {
            isAMD_ = true;
        }

        // load bitset with flags for function 0x00000001
        if (nIds_ >= 1)
        {
            f_1_ECX_ = data_[1][2];
            f_1_EDX_ = data_[1][3];
        }

        // load bitset with flags for function 0x00000007
        if (nIds_ >= 7)
        {
            f_7_EBX_ = data_[7][1];
            f_7_ECX_ = data_[7][2];
        }

        // Calling __cpuid with 0x80000000 as the function_id argument
        // gets the number of the highest valid extended ID.
        __cpuid(cpui.data(), 0x80000000);
        nExIds_ = cpui[0];

        char brand[0x40];
        memset(brand, 0, sizeof(brand));

        for (int i = 0x80000000; i <= nExIds_; ++i)
        {
            __cpuidex(cpui.data(), i, 0);
            extdata_.push_back(cpui);
        }

        // load bitset with flags for function 0x80000001
        if (nExIds_ >= 0x80000001)
        {
            f_81_ECX_ = extdata_[1][2];
            f_81_EDX_ = extdata_[1][3];
        }

        // Interpret CPU brand string if reported
        if (nExIds_ >= 0x80000004)
        {
            memcpy(brand, extdata_[2].data(), sizeof(cpui));
            memcpy(brand + 16, extdata_[3].data(), sizeof(cpui));
            memcpy(brand + 32, extdata_[4].data(), sizeof(cpui));
            brand_ = brand;
        }
    };

    int nIds_;
    int nExIds_;
    std::string vendor_;
    std::string brand_;
    bool isIntel_;
    bool isAMD_;
    std::bitset<32> f_1_ECX_;
    std::bitset<32> f_1_EDX_;
    std::bitset<32> f_7_EBX_;
    std::bitset<32> f_7_ECX_;
    std::bitset<32> f_81_ECX_;
    std::bitset<32> f_81_EDX_;
    std::vector<std::array<int, 4>> data_;
    std::vector<std::array<int, 4>> extdata_;
};

InstructionSet_Internal CPU_Rep;

bool InstructionSet::SSE41(void) { return CPU_Rep.f_1_ECX_[19]; }
bool InstructionSet::AVX2(void) { return CPU_Rep.f_7_EBX_[5]; }