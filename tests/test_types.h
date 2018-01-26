#pragma once

#include <memory>

class Interface {
    protected:
        Interface(int data) : m_data(data) {};

    public:
        int m_data;

        virtual ~Interface() = default;

        virtual void DoThings() = 0;
};

class SimpleImplementation : public Interface {
    public:
        SimpleImplementation(int data) : Interface(data) {};
        ~SimpleImplementation() = default;

        void DoThings() {};
};

class NonCopyable
{
    public:
        int m_a;

        NonCopyable(int a) : m_a(a) {}

        NonCopyable(const NonCopyable&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
        NonCopyable(NonCopyable&&) = default;
        NonCopyable& operator=(NonCopyable&&) = default;
};

class SharedImplementationDecorator : public Interface {
    private:
        std::shared_ptr<Interface> m_decorated;

    public:
        SharedImplementationDecorator(int data, std::shared_ptr<Interface> decorated)
            : Interface(data), m_decorated(decorated) {};
        ~SharedImplementationDecorator() = default;

        SharedImplementationDecorator(const SharedImplementationDecorator &) = default;
        SharedImplementationDecorator & operator=(const SharedImplementationDecorator &) = default;

        SharedImplementationDecorator(SharedImplementationDecorator &&) = default;
        SharedImplementationDecorator & operator=(SharedImplementationDecorator &&) = default;

        void DoThings() {};
};

class UniqueImplementationDecorator : public Interface {
    private:
        std::unique_ptr<Interface> m_decorated;

    public:
        UniqueImplementationDecorator(int data, std::unique_ptr<Interface> decorated)
            :  Interface(data), m_decorated(std::move(decorated)) {};
        ~UniqueImplementationDecorator() = default;

        UniqueImplementationDecorator(const UniqueImplementationDecorator &) = delete;
        UniqueImplementationDecorator & operator=(const UniqueImplementationDecorator &) = delete;

        UniqueImplementationDecorator(UniqueImplementationDecorator && other) : Interface(m_data) {
            m_decorated = std::move(other.m_decorated);
            m_data = other.m_data;
        }

        UniqueImplementationDecorator & operator=(UniqueImplementationDecorator && other) {
            if (this != &other) {
                m_decorated = std::move(other.m_decorated);
                m_data = other.m_data;
            }
            return *this;
        }

        void DoThings() {};
};

class ComplexImplementation : public Interface {
    private:
        std::shared_ptr<Interface> m_obj1;
        std::shared_ptr<Interface> m_obj2;

    public:
        ComplexImplementation(int data, std::shared_ptr<Interface> obj1, std::shared_ptr<Interface> obj2)
            : Interface(data), m_obj1(obj1), m_obj2(obj2) {};
        ~ComplexImplementation() = default;

        ComplexImplementation(const ComplexImplementation &) = default;
        ComplexImplementation & operator=(const ComplexImplementation &) = default;

        ComplexImplementation(ComplexImplementation &&) = default;
        ComplexImplementation & operator=(ComplexImplementation &&) = default;

        void DoThings() {};
};
