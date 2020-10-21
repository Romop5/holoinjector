#ifndef VE_CONTEXT_TRACKER_HPP
#define VE_CONTEXT_TRACKER_HPP
#include <unordered_map>
#include <cassert>

/**
 * @brief Generic object tracker functionality
 *
 * Provides the most generic object tracking functionality, that is used almost
 * by all OpenGL objects (e.g. shaders, buffers)
 */
template<typename T>
class ContextTracker
{
    public:
    using MapType = std::unordered_map<size_t, T>;
    ContextTracker() = default;
    bool has(size_t id) const;
    T get(size_t id);
    T const getConst(size_t id) const;
    void add(size_t id, T object);
    void remove(size_t id);

    // TODO: replace with something that can iterate
    MapType& getMap();
    protected:
    MapType m_storage;
};

/**
 * @brief Extends ContextTracker with binding functionality
 */
template<typename T>
class BindableContextTracker: public ContextTracker<T>
{
    public:
    BindableContextTracker() = default;
    void bind(size_t id);
    void unbind();

    bool hasBounded() const;
    T getBound();
    T const getBoundConst() const;

    size_t getBoundId() const;

    protected:
    size_t m_currentlyBoundObjectId = 0;
};

///////////////////////////////////////////////////////////////////////////////
// Implementation
///////////////////////////////////////////////////////////////////////////////

template<typename T>
bool ContextTracker<T>::has(size_t id) const
{
    return m_storage.count(id) > 0;
}

template<typename T>
T ContextTracker<T>::get(size_t id)
{
    assert(this->has(id) == true);
    return m_storage.at(id);
}

template<typename T>
T const ContextTracker<T>::getConst(size_t id) const
{
    assert(this->has(id) == true);
    return m_storage.at(id);
}

template<typename T>
void ContextTracker<T>::add(size_t id,T object) 
{
    m_storage[id] = object;
}

template<typename T>
void ContextTracker<T>::remove(size_t id)
{
    m_storage[id].remove(id);
}

template<typename T>
typename ContextTracker<T>::MapType& ContextTracker<T>::getMap()
{
    return m_storage;
}
///////////////////////////////////////////////////////////////////////////////
// BindableContextTracker 
///////////////////////////////////////////////////////////////////////////////
template<typename T>
void BindableContextTracker<T>::bind(size_t id)
{
    m_currentlyBoundObjectId = id;
}

template<typename T>
void BindableContextTracker<T>::unbind()
{
    m_currentlyBoundObjectId = 0;
}

template<typename T>
bool BindableContextTracker<T>::hasBounded() const
{
    return m_currentlyBoundObjectId != 0;
}

template<typename T>
T BindableContextTracker<T>::getBound()
{
    assert(this->has(m_currentlyBoundObjectId));
    return this->get(m_currentlyBoundObjectId);
}

template<typename T>
T const BindableContextTracker<T>::getBoundConst() const
{
    assert(this->has(m_currentlyBoundObjectId));
    return this->getConst(m_currentlyBoundObjectId);
}

template<typename T>
size_t BindableContextTracker<T>::getBoundId() const
{
    return m_currentlyBoundObjectId;
}

#endif
