template<class T>
struct nullable {
private:
    T * _value;
public:
    bool null() const {
        return _value != nullptr;        
    }
    operator T&();
    operator const T&();
    nullable() : _value(nullptr) {}
    nullable(const T&);
    ~nullable();
    void nullify();
};


template<class T>
inline nullable<T>::nullable(const T& copy) {
    _value = new T(copy);
}

template<class T>
inline nullable<T>::~nullable() {
    if(_value != nullptr)
        delete _value;
}

template<class T>
inline nullable<T>::operator T&() {
    return *_value;    
}

template<class T>
inline nullable<T>::operator const T&() {
    return *_value;    
}

template<class T>
inline void nullable<T>::nullify() {
    if(_value != nullptr)
        delete _value;
}
