#include "obspp-manager.hpp"

obs::objectManager g_objectManager;

namespace obs {

uint32_t objectManager::map(obs::object object)
{
    uint32_t id = m_index.generateNewIndex();
    m_objects[id] = object;

    return id;
}

void objectManager::unmap(uint32_t id)
{
    bool used = m_index.isUsed(id, true);

    if (!used)
        return;

    m_index.markUsed(id, false);
    auto it = m_objects.find(id);
    m_objects.erase(it);
}

obs::object objectManager::getObject(uint32_t id)
{
    bool used = m_index.isUsed(id, true);

    if (!used)
        return obs::object{ obs::object::invalid, nullptr };

    auto it = m_objects.find(id);
    return it->second;
}

bool objectManager::isUsed(uint32_t id, bool used)
{
	return m_index.isUsed(id, used);
}

void object::checkType(obs::object::objectType type)
{
    if (type != this->type) 
        throw std::runtime_error("object::check_type failed");
}


bool object::testType(obs::object::objectType type)
{
    return type == this->type;
}

}