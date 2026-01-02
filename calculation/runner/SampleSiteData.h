//*****************************************************************************
#ifndef __SAMPLE_SITE_DATA_H_
#define __SAMPLE_SITE_DATA_H_
//*****************************************************************************

/* Sample site data for the signed rank data model. */
class SampleSiteData {
private:
    double _baseline; // baseline value
    double _current; // current value

public:
    SampleSiteData() : _baseline(0.0), _current(0.0) {}
    SampleSiteData(double baseline, double current) :
        _baseline(baseline), _current(current) {
    }
    bool operator==(const SampleSiteData& other) const {
        return _baseline == other._baseline && _current == other._current;
    }
    friend std::ostream& operator<<(std::ostream& os, const SampleSiteData& obj) {
        return os << obj.baseline() << "," << obj.current();
    }
    void add(const SampleSiteData& other) {
        _baseline += other.baseline();
        _current += other.current();
    }
    double baseline() const { return _baseline; }
    double current() const { return _current; }
    double difference() const { return _current - _baseline; }
};

typedef std::map<size_t, SampleSiteData> SampleSiteMap_t;
std::pair<double, double> getAverage(const SampleSiteMap_t& ssData);
SampleSiteMap_t& combine(SampleSiteMap_t& accumulation, const SampleSiteMap_t& ssMap, bool clear=false);
//******************************************************************************
#endif