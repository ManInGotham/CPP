#include <windows.h>
#include <stdint.h>

#define WIN32_LEAN_AND_MEAN 1

#pragma comment(linker, "/NODEFAULTLIB")  
#pragma runtime_checks( "", off ) // https://msdn.microsoft.com/en-us/library/6kasb93x.aspx
#pragma comment(linker, "/STACK:167772160,167772160")
#pragma comment( user, "Compiled on " __DATE__ " at " __TIME__ ) 

#pragma comment(linker, "/ENTRY:WinMainCRTStartup")
#pragma comment( linker, "/subsystem:windows" )

    /*
    initialization and assignment of large arrays / structures*/

#include "float.h"
#include "math.h"
#include "memset.h"
#include "seh.h"

#include <stddef.h> // if you want size_t and NULL
#include <stdint.h> // various intXX_t and uintXX_t typedefs
#include <stdarg.h> // va_arg, va_start, va_end, va_arg intrinsics
#include <intrin.h> // few other headers for intrinsic functions (rdtsc, cpuid, SSE, SSE2, etc..)

//Remember that you are not allowed to use following features :
//    1) C++ RTTI( it's turned off by -GR- anyway)
//    2) C++ exceptions - try / catch / throw ( this it's turned off by -EHa-)
//    3) SEH exceptions - you could use them if you implement _C_specific_handler( for 64 - bit code ) and _except_handler3( for 32 - bit code ) functions.See simple expample how to do that by calling original C runtime functions in win32_crt_seh.cpp file.
//    4) Global objects with C++ constructors and destructors - it's possible to implement it, but it's not needed for us.
//    5) Pure virtual functions in C++ classes - for that you'll need to implement "__purecall" function, but we are also not interested in this.
//    6) No new / delete C++ operators, they are use global new / delete functions.You'll need to either override ne

class A
{
public:
    A()
    {
        int a = 5 + 3;
    }
};

class B : A
{
};

UINT T( HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int ShowCode )
{
    A a;

    char BigArray[ 4096 * 10 ];
    BigArray[ 0 ] = 0;
    char BigArray2[ 100 ] = {};

    float f = 1000.0f;
    double d = 1000000000.0;

    volatile int64_t s = 1;
    volatile uint64_t u = 1;

    s += s;
    s -= s;
    s *= s;
    s /= s;
    s %= s;
    s >>= 33;
    s <<= 33;

    u += u;
    u -= u;
    u *= u;
    u /= u;
    u %= u;
    u >>= 33;
    u <<= 33;


    return 0;
}


void __stdcall WinMainCRTStartup()
{
    UINT Result = T( GetModuleHandle( 0 ), 0, 0, 0 );
    ExitProcess( Result );
}


