#pragma once

template<class Derived>
class Singleton
{
public:
    static Derived* instance()
    {
      return s_instance;
    }

protected:
    Singleton()
    {
        s_instance = this;
    }

    ~Singleton()
    {
        if (s_instance == this)
            s_instance = nullptr;
    }

private:
    static Derived* s_instance;
};