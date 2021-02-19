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
    T& get(size_t id);
    const T& getConst(size_t id) const;
    void add(size_t id, T object);
    void remove(size_t id);
    size_t size() const;

    // TODO: replace with something that can iterate
    MapType& getMap();
    const MapType& getConstMap() const;
    protected:
    MapType m_storage;
};

/**
 * @brief Extends ContextTracker with binding functionality
 */
template<typename T, bool IS_ZERO_RESERVED = true>
class BindableContextTracker: public ContextTracker<T>
{
    public:
    BindableContextTracker() = default;
    void bind(size_t id);
    void unbind();

    bool hasBounded() const;
    T& getBound();
    const T& getBoundConst() const;

    size_t getBoundId() const;

    // Overrides
    void remove(size_t id);

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
T& ContextTracker<T>::get(size_t id)
{
    assert(this->has(id) == true);
    return m_storage.at(id);
}

template<typename T>
const T& ContextTracker<T>::getConst(size_t id) const
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
    m_storage.erase(id);
}

template<typename T>
size_t ContextTracker<T>::size() const
{
    return m_storage.size();
}

template<typename T>
typename ContextTracker<T>::MapType& ContextTracker<T>::getMap()
{
    return m_storage;
}

template<typename T>
const typename ContextTracker<T>::MapType& ContextTracker<T>::getConstMap() const
{
    return m_storage;
}

///////////////////////////////////////////////////////////////////////////////
// BindableContextTracker
///////////////////////////////////////////////////////////////////////////////
template<typename T, bool IS_ZERO_RESERVED>
void BindableContextTracker<T,IS_ZERO_RESERVED>::bind(size_t id)
{
    m_currentlyBoundObjectId = id;
}

template<typename T, bool IS_ZERO_RESERVED>
void BindableContextTracker<T,IS_ZERO_RESERVED>::unbind()
{
    m_currentlyBoundObjectId = 0;
}

template<typename T, bool IS_ZERO_RESERVED>
bool BindableContextTracker<T,IS_ZERO_RESERVED>::hasBounded() const
{
    // if !IS_ZERO_RESERVED, then 0 object is active by default 
    // (e.g. texture unit)
    return (IS_ZERO_RESERVED?m_currentlyBoundObjectId != 0:true);
}

template<typename T, bool IS_ZERO_RESERVED>
T& BindableContextTracker<T,IS_ZERO_RESERVED>::getBound()
{
    assert(this->has(m_currentlyBoundObjectId));
    return this->get(m_currentlyBoundObjectId);
}

template<typename T, bool IS_ZERO_RESERVED>
const T& BindableContextTracker<T,IS_ZERO_RESERVED>::getBoundConst() const
{
    assert(this->has(m_currentlyBoundObjectId));
    return this->getConst(m_currentlyBoundObjectId);
}

template<typename T, bool IS_ZERO_RESERVED>
size_t BindableContextTracker<T,IS_ZERO_RESERVED>::getBoundId() const
{
    return m_currentlyBoundObjectId;
}

template<typename T, bool IS_ZERO_RESERVED>
void BindableContextTracker<T,IS_ZERO_RESERVED>::remove(size_t id)
{
    if(m_currentlyBoundObjectId == id)
    {
        m_currentlyBoundObjectId = 0;
    }

    ContextTracker<T>::remove(id);
}


#endif
