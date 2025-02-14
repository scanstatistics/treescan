//******************************************************************************
#include "TreeScan.h"
#pragma hdrstop
//******************************************************************************
#include "ChartGenerator.h"
#include "ScanRunner.h"
#include "SimulationVariables.h"
#include "Toolkit.h"

/** ------------------- AbstractChartGenerator --------------------------------*/
const char * AbstractChartGenerator::HTML_FILE_EXT = ".html";

const char * AbstractChartGenerator::TEMPLATE_BODY = "\n \
        <body style=\"margin:0;background-color: #fff;\"> \n \
        <div id=\"load_error\" style=\"color:#101010; text-align: center;font-size: 1.2em; padding: 20px;background-color: #ece1e1; border: 1px solid #e49595; display:none;\"></div> \n \
	    <div style=\"position: relative;\"> \n \
	        <div class=\"search-and-account\" title=\"Choose which graphs to display.\"> \n \
                <a href=\"#\" data-toggle=\"modal\" data-target=\"#graphs-modal-modal\" class=\"offscreen\"> \n \
                    <span class=\"screen-reader-text\">Search</span> \n \
                        <svg width=\"40\" height=\"40\" fill=\"red\"> \n \
                            <image xlink:href=\"--resource-path--images/graph-choose.svg\" src=\"--resource-path--images/graph-choose.png\" width=\"40\" height=\"40\"/> \n \
                        </svg> \n \
                </a> \n \
            </div> \n \
        </div> \n \
        <div class=\"modal fade\" id=\"graphs-modal-modal\" data-backdrop=\"static\" data-keyboard=\"false\"> \n \
            <div class=\"modal-dialog modal-sm\"> \n \
                <div class=\"modal-content\"> \n \
                    <div class=\"modal-body\"> \n \
                        <div style=\"display: table;\"> \n \
                            <div class=\"btn-group\" style=\"display: table-cell;\"> \n \
                                <label style=\"font-size: 16px;\">Choose which graphs to display:</label> \n \
                                <select id=\"graph-checkbox-list\" multiple=\"multiple\"> \n \
                                --graph-list-options-- \n \
                                </select> \n \
                            </div> \n \
                           <div class=\"btn-group-vertical\" style=\"display: table-cell; padding-left: 10px;\"> \n \
                               <button id=\"id_apply_graphs\" class=\"btn btn-primary export-submit btn-sm\">Apply</button> \n \
                               <button id=\"id_cancel_modal\" type=\"button\" class=\"btn btn-warning btn-sm\" data-dismiss=\"modal\">Cancel</button> \n \
                           </div> \n \
                       </div> \n \
                       <div class=\"progress\"> \n \
                           <div class=\"progress-bar\" role=\"progressbar\" aria-valuenow=\"0\" aria-valuemin=\"0\" aria-valuemax=\"100\" style=\"width:0%\"> \n \
                               <span class=\"sr-only\">0 % Complete</span> \n \
                           </div> \n \
                       </div> \n \
                    </div> \n \
                </div> \n \
            </div> \n \
        </div> \n \
        --main-content-- \n";

/** ------------------- TemporalChartGenerator --------------------------------*/

const char * TemporalChartGenerator::FILE_SUFFIX_EXT = ".temporal";
const int TemporalChartGenerator::MAX_INTERVALS = 4000;
const int TemporalChartGenerator::MAX_X_AXIS_TICKS = 500;

const char * TemporalChartGenerator::BASE_TEMPLATE = " \
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"> \n \
<html lang=\"en\"> \n \
    <head> \n \
        <title>--title--</title> \n \
        <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"> \n \
        <link rel='stylesheet' href='--resource-path--javascript/highcharts/highcharts-9.1.2/code/css/highcharts.css' type='text/css'>\n \
        <link rel='stylesheet' href='--resource-path--javascript/jquery/bootstrap/3.3.7/bootstrap.min.css'> \n \
        <link rel='stylesheet' href='--resource-path--javascript/jquery/bootstrap/bootstrap-multiselect/bootstrap-multiselect.css'> \n \
        <link rel='stylesheet' href='--resource-path--html-results/treescan-temporal.1.0.css'> \n \
        <script type='text/javascript' src='--resource-path--javascript/jquery/jquery-3.1.1.min.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/highcharts/highcharts-9.1.2/code/highcharts.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/highcharts/highcharts-9.1.2/code/modules/exporting.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/highcharts/highcharts-9.1.2/code/modules/offline-exporting.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/jquery/bootstrap/3.3.7/bootstrap.min.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/jquery/bootstrap/bootstrap-multiselect/bootstrap-multiselect.js'></script> \n \
        <script type='text/javascript'> \n \
            var charts = {}; \n \
            var tech_support = '--tech-support-email--'; \n \
            $(document).ready(function () { \n \
                --charts--   \n\n \
            }); \n \
        </script> \n \
        <script type='text/javascript' src='--resource-path--html-results/treescan-temporal.1.0.js'></script> \n \
    </head> \n \
    <body> \n \
        <!--[if lt IE 9]> \n \
        <div id=\"ie\" style=\"z-index:255;border-top:5px solid #fff;border-bottom:5px solid #fff;background-color:#c00; color:#fff;\"><div class=\"iewrap\" style=\"border-top:5px solid #e57373;border-bottom:5px solid #e57373;\"><div class=\"iehead\" style=\"margin: 14px 14px;font-size: 20px;\">Notice to Internet Explorer users!</div><div class=\"iebody\" style=\"font-size: 14px;line-height: 14px;margin: 14px 28px;\">It appears that you are using Internet Explorer, <strong>this page may not display correctly with versions 8 or earlier of this browser</strong>.<br /><br /> \n \
            <i>This page is known to display correctly with the following browsers: Safari 4+, Firefox 3+, Opera 10+ and Google Chrome 5+.</i> \n \
        </div></div></div> \n \
        <![endif]--> \
        --body--<div style=\"font-style:italic;margin-left:20px;font-size:14px;\">Generated with --treescan-version--</div> \n \
    </body> \n \
</html> \n";

