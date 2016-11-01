#include <windows.h> 
#include <csignal> 
#include <cstdio>

#include "cuckoo.hpp"
#include "sha256.h"

constexpr auto complexity = 26u;

volatile bool quit = false;
void c_signal_handler (int);

int main (int argc, char ** argv) {
    std::signal (SIGTERM, c_signal_handler);
    std::signal (SIGINT,  c_signal_handler);

    if (argc >= 2) {
        if (auto data = VirtualAlloc (NULL, cuckoo <complexity> ::memrq, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)) {
    
            // CryptCreateHash (..., CALG_SHA_256, NULL, NULL, ...)
            sha256 sha;
            sha256_initialize (&sha);
            sha256_finalize (&sha, (const unsigned char *) argv [1], strlen (argv [1]));
            
            cuckoo <complexity> cc (data);
            
            std::printf("computing... ");
    
            auto t = GetTickCount ();
            if (cc.find (&sha, &quit)) {
                
                std::printf("done in %.1fs, verifying... ", (GetTickCount () - t) / 1000.0);
                
                if (cc.verify (&sha)) {
                    
                    std::printf("okay, testing broken... ");
                    
                    unsigned char buffer [42*6];
                    cc.serialize (buffer);
                    ++buffer [0];
                    cc.load (buffer);
        
                    if (!cc.verify (&sha)) {
                        std::printf ("OK!\n");
                    } else {
                        std::printf ("FAILED!\n");
                    };
                } else {
                    std::printf ("FAILED!\n");
                };
            } else {
                if (!quit) {
                    std::printf("FAILED BAD!\n");
                };
            };
            
            VirtualFree (data, 0, MEM_RELEASE);
        } else {
            std::printf("out of memory.\n");
        };
    };
    return 0;
};

void c_signal_handler (int) {
    quit = true;
    std::printf ("cancelled\n");
    return;
};

