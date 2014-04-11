//*****************************************************************************
#ifndef __WINDOW_LENGTH_H
#define __WINDOW_LENGTH_H
//*****************************************************************************
/** abstract base class for getting minimum/maximum window lengths */
class AbstractWindowLength {
    public:
        AbstractWindowLength() { }
        virtual ~AbstractWindowLength() {}

        virtual void reset() {}
        virtual int minimum() = 0;
        virtual int maximum() = 0;
};

/** fixed length minimum/maximum window lengths */
class WindowLength : public AbstractWindowLength {
    private:
        int _minimum;
        int _maximum;

    public:
        WindowLength(int minimum, int maximum) : _minimum(minimum), _maximum(maximum) {}
        virtual ~WindowLength() {}

        virtual int minimum() {return _minimum;}
        virtual int maximum() {return _maximum;}
};
//*****************************************************************************
#endif