const char * TemporalChartGenerator::TEMPLATE_CHARTHEADER = "\n \
                var --container-id-- = { \n \
                    chart: { height: 400, renderTo: '--container-id--', zoomType:'xy', panning: true, panKey: 'shift', resetZoomButton: {relativeTo: 'chart', position: {x: -80, y: 10}, theme: {fill: 'white',stroke: 'silver',r: 0,states: {hover: {fill: '#41739D', style: { color: 'white' } } } } }, marginBottom: --margin-bottom--, borderColor: '#888888', plotBackgroundColor: '#e6e7e3', borderRadius: 0, borderWidth: 1, marginRight: --margin-right-- }, \n \
                    title: { text: '--chart-title--', align: 'center' }, \n \
                    exporting: {fallbackToExportServer: false, filename: 'cluster_graph', chartOptions: { plotOptions: { series: { showInLegend: false } } }, buttons: get_extended_export_buttons('Chart Options', showChartOptions)}, \n \
                    plotOptions: { column: { grouping: true, stacking: 'normal' }}, \n \
                    responsive: { rules: [{ condition: {  maxWidth: null }, chartOptions: { chart: { height: 400 }, subtitle: { text: null }, navigator: { enabled: false } } }] }, \n \
                    tooltip: { crosshairs: true, shared: true, formatter: function(){var is_cluster = false;var has_observed = false;$.each(this.points, function(i, point) {if (point.series.options.id == 'cluster') {is_cluster = true;}if (point.series.options.id == 'obs') {has_observed = true;}});var s = '<b>'+ this.x +'</b>'; if (is_cluster) {s+= '<br/><b>Cluster Point</b>';}$.each(this.points,function(i, point){if (point.series.options.id == 'cluster'){if (!has_observed) {s += '<br/>Observed: '+ point.y;}} else {s += '<br/>'+ point.series.name +': '+ point.y;}});return s;}, }, \n \
                    legend: { enabled: false, backgroundColor: '#F5F5F5', verticalAlign: 'bottom', y: 20 }, \n \
                    xAxis: [{ categories: [--categories--], tickmarkPlacement: 'on', labels: { step: --step--, rotation: -45, align: 'right' }, tickInterval: --tickinterval-- }], \n \
                    yAxis: [{ title: { enabled: true, text: 'Number of Cases', style: { fontWeight: 'normal' } }, min: 0, showEmpty: false }--additional-yaxis--], \n \
                    navigation: { buttonOptions: { align: 'right' } }, \n \
                    series: [--series--]\n \
                }; \n \
                charts['--container-id--'] = --container-id--;";

const char * TemporalChartGenerator::TEMPLATE_CHARTSERIES = "\
                      <div class=\"options-row\"> \n \
                          <label>Series Inside Cluster</label> \n \
                              <div> \n \
                                  <label class=\"container\">Observed \n \
                                      <input class =\"series-toggle\" series-id=\"cluster_obs\" name=\"--container-id--_observed_series_toggle\" id=\"id_--container-id--_observed_series_toggle\" value=\"observed\" type=checkbox checked /> \n \
                                      <span class=\"checkmark\"></span> \n \
                                  </label> \n \
                                  <label class=\"container\">Expected \n \
                                      <input class=\"series-toggle\" series-id=\"cluster_exp\" name=\"--container-id--_expected_series_toggle\" id=\"id_--container-id--_observed_series_toggle\" value=\"expected\" type=checkbox /> \n \
                                      <span class=\"checkmark\"></span> \n \
                                  </label> \n \
                                  <label class=\"container\">Observed / Expected \n \
                                      <input class=\"series-toggle\" series-id=\"cluster_obs_exp\" name=\"--container-id--_ode_series_toggle\" id=\"id_--container-id--_ode_series_toggle\" value=\"ode\" type=checkbox /> \n \
                                      <span class=\"checkmark\"></span> \n \
                                  </label> \n \
                                  <label class=\"container\">Percent Cases \n \
                                      <input class=\"series-toggle\" series-id=\"cluster_perc_cases\" name=\"--container-id--_case_perc_series_toggle\" id=\"id_--container-id--_case_perc_series_toggle\" value=\"case_perc\" type=checkbox checked /> \n \
                                      <span class=\"checkmark\"></span> \n \
                                  </label> \n \
                              </div> \n \
                      </div> \n \
                      <div class=\"options-row\"> \n \
                          <label>Series Outside Cluster</label> \n \
                          <div> \n \
                              <label class=\"container\">Observed \n \
                                  <input class=\"series-toggle\" series-id=\"obs\" name=\"--container-id--_observed_series_toggle\" id=\"id_--container-id--_observed_series_toggle\" value=\"observed\" type=checkbox checked /> \n \
                                  <span class=\"checkmark\"></span> \n \
                              </label> \n \
                              <label class=\"container\">Expected \n \
                                  <input class=\"series-toggle\" series-id=\"exp\" name=\"--container-id--_expected_series_toggle\" id=\"id_--container-id--_observed_series_toggle\" value=\"expected\" type=checkbox /> \n \
                                  <span class=\"checkmark\"></span> \n \
                              </label> \n \
                          </div> \n \
                      </div> \n";

