/* 
 *   Minimal reimplementation of std::atomic_ref to use with GCC <10
 *
 */

template<class T>
class AtomicRef {
public:
    AtomicRef() : addr(nullptr) {}
    AtomicRef(T *ad) : addr(ad) {}

    inline T load() {
        alignas( sizeof(T) ) T ret = __atomic_load_n(addr, __ATOMIC_ACQUIRE);
        return ret;
    }

    inline void store(T val) {
        alignas ( sizeof(T) ) T v = val;
        __atomic_store_n(addr, v, __ATOMIC_RELEASE);
    }
private:
    T *addr;
};