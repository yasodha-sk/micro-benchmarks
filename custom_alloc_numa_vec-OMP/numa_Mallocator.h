// Example from cppreference
// https://en.cppreference.com/cpp/named_req/Allocator
//
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <numa.h>
#include <numaif.h>

using namespace std;

template<class T>
class numa_Mallocator
{
	public:
    typedef T value_type;
    
    numa_Mallocator() = default;
    
    template<class U>
    constexpr numa_Mallocator(const numa_Mallocator <U>&) noexcept {}
    
    [[nodiscard]] T* allocate(std::size_t n)
    {
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
            throw std::bad_array_new_length();
        
        if (auto p = static_cast<T*>(numa_alloc_onnode(n * sizeof(T),1)))
        {
            //report(p, n);
            return p;
        }
        
        throw std::bad_alloc();
    }
    
    void deallocate(T* p, std::size_t n) noexcept
    {
        report(p, n, 0);
        //free(p);
	numa_free(p, n);
    }
private:
    void report(T* p, std::size_t n, bool alloc = true) const
    {
        std::cout << "numa_Malloc " << (alloc ? "Alloc: " : "Dealloc: ") << "for " << n << " elements " << sizeof(T) * n
                  << " bytes at " << std::hex << std::showbase
                  << reinterpret_cast<void*>(p) << std::dec << '\n';
    }
};

template<class T, class U>
bool operator==(const numa_Mallocator <T>&, const numa_Mallocator <U>&) { return true; }

template<class T, class U>
bool operator!=(const numa_Mallocator <T>&, const numa_Mallocator <U>&) { return false; }

