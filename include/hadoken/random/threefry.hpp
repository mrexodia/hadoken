#   include <algorithm>/**
 * Copyright (c) 2016, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 * Boost Software License - Version 1.0
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
*
*/


//
// This work is derivated from the boost.Random123
// repository accessible here https://github.com/DEShawResearch/Random123-Boost
//
//

#ifndef _HADOKEN_RANDOM_THREEFRY_
#define _HADOKEN_RANDOM_THREEFRY_


#include <numeric>
#include <cstdint>
#include <random>


#include <hadoken/config/platform_config.hpp>

#ifdef HADOKEN_COMPILER_IS_NVCC
#   include <hadoken/gpu/algorithm.hpp>
#   include <hadoken/gpu/array.hpp>
#else
#   include <algorithm>
#   include <array>

#endif

///
///  threefry is a state-less counter base random generator
///  derivated from the block cipher threefish
///  part of the "The Skein Hash Function Family" by
///   Niels Ferguson, Stefan Lucks, Bruce Schneier, Doug Whiting
///   Mihir Bellare, Tadayoshi Kohno, Jon Callas, Jesse Walker (
///
///   threefry has been presented at SC11 in the publication
///
/// "Parallel random numbers: as easy as 1, 2, 3".
///    John K. Salmon, Mark A. Moraes, Ron O. Dror, David E. Shaw" (doi:10.1145/2063384.2063405)
///
///  This implementation is freely inspired of Boost.Random123  (https://github.com/DEShawResearch/Random123-Boost )
///

namespace hadoken{



namespace utils{
#ifdef  HADOKEN_COMPILER_IS_NVCC
    using namespace gpu;
#else    
    using namespace std;
#endif    
};   


namespace impl {

// threefry_constants is an abstract template that will be
// specialized with the KS_PARITY and Rotation constants
// of the threefry generators.  These constants are carefully
// chosen to achieve good randomization.
//  threefry_constants<2, uint32_t>
//  threefry_constants<2, uint64_t>
//  threefry_constants<4, uint32_t>
//  threefry_constants<4, uint64_t>
// The constants here are from Salmon et al <FIXME REF>.
//
// See Salmon et al, or Schneier's original work on Threefish <FIXME
// REF> for information about how the constants were obtained.
template <unsigned _N, typename Uint>
struct threefry_constants{
};

// 2x32 constants
template <>
struct threefry_constants<2, uint32_t>{

    HADOKEN_DECORATE_HOST_DEVICE
    static constexpr int32_t ks_parity(){
        return UINT32_C(0x1BD11BDA);
    }

    HADOKEN_DECORATE_HOST_DEVICE
    static inline unsigned rotations(int pos){
        constexpr unsigned rotations[8] = {13, 15, 26, 6, 17, 29, 16, 24};
        return rotations[pos];
    }
};


// 4x32 contants
template <>
struct threefry_constants<4, uint32_t>{

    HADOKEN_DECORATE_HOST_DEVICE
    static constexpr int32_t ks_parity(){
        return UINT32_C(0x1BD11BDA);
    }

    HADOKEN_DECORATE_HOST_DEVICE
    static inline unsigned rotations0(int pos){
        constexpr unsigned rotations[8] = {10, 11, 13, 23, 6, 17, 25, 18};
        return rotations[pos];
    }

    HADOKEN_DECORATE_HOST_DEVICE
    static inline unsigned rotations1(int pos){
        constexpr unsigned rotations[8] = {26, 21, 27, 5, 20, 11, 10, 20};
        return rotations[pos];
    }
};




// 2x64 constants
template <>
struct threefry_constants<2, uint64_t>{

    HADOKEN_DECORATE_HOST_DEVICE
    static constexpr uint64_t ks_parity(){
        return UINT64_C(0x1BD11BDAA9FC1A22);
    }