const char * TemporalChartGenerator::TEMPLATE_CHARTSERIES_PT = "\
                      <div class=\"options-row\"> \n \
                          <label>Series Cluster</label> \n \
                          <div> \n \
                              <label class=\"container\">Observed \n \
                                  <input class=\"series-toggle\" series-id=\"obs\" name=\"--container-id--_observed_series_toggle\" id=\"id_--container-id--_observed_series_toggle\" value=\"observed\" type=checkbox checked /> \n \
                                  <span class=\"checkmark\"></span> \n \
                              </label> \n \
                              <label class=\"container\">Expected \n \
                                  <input class=\"series-toggle\" series-id=\"exp\" name=\"--container-id--_expected_series_toggle\" id=\"id_--container-id--_observed_series_toggle\" value=\"expected\" type=checkbox checked /> \n \
                                  <span class=\"checkmark\"></span> \n \
                              </label> \n \
                          </div> \n \
                      </div> \n";

const char * TemporalChartGenerator::TEMPLATE_CHARTSECTION = "\
	        <div class=\"item\" style=\"display:none;\"><div style=\"margin:20px;\" class=\"chart-section\"> \n \
            <div id=\"--container-id--\" class=\"highchart-container\" style=\"margin-top:0px;\"></div> \n \
            <div class=\"options\"> \n \
                <div class=\"show-chart-options\"><a href=\"#\">Show Chart Options</a></div> \n \
                <div class=\"chart-options\"> \n \
                    <div class=\"options-table\"> \n \
                    <div class=\"row\"> \n \
                    <div class=\"col-md-6\"> \n \
                      <h4>Chart Options</h4> \n \
                      <div class=\"options-row\"> \n \
                          <label for=\"title_obs\">Title</label> \n \
                          <div><input type=\"text\" style=\"width:95%;\" class=\"title-setter\" id=\"title_obs\"> \n \
                              <p class=\"help-block\">Title can be changed by editing this text.</p> \n \
                          </div> \n \
                      </div> \n--series-selection-- \
                      <div class=\"options-row\"> \n \
                          <label>Observed Chart Type</label> \n \
                          <div> \n \
                            <label> \n \
                              <input type=\"radio\" name=\"--container-id--_obs_series_type\" series-type=\"column\" series-id=\"--chart-switch-ids--\" checked=checked/>Histogram \n \
                            </label> \n \
                            <label> \n \
                              <input type=\"radio\" name=\"--container-id--_obs_series_type\" series-type=\"line\" series-id=\"--chart-switch-ids--\"/>Line \n \
                            </label> \n \
                            <p class=\"help-block\">Switch the series type between line and histogram.</p> \n \
                          </div> \n \
                      </div> \n \
                      <div class=\"options-row\"> \n \
                          <label>Cluster Band</label> \n \
                          <div> \n \
                            <label> \n \
                              <input type=\"checkbox\" class=\"show-cluster-band\" name=\"--container-id--_cluster_band\" start-idx=\"--cluster-start-idx--\" end-idx=\"--cluster-end-idx--\"/>Show Cluster Band \n \
                            </label> \n \
                            <p class=\"help-block\">Band stretching across the plot area marking cluster interval.</p> \n \
                          </div> \n \
                      </div> \n \
                      <div class=\"options-row\">To zoom a portion of the chart, select and drag mouse within the chart. Hold down shift key to pan zoomed chart.</div> \n \
                    </div> \n \
                    --cluster-details-- \n \
                    </div> \n \
                    </div> \n \
                    <div class=\"hide-chart-options\"><a href=\"#\">Close Chart Options</a></div> \n \
                </div> \n \
            </div> \n \
         </div></div> \n";

const char* TemporalChartGenerator::TEMPLATE_CLUSTERDETAILS = "\n \
            <div class='col-md-6'> \n \
              <h4 style='text-align:left;margin-bottom:5px;'>2 x 2 Table</h4> \n \
              <div class='row' style='margin-left:10px;'> \n \
                <table class='table table-condensed cluster-details' cellpadding='3' cellspacing='0'> \n \
                  <thead> \n \
                    <tr> \n \
                      <th style='border-bottom: none;'></th> \n \
                      <th colspan='2' style='padding-bottom:0px;text-align:left;'>Cluster Time Period</th> \n \
                    </tr> \n \
                   <tr> \n \
                     <th style='text-align:left;border-top:none;'>Cluster Node</th> \n \
                     <th class='cluster-details-sub-header'>Inside</th> \n \
                     <th class='cluster-details-sub-header'>Outside</th> \n \
                    </tr> \n \
                  </thead> \n \
                  <tbody> \n \
                    <tr><th class='cluster-details-sub-header'>Inside</th><td>--inside-inside--</td><td>--outside-inside--</td></tr> \n \
                    <tr><th class='cluster-details-sub-header'>Outside</th><td>--inside-outside--</td><td>--outside-outside--</td></tr> \n \
                    <tr class='cluster-details-percentages'><th></th><td>--inside-percent--%</td><td>--outside-percent--%</td></tr> \n \
                  </tbody> \n \
                </table> \n \
              </div> \n \
            </div> \n";

