
TREESCAN     := /prj/treescan/build.area/treescan

CC          := g++
M_CFLAGS    :=
APPLICATION := TreeScan
MAC_APPLICATION := TreeScan_mac
LINUX_LIBRARY := libtreescan.linux.so
SOLARIS_LIBRARY := libtreescan.solaris.so
MAC_LIBRARY := libtreescan.jnilib
DEBUG       := -ggdb
COMPILATION := -m32
OPTIMIZATION := -O3

CALCULATION    := $(TREESCAN)/calculation
RUNNER         := $(TREESCAN)/calculation/runner
OUTPUT         := $(TREESCAN)/calculation/output
PRINT          := $(TREESCAN)/calculation/print
UTILITY        := $(TREESCAN)/calculation/utility
RANDOMIZER     := $(TREESCAN)/calculation/randomization
LOGLIKELIHOOD  := $(TREESCAN)/calculation/loglikelihood
#JNI         :=
BOOSTDIR    := $(TREESCAN)/../boost/boost_1_46_0
INCLUDEDIRS := -I$(CALCULATION) -I$(UTILITY) -I$(OUTPUT) -I$(PRINT) -I$(UTILITY) -I$(RANDOMIZER) -I$(LOGLIKELIHOOD) -I$(RUNNER) -I$(BOOSTDIR) -I$(JNI)
DEFINES     := -DBOOST_ALL_NO_LIB

CFLAGS      := -c $(M_CFLAGS) $(COMPILATION) -Wno-deprecated $(OPTIMIZATION) $(DEBUG) $(INCLUDEDIRS) $(DEFINES) $(THREAD_DEFINE)
LFLAGS      := $(COMPILATION) -Wl,-Bstatic -lm -Wl,-Bdynamic -lrt -lpthread

# Linux link flags
L_DLFLAGS   := -shared $(COMPILATION) -Wl,-soname,$(LINUX_LIBRARY).x.x -o $(LINUX_LIBRARY).x.x.0

# Solaris link flags
S_DLFLAGS   := -shared $(COMPILATION) -z text -o $(SOLARIS_LIBRARY).x.x.0

# Mac OS X flags
M_LFLAGS      := $(COMPILATION) -Wl,-dynamic -lstdc++ -lm
M_DLFLAGS     := -shared $(COMPILATION) -install_name $(MAC_LIBRARY)

SRC         := $(RUNNER)/ScanRunner.cpp \
               $(RUNNER)/DataSource.cpp \
               $(RUNNER)/DataTimeRanges.cpp \
               $(OUTPUT)/DataFileWriter.cpp \
               $(OUTPUT)/ResultsFileWriter.cpp \
               $(PRINT)/BasePrint.cpp \
               $(PRINT)/PrintScreen.cpp \
               $(PRINT)/PrintQueue.cpp \
               $(RANDOMIZER)/Randomization.cpp \
               $(RANDOMIZER)/DenominatorDataRandomizer.cpp \
               $(RANDOMIZER)/BernoulliRandomizer.cpp \
               $(RANDOMIZER)/PoissonRandomizer.cpp \
               $(RANDOMIZER)/TemporalRandomizer.cpp \
               $(LOGLIKELIHOOD)/Loglikelihood.cpp \
               $(UTILITY)/RandomDistribution.cpp \
               $(UTILITY)/RandomNumberGenerator.cpp \
               $(UTILITY)/PrjException.cpp \
               $(UTILITY)/UtilityFunctions.cpp \
               $(UTILITY)/MonteCarloSimFunctor.cpp \
               $(UTILITY)/MCSimJobSource.cpp \
               $(UTILITY)/FileName.cpp \
               $(UTILITY)/TimeStamp.cpp \
               $(UTILITY)/FieldDef.cpp \
               $(UTILITY)/contractor.cpp \
               $(UTILITY)/AsynchronouslyAccessible.cpp \
               $(UTILITY)/AsciiPrintFormat.cpp \
               $(CALCULATION)/Parameters.cpp \
               $(CALCULATION)/ParametersPrint.cpp \
               $(CALCULATION)/ParameterFileAccess.cpp \
               $(CALCULATION)/ParametersValidate.cpp \
               $(CALCULATION)/Toolkit.cpp \
               $(BOOSTDIR)/libs/thread/src/pthread/once.cpp \
               $(BOOSTDIR)/libs/thread/src/pthread/thread.cpp \
               $(BOOSTDIR)/libs/program_options/src/cmdline.cpp \
               $(BOOSTDIR)/libs/program_options/src/config_file.cpp \
               $(BOOSTDIR)/libs/program_options/src/convert.cpp \
               $(BOOSTDIR)/libs/program_options/src/options_description.cpp \
               $(BOOSTDIR)/libs/program_options/src/positional_options.cpp \
               $(BOOSTDIR)/libs/program_options/src/split.cpp \
               $(BOOSTDIR)/libs/program_options/src/utf8_codecvt_facet.cpp \
               $(BOOSTDIR)/libs/program_options/src/value_semantic.cpp \
               $(BOOSTDIR)/libs/program_options/src/variables_map.cpp \
               $(BOOSTDIR)/libs/system/src/error_code.cpp               

APP_SRC     := $(TREESCAN)/batch_application/TreeScan.cpp
LIB_SRC     := $(TREESCAN)/library/SharedLibrary.cpp \
               $(TREESCAN)/library/JNIPrintWindow.cpp \
               $(TREESCAN)/library/ParametersUtility.cpp \
               $(TREESCAN)/library/JNIException.cpp \
               $(TREESCAN)/library/PrintCallback.cpp

OBJS        := $(SRC:.cpp=.o)
APP_OBJS    := $(APP_SRC:.cpp=.o)
LIB_OBJS    := $(LIB_SRC:.cpp=.o)

.PHONY: clean all thin

all : $(APPLICATION) $(LINUX_LIBRARY)

thin : all
	strip $(APPLICATION)
	strip $(LINUX_LIBRARY).x.x.0

$(APPLICATION) : $(OBJS) $(APP_OBJS)
	$(CC) $(OBJS) $(APP_OBJS) $(LFLAGS) -o $@

$(MAC_APPLICATION) : $(OBJS) $(APP_OBJS)
	$(CC) $(OBJS) $(APP_OBJS) $(M_LFLAGS) -o $@

$(LINUX_LIBRARY) : $(OBJS) $(LIB_OBJS)
	$(CC) $(L_DLFLAGS) $(OBJS) $(LIB_OBJS) -lm -lrt -lpthread

$(SOLARIS_LIBRARY) : $(OBJS) $(LIB_OBJS)
	$(CC) $(S_DLFLAGS) $(OBJS) $(LIB_OBJS) -lm -lrt -lpthread

$(MAC_LIBRARY) : $(OBJS) $(LIB_OBJS)
	$(CC) $(M_DLFLAGS) $(OBJS) $(LIB_OBJS) -lstdc++ -lm -o $@
%.o : %.cpp
	$(CC) $(CFLAGS) $< -o $@

clean :
	rm -f core $(OBJS)
	rm -f $(APPLICATION)
	rm -f core $(APP_OBJS)
	rm -f $(LINUX_LIBRARY).x.x.0
	rm -f $(SOLARIS_LIBRARY).x.x.0
	rm -f core $(LIB_OBJS)
	rm -f $(MAC_APPLICATION)
	rm -f $(MAC_LIBRARY)
