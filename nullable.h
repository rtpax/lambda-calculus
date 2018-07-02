template<class T>
struct nullable {
private:
    T * _value;
public:
    bool null() const {
        return _value != nullptr;        
    }
    operator T&() { return *_value; }
    operator const T&() { return *_value; }
    nullable() : _value(nullptr) {}
    nullable(const T& copy) { _value = new T(copy); }
    ~nullable() { nullify(); }
    T& get() { return *_value; }
    const T& get() const { return *_value; }
    T set(const T& set_to) {
        if(_value == nullptr)
            return *(_value = new T(set_to));
        else
            return *_value = set_to; 
    }
    T set(T& set_to) {
        if(_value == nullptr)
            return *(_value = new T(set_to));
        else
            return *_value = set_to; 
    }
    T operator=(const T& set_to) { return _value = set_to; }
    T operator=(T& set_to) { return _value = set_to; }
    void nullify() {
        if(_value != nullptr)
            delete _value;
        _value = nullptr;
    }
};