/** Constructor */
TemporalChartGenerator::TemporalChartGenerator(const ScanRunner& scanner, const SimulationVariables& simVars):_scanner(scanner), _simVars(simVars) {
    if (_scanner.getParameters().getScanType() == Parameters::TREETIME) {
        _ptcases = std::vector<int>(static_cast<int>(_scanner.getParameters().getDataTimeRangeSet().getTotalDaysAcrossRangeSets()) + 1, 0);
        // number of cases by time interval for all nodes
        for (size_t n = 0; n < _scanner.getNodes().size(); ++n) {
            const NodeStructure& thisNode(*(_scanner.getNodes()[n]));
            for (size_t i = 0; i < thisNode.getIntC_C().size() - 1; ++i)
                _ptcases[i] += thisNode.getIntC_C()[i];
        }
    }
}

/** Creates HighCharts graph for purely temporal cluster. */
void TemporalChartGenerator::generateChart() const {
    std::string nodeName, nodeShortName, buffer, buffer2;
    FileName fileName;
    const Parameters parameters = _scanner.getParameters();

    try {
        fileName.setFullPath(parameters.getOutputFileName().c_str());
        getFilename(fileName);

        std::ofstream HTMLout;
        // open output file
        HTMLout.open(fileName.getFullPath(buffer).c_str());
        if (!HTMLout) throw resolvable_error("Error: Could not open file '%s'.\n", fileName.getFullPath(buffer).c_str());
        if (_scanner.getCuts().size() == 0 || !_scanner.reportableCut(*_scanner.getCuts()[0])) {
            HTMLout.close();
            return;
        }

        std::stringstream html, charts_javascript, cluster_sections, chart_select_options;

        // read template into stringstream
        html << BASE_TEMPLATE << std::endl;
        // replace page title
        templateReplace(html, "--title--", "Cluster Temporal Graph");
        // replace specialized body
        templateReplace(html, "--body--", TEMPLATE_BODY);
        // site resource link path
        templateReplace(html, "--resource-path--", AppToolkit::getToolkit().GetWebSite());
        // tech support link path
        templateReplace(html, "--tech-support-email--", AppToolkit::getToolkit().GetTechnicalSupportEmail());

        // set margin bottom according to time precision
        int margin_bottom=130;
        switch (parameters.getDatePrecisionType()) {
            case DataTimeRange::YEAR : margin_bottom = 90; break;
            case DataTimeRange::MONTH : margin_bottom = 100; break;
            case DataTimeRange::DAY:
            case DataTimeRange::GENERIC:
            default: margin_bottom=100;
        }

        // Determine clusters will have a graph generated based on settings.
        std::vector<const CutStructure*> graphClusters;
        switch (parameters.getTemporalGraphReportType()) {
            case Parameters::MLC_ONLY :
                graphClusters.push_back(_scanner.getCuts()[0]); break;
            case Parameters::X_MCL_ONLY :
                for (int i=0; i < parameters.getTemporalGraphMostLikelyCount() && static_cast<unsigned int>(i) < _scanner.getCuts().size(); ++i) {
                    if (_scanner.reportableCut(*_scanner.getCuts()[i]))
                        graphClusters.push_back(_scanner.getCuts()[i]);
                }
                break;
            case Parameters::SIGNIFICANT_ONLY :
                for (ScanRunner::CutStructureContainer_t::const_iterator itr = _scanner.getCuts().begin(); itr != _scanner.getCuts().end(); ++itr) {
                    if (_scanner.reportableCut(*(*itr)) && (*itr)->getPValue(_scanner) <= parameters.getTemporalGraphSignificantCutoff())
                        graphClusters.push_back(*itr);
                }
                break;
        }

        for (size_t clusterIdx=0; clusterIdx < graphClusters.size(); ++clusterIdx) {
            const CutStructure& cluster = *graphClusters[clusterIdx];
            // calculate the graphs interval groups for this cluster
            intervalGroups groups = getIntervalGroups(cluster);
            std::stringstream chart_js, chart_series, chart_section, categories, cluster_details;
            // set the chart header for this cluster
            chart_js << TEMPLATE_CHARTHEADER;
            bool is_pt(parameters.getScanType() == Parameters::TIMEONLY); // not if cluster is purely temporal
            // define seach series that we'll graph - next three are always printed.
            std::auto_ptr<ChartSeries> observedSeries(new ChartSeries("obs", 1, "column", (is_pt ? "Observed" : "Observed (Outside Cluster)"), is_pt ? "471D1B" : "7D96B0", "square", 0, true, "1", "Solid", "observed"));
            std::auto_ptr<ChartSeries> expectedSeries(new ChartSeries("exp", is_pt ? 3 : 2, "line", (is_pt ? "Expected" : "Expected (Outside Cluster)"), is_pt ? "394521" : "89A54E", "triangle", 0));
            std::auto_ptr<ChartSeries> clusterSeries; // (new ChartSeries("cluster", is_pt ? 2 : 5, "column", "Cluster", "AA4643", "circle", 0));
            // the remaining series are conditionally present in the chart
            std::auto_ptr<ChartSeries> observedClusterSeries, expectedClusterSeries;
            std::auto_ptr<ChartSeries> odeSeries, cluster_odeSeries, percentCasesSeries, cluster_percentCasesSeries;
            std::stringstream additional_yaxis;
            // space-time clusters also graph series which allow comparison between inside and outside the cluster
            if (!is_pt) {
                observedClusterSeries.reset(new ChartSeries("cluster_obs", 3, "column", "Observed (Inside Cluster)", "471D1B", "square", 0, true, "1", "Solid", "observed"));
                expectedClusterSeries.reset(new ChartSeries("cluster_exp", 4, "line", "Expected (Inside Cluster)", "394521", "triangle", 0));
                percentCasesSeries.reset(new ChartSeries("perc_cases", 6, "line", "Percent Cases (Outside Cluster)", "46460F", "circle", 2, false));
                cluster_percentCasesSeries.reset(new ChartSeries("cluster_perc_cases", 6, "line", "Percent Cases (Inside Cluster)", "C24641", "circle", 2, false, "1", "Dot"));
            }
            // the Poisson and Exponential models also graphs observed / expected
            if (parameters.getModelType() == Parameters::BERNOULLI_TIME) {
                // the Bernoulli model also graphs cases / (cases + controls)
                // graphing cases ratio, with y-axis along right side
                additional_yaxis << ", { title: { enabled: true, text: 'Cases Ratio', style: { fontWeight: 'normal' } }, max: 1, min: 0, opposite: false, showEmpty: false }";
                odeSeries.reset(new ChartSeries("case_ratio", 2, "line", (is_pt ? "Cases Ratio" : "Cases Ratio (Outside Cluster)"), "00FF00", "triangle", 1));
                if (!is_pt)
                    // space-time clusters also graph series which allow comparison between inside and outside the cluster
                    cluster_odeSeries.reset(new ChartSeries("cluster_obs_exp", 2, "line", "Cases Ratio (Inside Cluster)", "FF8000", "triangle", 1));
            } else if (parameters.getModelType() == Parameters::UNIFORM || (parameters.getModelType() == Parameters::MODEL_NOT_APPLICABLE && parameters.getConditionalType() == Parameters::NODEANDTIME)) {
                // graphing observed / expected, with y-axis along right side
                additional_yaxis << ", { title: { enabled: true, text: 'Observed / Expected', style: { fontWeight: 'normal' } }, min: 0, opposite: false, showEmpty: false }";
                odeSeries.reset(new ChartSeries("obs_exp", 2, "line", (is_pt ? "Observed / Expected" : "Observed / Expected (Outside Cluster)"), "00FF00", "triangle", 1, false));
                if (!is_pt)
                    // space-time clusters also graph series which allow comparison between inside and outside the cluster
                    cluster_odeSeries.reset(new ChartSeries("cluster_obs_exp", 2, "line", "Observed / Expected (Inside Cluster)", "FF8000", "triangle", 1, false));
            }
            additional_yaxis << ", { title: { enabled: true, text: 'Percent Cases', style: { fontWeight: 'normal' } }, max: 100, min: 0, startOnTick: false, endOnTick: false, gridLineWidth: 0.1, opposite: true, showEmpty: false, labels: {format: '{text}%'} }";
            templateReplace(chart_js, "--additional-yaxis--", additional_yaxis.str());
            // set default chart title
            if (is_pt)
                nodeName = "Detected Cluster";
            else {
                auto node = _scanner.getNodes().at(cluster.getID());
                nodeName = node->getIdentifier();
                if (node->getName().size()) {
                    nodeName += " ("; nodeName += node->getName();  nodeName += ")";
                }
            }
            nodeShortName = nodeName;
            htmlencode(nodeName);
            if (nodeShortName.size() > 50) {
                nodeShortName.resize(50);
                nodeShortName.resize(52, '.');
            }
            htmlencode(nodeShortName);
            templateReplace(chart_js, "--chart-title--", nodeName);
            templateReplace(chart_js, "--margin-bottom--", printString(buffer, "%d", margin_bottom));
            templateReplace(chart_js, "--margin-right--", printString(buffer, "%d", (odeSeries.get() || cluster_odeSeries.get() ? 80 : 20)));
            // increase x-axis 'step' if there are many intervals, so that labels are not crowded
            //  -- empirically, 50 ticks seems like a good upper limit
            templateReplace(chart_js, "--step--", printString(buffer, "%u", static_cast<int>(std::ceil(static_cast<double>(groups.getGroups().size())/50.0))));
            // get series datastreams plus cluster indexes start and end ticks
            std::pair<int,int> cluster_grp_idx = getSeriesStreams(
                cluster, groups, 0, categories, clusterSeries.get(), *observedSeries, *expectedSeries, observedClusterSeries.get(), 
                expectedClusterSeries.get(), odeSeries.get(), cluster_odeSeries.get(), percentCasesSeries.get(), cluster_percentCasesSeries.get()
            );
            // define the identifying attribute of this chart
            templateReplace(chart_js, "--container-id--", printString(buffer, "chart_%d_%u", clusterIdx + 1, 0 + 1));
            // add select option for this chart
            chart_select_options << "<option value=\"" << buffer.c_str() << "\" " << (clusterIdx == 0 ? "selected=selected" : "") << ">" << nodeShortName.c_str() << "</option>" << std::endl;
            templateReplace(chart_js, "--tickinterval--", 
                printString(buffer, "%u", static_cast<unsigned int>(std::ceil(static_cast<double>(groups.getGroups().size()) / static_cast<double>(MAX_X_AXIS_TICKS))))
            );
            templateReplace(chart_js, "--categories--", categories.str());
            // replace the series
            if (clusterSeries.get()) chart_series << clusterSeries->toString(buffer).c_str();
            chart_series << (clusterSeries.get() ? "," : "") << observedSeries->toString(buffer).c_str();
            chart_series << "," << expectedSeries->toString(buffer).c_str();
            if (observedClusterSeries.get())
                chart_series << "," << observedClusterSeries->toString(buffer).c_str();
            if (expectedClusterSeries.get())
                chart_series << "," << expectedClusterSeries->toString(buffer).c_str();
            //if (odeSeries.get()) 
            //    chart_series << "," << odeSeries->toString(buffer).c_str();
            if (cluster_odeSeries.get())
                chart_series << "," << cluster_odeSeries->toString(buffer).c_str();
            //chart_series << "," << percentCasesSeries->toString(buffer).c_str();
            if (cluster_percentCasesSeries.get())
                chart_series << "," << cluster_percentCasesSeries->toString(buffer).c_str();
            templateReplace(chart_js, "--series--", chart_series.str());
            // add this charts javascript to collection
            charts_javascript << chart_js.str() << std::endl;
            // create chart html section
            chart_section << TEMPLATE_CHARTSECTION;
            templateReplace(chart_section, "--series-selection--", is_pt ? TEMPLATE_CHARTSERIES_PT : TEMPLATE_CHARTSERIES);
            templateReplace(chart_section, "--container-id--", printString(buffer, "chart_%d_%u", clusterIdx + 1, 0 + 1));
            templateReplace(chart_section, "--cluster-start-idx--", printString(buffer, "%d", cluster_grp_idx.first));
            templateReplace(chart_section, "--cluster-end-idx--", printString(buffer, "%d", cluster_grp_idx.second - 1));
            templateReplace(chart_section, "--chart-switch-ids--", is_pt ? "obs" : "obs,cluster_obs");
            cluster_details << TEMPLATE_CLUSTERDETAILS;
            if (parameters.getScanType() == Parameters::TREETIME) {
                TemporalChartGenerator::CutCaseTotals_t caseTotals = getCutCaseTotals(cluster);
                templateReplace(cluster_details, "--inside-inside--", printString(buffer, "%d", caseTotals.get<0>()));
                templateReplace(cluster_details, "--outside-inside--", printString(buffer, "%d", caseTotals.get<1>()));
                templateReplace(cluster_details, "--inside-outside--", printString(buffer, "%d", caseTotals.get<2>()));
                templateReplace(cluster_details, "--outside-outside--", printString(buffer, "%d", caseTotals.get<3>()));
                templateReplace(cluster_details, "--inside-percent--",
                    printString(buffer, "%.1f", static_cast<double>(caseTotals.get<0>()) / static_cast<double>(caseTotals.get<0>() + caseTotals.get<2>()) * 100.0)
                );
                templateReplace(cluster_details, "--outside-percent--",
                    printString(buffer, "%.1f", static_cast<double>(caseTotals.get<1>()) / static_cast<double>(caseTotals.get<1>() + caseTotals.get<3>()) * 100.0)
                );
                templateReplace(chart_section, "--cluster-details--", cluster_details.str());
            } else {
                templateReplace(chart_section, "--cluster-details--", "");
            }
            // add section to collection of sections
            cluster_sections << chart_section.str();
        }

        templateReplace(html, "--charts--", charts_javascript.str());
		templateReplace(html, "--graph-list-options--", chart_select_options.str());
		if (graphClusters.size()) {
            templateReplace(html, "--main-content--", cluster_sections.str());
        } else {
            printString(buffer2, "<h3 style=\"text-align:center;\">No significant clusters to graph. All clusters had a p-value greater than %lf.</h3>",
                0.05 // _dataHub.GetParameters().getTemporalGraphSignificantCutoff()
            );
            templateReplace(html, "--main-content--", buffer2.c_str());
        }
		printString(buffer,
			"TreeScan v%s.%s%s%s%s%s",
			VERSION_MAJOR,
			VERSION_MINOR,
			(!strcmp(VERSION_RELEASE, "0") ? "" : "."),
			(!strcmp(VERSION_RELEASE, "0") ? "" : VERSION_RELEASE),
			(strlen(VERSION_PHASE) ? " " : ""),
			VERSION_PHASE
		);
		templateReplace(html, "--treescan-version--", buffer.c_str());
        HTMLout << html.str() << std::endl;
        HTMLout.close();
    } catch (prg_exception& x) {
        x.addTrace("generate()","TemporalChartGenerator");
        throw;
    }
}

