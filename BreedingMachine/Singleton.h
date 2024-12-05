#pragma once
#include "Rasticore/rasti_main.h"

class SingletonWrapper
{
public:
    static rasticore::RastiCoreRender* object;
    static rasticore::RastiCoreRender* GetInstance()
    {
        if (object == nullptr)
            object = new rasticore::RastiCoreRender(100);
        return object;
    }

private:
    SingletonWrapper() {}
};
rasticore::RastiCoreRender* SingletonWrapper::object = nullptr;