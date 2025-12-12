#ifndef __ISYSTEM_HPP__
#define __ISYSTEM_HPP__

class ISystem
{
    public:
        virtual void Update() = 0;
        virtual ~ISystem() {}
};

#endif
