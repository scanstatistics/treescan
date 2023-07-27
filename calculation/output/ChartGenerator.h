//******************************************************************************
#ifndef __ChartGenerator_H
#define __ChartGenerator_H
//******************************************************************************
#include "FileName.h"
#include "UtilityFunctions.h"
#include "SimulationVariables.h"
#include <fstream>

/* abstract base class for chart generation. */
class AbstractChartGenerator {
    protected:
        static const char * HTML_FILE_EXT;
        static const char * TEMPLATE_BODY;

    public:
        AbstractChartGenerator() {}
        virtual void generateChart() const = 0;
};

class ChartSeries {
    protected:
        std::string _id;
        unsigned int _zindex;
        std::string _type;
        std::string _name;
        std::string _color;
        std::string _symbol;
        std::string _opacity;
        std::string _dashstyle;
		std::string _stack;
        unsigned int _y_axis;
        bool _visible;
        std::stringstream _data_stream;

    public:
        ChartSeries(const std::string& id, unsigned int zindex, const std::string& type, 
                    const std::string& name, const std::string& color, const std::string& symbol, unsigned int y_axis, 
			        bool visible=true, const std::string& opacity="1", const std::string& dashstyle="Solid", const std::string& stack="")
                    : _id(id), _zindex(zindex), _type(type), _name(name), _color(color), _symbol(symbol), _y_axis(y_axis), _visible(visible),
			          _opacity(opacity), _dashstyle(dashstyle), _stack(stack) {}

        std::stringstream & datastream() {return _data_stream;}
        std::string & toString(std::string& r) const {
            std::stringstream s;

            s << "{ id: '" << _id.c_str() << "', zIndex: " << _zindex << ", type: '" << _type.c_str() << "', name: '" << _name.c_str()
              << "', color: '#" << _color.c_str() << "', dashStyle: '" << _dashstyle.c_str() << "', yAxis: " << _y_axis << ", visible:" << (_visible ? "true" : "false") << ", opacity: " << _opacity
              <<", marker: { enabled: true, symbol: '" << _symbol.c_str() << "', radius: 0 }, data: [" << _data_stream.str().c_str() << "]";
			if (!_stack.empty())
				s << ", stack: '" << _stack << "'";
			s << " }";
            r = s.str();
            return r;
        }
};

class ScanRunner;
class CutStructure;

/* generator for temporal chart */
class TemporalChartGenerator : public AbstractChartGenerator {
    public:
        static const char * FILE_SUFFIX_EXT;
        static const int    MAX_INTERVALS;
        static const int    MAX_X_AXIS_TICKS;

    protected:
        static const char * BASE_TEMPLATE;
        static const char * TEMPLATE_CHARTHEADER;
		static const char * TEMPLATE_CHARTSERIES;
		static const char * TEMPLATE_CHARTSERIES_PT;
		static const char * TEMPLATE_CHARTSECTION;
        const ScanRunner &  _scanner;
        const SimulationVariables & _simVars;

        class intervalGroups {
            public:
                typedef std::vector<std::pair<int,int> > intervals_t;
            private:
                intervals_t _interval_grps;
            public:
                void addGroup(int start, int end) {_interval_grps.push_back(std::make_pair(start,end));}
                const intervals_t& getGroups() const {return _interval_grps;}
        };
        intervalGroups getIntervalGroups(const CutStructure& cluster) const;
        std::pair<int, int> getSeriesStreams(const CutStructure& cluster,
                                              const intervalGroups& groups,
                                              size_t dataSetIdx,
                                              std::stringstream& categories,
                                              ChartSeries*  clusterSeries,
                                              ChartSeries& observedSeries,
                                              ChartSeries& expectedSeries,
                                              ChartSeries * cluster_observedSeries,
                                              ChartSeries * cluster_expectedSeries,
                                              ChartSeries * odeSeries,
                                              ChartSeries * cluster_odeSeries,
                                              ChartSeries * percCasesSeries,
                                              ChartSeries * cluster_percCasesSeries
            ) const;

    public:
        TemporalChartGenerator(const ScanRunner& dataHub, const SimulationVariables& simVars);

        virtual void generateChart() const;
        static FileName& getFilename(FileName& filename);
};
//******************************************************************************
#endif
