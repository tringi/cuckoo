#ifndef CUCKOO_TCC
#define CUCKOO_TCC

#include <algorithm>
#include <cstring>
#include <cstdio>

template <unsigned int CycleSize, unsigned int Easiness>
bool cuckoo <CycleSize, Easiness> ::find (const void * hash,
                                          std::uint64_t round,
                                          volatile bool * cancel) {
    if (!this->graph)
        return false;
    
    this->init (hash, round);
    static constexpr std::size_t MaxPathLength = 8192;

    nonce_type us [MaxPathLength];
    nonce_type vs [MaxPathLength];
    
    std::memset (us, 0, sizeof us);
    std::memset (vs, 0, sizeof vs);
    
    for (nonce_type nonce = 0u; nonce != (Easiness * this->size / 100u) && !(cancel && *cancel); ++nonce) {
        
        auto u0 = this->node <0> (nonce);
        if (!u0)
            continue;
        
        auto v0 = this->node <1> (nonce);
        auto u = this->graph [u0];
        auto v = this->graph [v0];

        us [0] = u0;
        vs [0] = v0;
        
        auto nu = this->path (this->graph, u, us, MaxPathLength);
        auto nv = this->path (this->graph, v, vs, MaxPathLength);
        
        if (nu == node_type (-1) || nv == node_type (-1))
            continue;
        
        if (us [nu] == vs [nv]) {
            auto min = std::min (nu, nv);
            for (nu -= min, nv -= min; us [nu] != vs [nv]; nu++, nv++)
                ;
            
            if (nu + nv + 1 == CycleSize) {
                std::pair <node_type, node_type> raw [CycleSize] = {
                    { (node_type) us [0], (node_type) vs [0] }
                };

                nonce = 0;
                nonce_type n = 1;
                
                while (nu--) {
                    raw [n++] = { (node_type) us [(nu + 1) & ~1], (node_type) us [nu | 1] };
                };
                while (nv--) {
                    raw [n++] = { (node_type) vs [nv | 1], (node_type) vs [(nv + 1) & ~1] };
                };
                
                std::sort (&raw [0], &raw [CycleSize]);
                
                auto rawend = &raw [CycleSize];
                for (n = 0; nonce != Easiness * this->size / 100u; ++nonce) {
                    
                    std::pair <node_type, node_type> edge = {
                        (node_type) this->node <0> (nonce),
                        (node_type) this->node <1> (nonce)
                    };
                    auto ri = std::remove (&raw [0], rawend, edge);
                    if (ri != rawend) {
                        this->cycle [n++] = nonce;
                        --rawend;
                    };
                };
                
                return true; 
            };
            
            continue;
        };
        
        if (nu < nv) {
            while (nu--) {
                this->graph [us [nu + 1]] = (node_type) us [nu];
            };
            this->graph [u0] = (node_type) v0;
        } else {
            while (nv--) {
                this->graph [vs [nv + 1]] = (node_type) vs [nv];
            };
            this->graph [v0] = (node_type) u0;
        };
    };

    return false;
};

template <unsigned int CycleSize, unsigned int Easiness>
bool cuckoo <CycleSize, Easiness> ::verify (const void * hash, std::uint64_t round) const {
    this->init (hash, round);
    std::uint64_t uvs [2 * CycleSize];
    
    for (auto n = 0u; n != CycleSize; ++n) {
        if (this->cycle [n] >= (Easiness * this->size / 100u) || (n && this->cycle [n] <= this->cycle [n - 1]))
            return false;
        
        uvs [2 * n + 0] = this->node <0> (this->cycle [n]);
        uvs [2 * n + 1] = this->node <1> (this->cycle [n]);
    };
    
    auto i = 0u;
    for (auto n = CycleSize; n; ) {
        auto j = i;
        for (auto k = i & 1; k < 2 * CycleSize; k += 2)
            if (k != i && uvs [k] == uvs [i]) {
                if (j != i)
                    return false;
                j = k;
            };
            
        if (j == i)
            return false;
            
        i = j ^ 1;
        if (--n && i == 0)
            return false;
    };
    return i == 0;
};

template <unsigned int CycleSize, unsigned int Easiness>
    template <unsigned int NonceBytes>
void cuckoo <CycleSize, Easiness> ::load (const unsigned char * data) {
    for (auto n = 0u; n != CycleSize; ++n) {
        this->cycle [n] = 0;
        for (auto b = 0u; b != NonceBytes; ++b) {
            reinterpret_cast <unsigned char *> (&this->cycle [n]) [b] = *data++;
        };
    };
};

template <unsigned int CycleSize, unsigned int Easiness>
    template <unsigned int NonceBytes>
void cuckoo <CycleSize, Easiness> ::serialize (unsigned char * data) const {
    for (auto n = 0u; n != CycleSize; ++n) {
        for (auto b = 0u; b != NonceBytes; ++b) {
            *data++ = reinterpret_cast <const unsigned char *> (&this->cycle [n]) [b];
        };
    };
};

template <unsigned int CycleSize, unsigned int Easiness>
typename
cuckoo <CycleSize, Easiness> ::node_type
cuckoo <CycleSize, Easiness> ::path (node_type * graph, node_type u,
                                     nonce_type * us, nonce_type max) {
    node_type nu = 0;
    for (; u; u = graph [u]) {
        if (++nu >= max) {
            return -1;
        };
        us [nu] = u;
    };
    return nu;
};

template <unsigned int CycleSize, unsigned int Easiness>
void cuckoo <CycleSize, Easiness> ::init (const void * data, std::uint64_t round) const {
    auto key = static_cast <const std::uint64_t *> (data);
    this->v [0] = key [0] ^ round;
    this->v [1] = key [1] ^ round;
    this->v [2] = key [2] ^ round;
    this->v [3] = key [3] ^ round;
};

template <unsigned int CycleSize, unsigned int Easiness>
std::uint64_t cuckoo <CycleSize, Easiness> ::rotl (std::uint64_t x,
                                                   unsigned int b) {
    return (x << b) | (x >> (64u - b));
};

template <unsigned int CycleSize, unsigned int Easiness>
void cuckoo <CycleSize, Easiness> ::round (std::uint64_t & v0,
                                           std::uint64_t & v1,
                                           std::uint64_t & v2,
                                           std::uint64_t & v3) {
    v0 += v1;
    v2 += v3;
    v1 = rotl (v1, 13);
    v3 = rotl (v3, 16);
    v1 ^= v0;
    v3 ^= v2;
    v0 = rotl (v0, 32);
    v2 += v1;
    v0 += v3;
    v1 = rotl (v1, 17);
    v3 = rotl (v3, 21);
    v1 ^= v2;
    v3 ^= v0;
    v2 = rotl (v2, 32);
    return;
};

template <unsigned int CycleSize, unsigned int Easiness>
    template <unsigned int UV>
typename
cuckoo <CycleSize, Easiness> ::nonce_type
cuckoo <CycleSize, Easiness> ::node (nonce_type nonce) const {
    auto v0 = this->v [0];
    auto v1 = this->v [1];
    auto v2 = this->v [2];
    auto v3 = this->v [3] ^ ((nonce << 1) | UV);
    
    this->round (v0, v1, v2, v3);
    this->round (v0, v1, v2, v3);

    v0 ^= (nonce << 1) | UV;
    v2 ^= 0xff;

    this->round (v0, v1, v2, v3);
    this->round (v0, v1, v2, v3);
    this->round (v0, v1, v2, v3);
    this->round (v0, v1, v2, v3);
    
    return (((v0 ^ v1 ^ v2 ^ v3) & this->mask) << 1) | UV;
};

#endif