/** Reports number of cases in respect to cluster window and geographical area:
      Inside Cluster Window, Inside Cluster Node
      Outside Cluster Window, Inside Cluster Node
      Inside Cluster Window, Outside Cluster Node
      Outside Cluster Window, Outside Cluster Node */
TemporalChartGenerator::CutCaseTotals_t TemporalChartGenerator::getCutCaseTotals(const CutStructure& cluster) const {
    int intervals = static_cast<int>(_scanner.getParameters().getDataTimeRangeSet().getTotalDaysAcrossRangeSets()) + 1;
    CutCaseTotals_t caseTotals(0, 0, 0, 0);
    for (int i = 0; i < intervals; ++i) {
        // calculate the expected and observed for this interval
        const NodeStructure& thisNode(*(_scanner.getNodes()[cluster.getID()]));
        int intervalClusterObserved = (i == intervals - 1 ? thisNode.getBrC_C()[i] : thisNode.getBrC_C()[i] - thisNode.getBrC_C()[i + 1]);
        int intervalTotalCases = (i == intervals - 1 ? _ptcases[i] : _ptcases[i] - _ptcases[i + 1]);
        if (cluster.getStartIdx() <= i && i <= cluster.getEndIdx()) {
            caseTotals.get<0>() += intervalClusterObserved;
            caseTotals.get<2>() += intervalTotalCases - intervalClusterObserved;
        } else {
            caseTotals.get<1>() += intervalClusterObserved;
            caseTotals.get<3>() += (intervalTotalCases - intervalClusterObserved);
        }
    }
    return caseTotals;
}

