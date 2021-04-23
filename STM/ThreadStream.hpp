#ifndef THREADSTREAM
#define THREADSTREAM

#include <iostream>
#include <sstream>
#include <mutex>

#define TERR ThreadStream(std::cerr)
#define TOUT ThreadStream(std::cout)

/**
 * Thread-safe std::ostream class.
 *
 * Usage:
 *    tout << "Hello world!" << std::endl;
 *    terr << "Hello world!" << std::endl;
 */
class ThreadStream : public std::ostringstream
{
    public:
        ThreadStream(std::ostream& os) : os_(os) {
            imbue(os.getloc());
            precision(os.precision());
            width(os.width());
            setf(std::ios::fixed, std::ios::floatfield);
        }

        ~ThreadStream() {
            std::lock_guard<std::mutex> guard(_mutex_threadstream);
            os_ << this->str();
        }

    private:
        static std::mutex _mutex_threadstream;
        std::ostream& os_;
};

std::mutex ThreadStream::_mutex_threadstream{};

#endif
