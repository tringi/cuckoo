#ifndef CUCKOO_HPP
#define CUCKOO_HPP

#include <cstdint>

template <unsigned int Complexity = 25u,
          unsigned int CycleSize = 42u,
          unsigned int Easiness = 70u>
struct cuckoo {
    private:
        typedef std::uint32_t node_type; // TODO: uint64_t if Complexity > 31
        typedef std::uint64_t nonce_type;

        static constexpr auto Size = 1uLL << Complexity;
        static constexpr auto NodeMask = (Size / 2uLL) - 1uLL;
        static constexpr auto MaxPathLength = 8192;

    public:

        // memrq
        //  - memory requirements for this particular complexity

        static constexpr std::size_t memrq = (Size + 1) * sizeof (node_type);
        
    private:
        
        // graph
        //  - graph memory required for computation
        
        node_type * graph;
        
        // v
        //  - algorithm data
        
        std::uint64_t mutable v [4];

        // cycle
        //  - computed cuckoo cycle

        nonce_type cycle [CycleSize];
        
    public:
        // cuckoo constructor
        //  - memory must be at least 'memrq' bytes
        //  - if not set, 'compute' returns false
        
        cuckoo (void * m = nullptr)
            : graph (static_cast <node_type *> (m)) {};
        
        // find
        //  - pass SHA-256 hash of source data
        //  - works as long as necessary to find and compute new cycle
        //  - providing and then setting cancel to true terminates computation
        
        bool find (const void * hash, volatile bool * cancel = nullptr);
        
        // verify
        //  - that 'this->cycle' forms a cycle in 'hash'-generated graph
        //  - returns true/false
        
        bool verify (const void * hash) const;
        
        // load
        // serialize
        //  - 'cycle' from/into buffer of (CycleSize * NonceBytes) bytes
        
        template <unsigned int NonceBytes = 6u>
        void load (const unsigned char * buffer);
        
        template <unsigned int NonceBytes = 6u>
        void serialize (unsigned char * buffer) const;
    
    private:
        void init (const void * hash) const;
        
        node_type path (node_type * graph, node_type u, node_type * us, nonce_type max);
        
        template <unsigned int UV>
        nonce_type node (nonce_type) const;

        static std::uint64_t rotl (std::uint64_t, unsigned int);
        static void round (std::uint64_t &, std::uint64_t &, std::uint64_t &, std::uint64_t &);
};

#include "cuckoo.tcc"
#endif