/** Calculates the best fit graph groupings for this cluster. */
TemporalChartGenerator::intervalGroups TemporalChartGenerator::getIntervalGroups(const CutStructure& cluster) const {
    intervalGroups groups;
    int intervals = static_cast<int>(_scanner.getParameters().getDataTimeRangeSet().getTotalDaysAcrossRangeSets());

    if (intervals <= MAX_INTERVALS) {
        // number of groups equals the number of intervals
        for (int i=0; i < intervals; ++i)
            groups.addGroup(i, i+1);
    } else {
        int startIdx = cluster.getStartIdx(), endIdx = cluster.getEndIdx();
        int cluster_length = endIdx - startIdx;
        if (cluster_length <= MAX_INTERVALS) {
            int extra_intervals = ((MAX_INTERVALS - cluster_length)/2) + 5;
            // we can show entire cluster intervals plus a few before and after
            for (int i=std::max(0, startIdx - extra_intervals); i < std::min(intervals, endIdx + extra_intervals); ++i)
                groups.addGroup(i, i+1);
        } else {
            // we can show entire cluster intervals but compressed -- plus a few before and after
            int compressed_interval_length = static_cast<int>(ceil(static_cast<double>(endIdx - startIdx)/static_cast<double>(MAX_INTERVALS)));
            // Note: This rough calculation of a compressed interval means that the clusters last interval might not fall cleanly onto an interval boundary.
            int extra_intervals = compressed_interval_length * 5;
            for (int i=std::max(0, startIdx - extra_intervals); i < std::min(intervals, endIdx + extra_intervals); i=i+compressed_interval_length)
                groups.addGroup(i, i + compressed_interval_length);
        }
    }
    return groups;
}