    HADOKEN_DECORATE_HOST_DEVICE
    static inline unsigned rotations(int pos){
        constexpr unsigned rotations[8] = {16, 42, 12, 31, 16, 32, 24, 21};
        return rotations[pos];
    }
};



// 4x64 constants
template <>
struct threefry_constants<4, uint64_t>{

    HADOKEN_DECORATE_HOST_DEVICE
    static constexpr uint64_t ks_parity(){
        return UINT64_C(0x1BD11BDAA9FC1A22);
    }

    HADOKEN_DECORATE_HOST_DEVICE
    static inline unsigned rotations0(int pos){
        constexpr unsigned rotations[8] = {14, 52, 23, 5, 25, 46, 58, 32};
        return rotations[pos];
    }

    HADOKEN_DECORATE_HOST_DEVICE
    static inline unsigned rotations1(int pos){
        constexpr unsigned rotations[8] = {16, 57, 40, 37, 33, 12, 22, 32};
        return rotations[pos];
    }
};

template <typename Uint>
HADOKEN_DECORATE_HOST_DEVICE
inline Uint threefry_rotl(Uint x, unsigned s){
    return (x<<s) | (x>>(std::numeric_limits<Uint>::digits-s));
}



/// the number of rounds is known at compile time
/// It allows us to use recursive partial template
/// specialization and to avoid branching on conditions
///
/// good compilers (GCC > 4.8, icc, clang ) transform this into
/// a single function without branching
///
/// we get a performance gain x4 on Intel I7 compared to a loop version
///

template <std::size_t r_remain, std::size_t r_max,
          typename Uint, typename Domain, typename Constants, std::size_t N>
struct rounds_functor{
    static_assert( N==2 || N==4, "number of rounds should be 2 or 4");
};

template <std::size_t r_remain, std::size_t r_max,
          typename Uint, typename Domain, typename Constants>
struct rounds_functor<r_remain, r_max, Uint, Domain, Constants, 4>{
    typedef Uint uint_type;
    typedef Domain domain_type;

    HADOKEN_DECORATE_HOST_DEVICE
    inline void operator() (const utils::array<uint_type, 5> & ks,
                        domain_type & c){
        constexpr std::size_t r = r_max - r_remain;

        if( ( r & 0x01)  ){
            c[0] += c[3];
            c[2] += c[1];
            c[3] = threefry_rotl(c[3],Constants::rotations0(r%8)) ^ c[0];
            c[1] = threefry_rotl(c[1],Constants::rotations1(r%8)) ^ c[2];

            const std::size_t r_next = r+1;
            const std::size_t r4 = r_next>>2;
            const std::size_t r_next_mod_4 = r_next%4;

            if( r_next_mod_4  == 0 ){
                c[0] += ks[(r4+0)%5];
                c[1] += ks[(r4+1)%5];
                c[2] += ks[(r4+2)%5];
                c[3] += ks[(r4+3)%5] + r4;
            }

        }else{
            c[0] += c[1];
            c[2] += c[3];
            c[1] = threefry_rotl(c[1], Constants::rotations0(r%8)) ^ c[0];
            c[3] = threefry_rotl(c[3], Constants::rotations1(r%8)) ^ c[2];

        }
        rounds_functor<r_remain-1, r_max, uint_type, domain_type, Constants, 4> func;
        func(ks, c);
        return;
    }


};

template <std::size_t r_max, typename Uint, typename Domain, typename Constants>
struct rounds_functor<0, r_max, Uint, Domain, Constants, 4>{
    typedef Uint uint_type;
    typedef Domain domain_type;

    HADOKEN_DECORATE_HOST_DEVICE
    inline void operator() (const utils::array<uint_type, 5> & ks,
                        domain_type & c){
        (void) ks;
        (void) c;
    }

};


template <std::size_t r_remain, std::size_t r_max,
          typename Uint, typename Domain, typename Constants>
struct rounds_functor<r_remain, r_max, Uint, Domain, Constants, 2>{
    typedef Uint uint_type;
    typedef Domain domain_type;

