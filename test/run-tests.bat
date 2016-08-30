

set app=%~dp0Win32\Release\test32.exe
set log=%~dp0test-log.txt
set report=%~dp0test-results.xml
set examples_path=%~dp0..\installers\examples
set testfiles_path=%~dp0test-sets

REM All Unit Tests
%app% --show_progress=yes --catch_system_errors=yes --build_info=yes --log_level=all --log_format=HRF --log_sink=%log% --report_format=XML --report_level=detailed --report_sink=%report% -- --fixture-examples-path=%examples_path% --fixture-testdata-path=%testfiles_path%

REM Julian Date Unit Tests
REM %app% --show_progress=yes --catch_system_errors=yes --log_level=all --log_sink=%log% --report_format=XML --report_level=detailed --report_sink=%report% --run_test=julian* -- --fixture-examples-path=%examples_path% --fixture-testdata-path=%testfiles_path%

REM Loglikelihood Unit Tests
REM %app% --show_progress=yes --catch_system_errors=yes --log_level=all --log_sink=%log% --report_format=XML --report_level=detailed --report_sink=%report% --run_test=*loglikelihood* -- --fixture-examples-path=%examples_path% --fixture-testdata-path=%testfiles_path%

REM sample data Unit Tests
REM %app% --show_progress=yes --catch_system_errors=yes --log_level=all --log_sink=%log% --report_format=XML --report_level=detailed --report_sink=%report% --run_test=*sampledata*,parameter* -- --fixture-examples-path=%examples_path% --fixture-testdata-path=%testfiles_path%
