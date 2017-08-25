#ifndef CUCKOO_HPP
#define CUCKOO_HPP

#include <cstdint>

template <unsigned int CycleSize = 42u,
          unsigned int Easiness = 70u>
struct cuckoo {
    private:
        typedef std::uint32_t node_type; // TODO: uint64_t if Complexity > 32
        typedef std::uint64_t nonce_type;

    public:

        // memrq
        //  - memory requirements for this particular complexity

        static constexpr std::size_t memrq (std::uint8_t complexity) {
            return ((1uLL << complexity) + 1) * sizeof (node_type);
        }
        
    private:
        
        // graph
        //  - graph memory required for computation
        
        node_type * graph;
        
        // size/mask
        //  - agorithm properties

        std::uint64_t const size;
        std::uint64_t const mask;

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
        
        cuckoo (std::uint8_t complexity, void * m = nullptr)
            : size (1uLL << complexity)
            , mask (size / 2uLL - 1uLL)
            , graph (static_cast <node_type *> (m)) {};
        
        // find
        //  - pass SHA-256 hash of source data
        //  - works as long as necessary to find and compute new cycle
        //  - providing and then setting cancel to true terminates computation
        
        bool find (const void * hash, std::uint64_t round = 0, volatile bool * cancel = nullptr);
        
        // verify
        //  - that 'this->cycle' forms a cycle in 'hash'-generated graph
        //  - returns true/false
        
        bool verify (const void * hash, std::uint64_t round = 0) const;
        
        // load
        // serialize
        //  - 'cycle' from/into buffer of (CycleSize * NonceBytes) bytes
        
        template <unsigned int NonceBytes = 6u>
        void load (const unsigned char * buffer);
        
        template <unsigned int NonceBytes = 6u>
        void serialize (unsigned char * buffer) const;
    
    private:
        void init (const void * hash, std::uint64_t round) const;
        
        node_type path (node_type * graph, node_type u, nonce_type * us, nonce_type max);
        
        template <unsigned int UV>
        nonce_type node (nonce_type) const;

        static std::uint64_t rotl (std::uint64_t, unsigned int);
        static void round (std::uint64_t &, std::uint64_t &, std::uint64_t &, std::uint64_t &);
};

#include "cuckoo.tcc"
#endif

