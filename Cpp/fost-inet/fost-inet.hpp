#ifdef FOST_OS_WINDOWS
#define FOST_INET_DECLSPEC __declspec(dllexport)
#else
#define FOST_INET_DECLSPEC
#endif


#include <fost/core>


namespace fostlib {
    extern const module c_fost_inet;
}
