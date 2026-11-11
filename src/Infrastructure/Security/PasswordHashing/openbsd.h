#ifndef ARC4RANDOM_H_INCLUDED
#define ARC4RANDOM_H_INCLUDED

#include <cstdlib>     /* srand, rand */
#include <cstddef>
#include <cstring>
#include <ctime>

#ifdef INVOKE_HAVE_SYSTEM_ARC4RANDOM_BUF
// The operating system initializes and supplies its own random source.
inline void arc4random_init(void) {}
#else

inline
void arc4random_buf(void *buf, size_t nbytes)
{

    for( size_t n = 0; n < nbytes; ++ n)
        ((char*)(buf))[n] = rand() %256;
}

inline
void arc4random_init(void)
{
    srand( (unsigned int) time(NULL));
}


#endif // INVOKE_HAVE_SYSTEM_ARC4RANDOM_BUF

#endif // ARC4RANDOM_H_INCLUDED
