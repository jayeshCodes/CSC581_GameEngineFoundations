//
// Created by Utsav Lal on 8/31/24.

#ifndef SINGLETON_HPP
#define SINGLETON_HPP

/**
 * 
 * @tparam T This is a generic singleton class. Any class inherited from this also becomes singleton. This class is inspired from chatgpt
 */
template<class T>

class Singleton {
public:
    // Disallow copy constructor and copy assignment to prevent copying
    Singleton(const Singleton &) = delete;

    Singleton &operator=(const Singleton &) = delete;

    // Static to access singleton instance
    static T *getInstance() {
        static T instance;
        return &instance;
    }

protected:
    Singleton() = default;

    virtual ~Singleton() = default;
};

#endif //SINGLETON_HPP