/** Calculates the series values in a purely temporal context. */
std::pair<int, int> TemporalChartGenerator::getSeriesStreams(const CutStructure& cluster,const intervalGroups& groups, size_t dataSetIdx,
                                                             std::stringstream& categories, ChartSeries * clusterSeries,
                                                             ChartSeries& observedSeries, ChartSeries& expectedSeries,
                                                             ChartSeries * cluster_observedSeries, ChartSeries * cluster_expectedSeries,
                                                             ChartSeries * odeSeries, ChartSeries * cluster_odeSeries,
                                                             ChartSeries * percCasesSeries, ChartSeries * cluster_percCasesSeries) const {

    std::string buffer;
    const Parameters parameters = _scanner.getParameters();
    std::pair<int, int> groupClusterIdx(std::numeric_limits<int>::max(), std::numeric_limits<int>::min());
    bool isUniformTime = parameters.getModelType() == Parameters::UNIFORM && !(parameters.isPerformingDayOfWeekAdjustment() || _scanner.isCensoredData());
    double T = static_cast<double>(parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets());
    int intervals = static_cast<int>(parameters.getDataTimeRangeSet().getTotalDaysAcrossRangeSets()) + 1;
    std::vector<int> pcases(intervals, 0); // number of cases by time interval for all nodes
    std::vector<double> pmeasure(intervals, 0); // expected cases by time interval for all nodes

    if (isUniformTime) { // Expected is constant for uniform time and not adjustments.
        std::fill(pmeasure.begin(), pmeasure.end() - 1, static_cast<double>(_scanner.getTotalC()) / T);
        TreeScan::cumulative_backward(pmeasure);
    }
    for (size_t n = 0; n < _scanner.getNodes().size(); ++n) {
        const NodeStructure& thisNode(*(_scanner.getNodes()[n]));
        for (size_t i = 0; i < thisNode.getIntC_C().size() - 1; ++i) {
            pcases[i] += thisNode.getIntC_C()[i];
            if (!isUniformTime) pmeasure[i] += thisNode.getIntN_C()[i];
        }
    }

    // define categories and replace in template
    DataTimeRange::DatePrecisionType precision = parameters.getDatePrecisionType();
    // iterate through groups, creating totals for each interval grouping
    for (intervalGroups::intervals_t::const_iterator itrGrp=groups.getGroups().begin(); itrGrp != groups.getGroups().end(); ++itrGrp) {
        // define date categories
        if (precision == DataTimeRange::GENERIC)
            categories << (itrGrp == groups.getGroups().begin() ? "'" : ",'") << itrGrp->first - _scanner.getZeroTranslationAdditive() << "'";
        else {
            std::pair<std::string, std::string> rangeDates = parameters.getDataTimeRangeSet().getDataTimeRangeSets().front().rangeToGregorianStrings(
                itrGrp->first - _scanner.getZeroTranslationAdditive(),
                itrGrp->second - _scanner.getZeroTranslationAdditive(),
                precision
            );
            categories << (itrGrp == groups.getGroups().begin() ? "'" : ",'") << rangeDates.first.c_str() << "'";
        }
        // calcuate the expected and observed for this interval
        double expected=0, cluster_expected=0;
        int observed=0, cluster_observed=0;
        for (int i=itrGrp->first; i < itrGrp->second; ++i) {
            expected += (i == intervals - 1 ? pmeasure[i] : pmeasure[i] - pmeasure[i+1]);
            observed += (i == intervals - 1 ? pcases[i] : pcases[i] - pcases[i+1]);
        }
        // if not purely temporal cluster, we're expressing this as outside verse inside cluster
        if (cluster_observedSeries || cluster_expectedSeries) {
            // calculate cluster observed and expected series across entire period, not just cluster window
            const NodeStructure& thisNode(*(_scanner.getNodes()[cluster.getID()]));
            for (int i = itrGrp->first; i < itrGrp->second; ++i) {
                if (isUniformTime) {
                    cluster_observed += (i == intervals - 1 ? thisNode.getBrC_C()[i] : thisNode.getBrC_C()[i] - thisNode.getBrC_C()[i + 1]);
                    cluster_expected += static_cast<double>(thisNode.getBrC()) * 1.0 / T; // expected is constant for uniform time
                } else {
                    cluster_observed += (i == intervals - 1 ? thisNode.getBrC_C()[i] : thisNode.getBrC_C()[i] - thisNode.getBrC_C()[i + 1]);
                    cluster_expected += (i == intervals - 1 ? thisNode.getBrN_C()[i] : thisNode.getBrN_C()[i] - thisNode.getBrN_C()[i + 1]);
                }
            }

            // removed observed and expected from overall temporal values
            observed -= cluster_observed;
            expected -= cluster_expected;
            if (macro_equal(0.0, expected, DBL_CMP_TOLERANCE)) expected = 0;
            if (cluster_odeSeries) {
                /** For the Bernoulli model, this represents the ratio of cases / (cases + controls) inside the cluster.
                For the Poisson/Exponential models, this represents the ratio of observed / expected inside the cluster. */
                getValueAsString(cluster_expected ? static_cast<double>(cluster_observed)/cluster_expected : 0, buffer, 2);
                cluster_odeSeries->datastream() <<  (itrGrp==groups.getGroups().begin() ? "" : ",") <<  buffer.c_str();
            }
        }
        if (odeSeries) {
            /** For the Bernoulli model, this represents the ratio of cases / (cases + controls) inside the cluster.
            For the Poisson/Exponential models, this represents the ratio of observed / expected inside the cluster. */
            getValueAsString(expected ? static_cast<double>(observed)/expected : 0, buffer, 2);
            odeSeries->datastream() << (itrGrp==groups.getGroups().begin() ? "" : ",") <<  buffer.c_str();
        }

        if (parameters.getModelType() == Parameters::BERNOULLI_TIME) {
            expected *= (static_cast<double>(_scanner.getTotalC()) / _scanner.getTotalN());
            cluster_expected *= (static_cast<double>(_scanner.getTotalC()) / _scanner.getTotalN());
        }

        // put totals to other streams
        expectedSeries.datastream() << (itrGrp==groups.getGroups().begin() ? "" : ",") << getValueAsString(expected, buffer, 2).c_str();
        observedSeries.datastream() <<  (itrGrp==groups.getGroups().begin() ? "" : ",") << observed;

        double totalCases = static_cast<double>(observed + cluster_observed);
		if (percCasesSeries)
			percCasesSeries->datastream() << (itrGrp == groups.getGroups().begin() ? "" : ",") << getValueAsString((totalCases ? static_cast<double>(observed) / totalCases : totalCases) * 100, buffer, 1).c_str();
		if (cluster_percCasesSeries)
			cluster_percCasesSeries->datastream() << (itrGrp == groups.getGroups().begin() ? "" : ",") << getValueAsString((totalCases ? static_cast<double>(cluster_observed) / totalCases : totalCases) * 100, buffer, 1).c_str();

        bool is_pt(_scanner.getParameters().getScanType() == Parameters::TIMEONLY); // not if cluster is purely temporal

        if (cluster_expectedSeries)
            cluster_expectedSeries->datastream() << (itrGrp==groups.getGroups().begin() ? "" : ",") << getValueAsString(cluster_expected, buffer, 2).c_str();
        if (cluster_observedSeries)
            cluster_observedSeries->datastream() << (itrGrp==groups.getGroups().begin() ? "" : ",") << cluster_observed;
        if (cluster.getStartIdx() <= itrGrp->first && itrGrp->second <= cluster.getEndIdx() + 1) {
            groupClusterIdx.first = std::min(groupClusterIdx.first, itrGrp->first);
            groupClusterIdx.second = std::max(groupClusterIdx.second, itrGrp->second);
            if (clusterSeries) clusterSeries->datastream() <<  (itrGrp==groups.getGroups().begin() ? "" : ",") << (is_pt ? observed : cluster_observed);
        } else {
            if (clusterSeries) clusterSeries->datastream() << (itrGrp==groups.getGroups().begin() ? "" : ",") << "null";
        }
    }

    // if cluster start index is not set, set to start of first group
    if (groupClusterIdx.first == std::numeric_limits<int>::max()) {
        groupClusterIdx.first = groups.getGroups().front().first;
    }
    // if cluster end index is not set, set to end of last group
    if (groupClusterIdx.second == std::numeric_limits<int>::min()) {
        groupClusterIdx.second = groups.getGroups().back().second;
    }
    return groupClusterIdx;
}

/** Alters passed filename to include suffix and extension. */
FileName& TemporalChartGenerator::getFilename(FileName& filename) {
    std::string buffer;
    printString(buffer, "%s%s", filename.getFileName().c_str(), FILE_SUFFIX_EXT);
    filename.setFileName(buffer.c_str());
    filename.setExtension(HTML_FILE_EXT);
    return filename;
}