    HADOKEN_DECORATE_HOST_DEVICE
    inline void operator() (const utils::array<uint_type, 3> & ks,
                        domain_type & c){
        constexpr std::size_t r = r_max - r_remain;

        c[0] += c[1];
        c[1] = threefry_rotl(c[1], Constants::rotations(r%8));
        c[1] ^= c[0];


        constexpr std::size_t r_next = r+1;
        constexpr std::size_t r4 = r_next>>2;
        constexpr std::size_t r_next_mod_4 = r_next%4;

        if(r_next_mod_4  == 0){
            c[0] += ks[r4%3];
            c[1] += ks[(r4+1)%3] + r4;
        }

        rounds_functor<r_remain-1, r_max, uint_type, domain_type, Constants, 2> func;
        func(ks, c);
        return;
    }


};


template <std::size_t r_max, typename Uint, typename Domain, typename Constants>
struct rounds_functor<0, r_max, Uint, Domain, Constants, 2>{
    typedef Uint uint_type;
    typedef Domain domain_type;

    HADOKEN_DECORATE_HOST_DEVICE
    inline void operator() (const utils::array<uint_type, 3> & ks,
                        domain_type & c){
        (void) ks;
        (void) c;
    }

};

} // impl

template <unsigned N, typename Uint, unsigned R=20, typename Constants=impl::threefry_constants<N, Uint> >
class threefry{
    static_assert( N==2 || N==4, "number of rounds should be 2 or 4");
public:
    typedef utils::array<Uint, N> domain_type;
    typedef utils::array<Uint, N> range_type;
    typedef utils::array<Uint, N> key_type;
    typedef Uint                  uint_type;

    HADOKEN_DECORATE_HOST_DEVICE
    explicit threefry() : k(){}
    HADOKEN_DECORATE_HOST_DEVICE
    explicit threefry(key_type _k) : k(_k) {}

    HADOKEN_DECORATE_HOST_DEVICE
    threefry(const threefry&)  = default;
    HADOKEN_DECORATE_HOST_DEVICE
    threefry(threefry &&) = default;

    HADOKEN_DECORATE_HOST_DEVICE
    threefry& operator =(const threefry &) = default;
    HADOKEN_DECORATE_HOST_DEVICE
    threefry& operator =(threefry && ) = default;

    HADOKEN_DECORATE_HOST_DEVICE
    void set_key(key_type _k){
        k = _k;
    }

    HADOKEN_DECORATE_HOST_DEVICE
    key_type get_key() const{
        return k;
    }

    HADOKEN_DECORATE_HOST_DEVICE
    bool operator==(const threefry& rhs) const{
        return k == rhs.k;
    }

    HADOKEN_DECORATE_HOST_DEVICE
    bool operator!=(const threefry& rhs) const{
        return k != rhs.k;
    }


    HADOKEN_DECORATE_HOST_DEVICE
    inline range_type operator()(const domain_type & counter){
        using namespace impl;
        utils::array<uint_type, N+1>  ks;
        domain_type c(counter);

        utils::copy(k.begin(), k.end(), ks.begin());
        ks[N] = utils::accumulate(k.begin(), k.end(), Constants::ks_parity(), utils::bit_xor<uint_type>());
        utils::transform(k.begin(), k.end(), c.begin(), c.begin(), utils::plus<uint_type>());

        rounds_functor<R, R, uint_type, domain_type, Constants, N> func;
        func(ks, c);

        return c;
    }


private:

    key_type k;
};




typedef threefry<4, std::uint64_t> threefry4x64;
typedef threefry<2, std::uint64_t> threefry2x64;


typedef threefry<4, std::uint32_t> threefry4x32;
typedef threefry<2, std::uint32_t> threefry2x32;

/// threefry4x64 is crush-resistant and the fastest one most
/// of the current platforms: use it by default
typedef threefry<4, std::uint64_t> threefry_default;


}

#endif // _HADOKEN_RANDOM_THREEFRY_
