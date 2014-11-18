//******************************************************************************
#ifndef __AppToolkit_H
#define __AppToolkit_H
//******************************************************************************
#include <list>
//#include "RunTimeComponents.h"
               
/** Application global toolkit. */
class AppToolkit {
  private:
    static AppToolkit         * gpToolKit;

    // system file
    static const char         * gsDebugFileName;
    // default defines
    static const char         * gsDefaultRunHistoryFileName;
    static const char         * gsDefaultTreeScanWebSite;
    static const char         * gsDefaultSubstantiveSupportEmail;
    static const char         * gsDefaultTechnicalSupportEmail;

    std::string                 gsApplicationFullPath;
    bool                        gbRunUpdateOnTerminate;
    std::string                 gsUpdateArchiveFilename;
    std::string                 gsVersion;
    std::string                 gsSystemIniFileName;
    //RunTimeComponentManager     gRunTimeComponentManager;
    FILE                      * gpDebugLog;

    void                        Setup(const char * sApplicationFullPath);

  public:
    AppToolkit(const char * sApplicationFullPath);
    virtual ~AppToolkit();

   void                         closeDebugFile();
   const char                 * GetAcknowledgment(std::string & Acknowledgment) const;
   const char                 * GetApplicationFullPath() const;
   //RunTimeComponentManager    & GetRunTimeComponentManager() { return gRunTimeComponentManager;}
   bool                         GetRunUpdateOnTerminate() const {return gbRunUpdateOnTerminate;}
   const char                 * GetSubstantiveSupportEmail() const;
   const char                 * GetTechnicalSupportEmail() const;
   const char                 * GetUpdateArchiveFilename() const {return gsUpdateArchiveFilename.c_str();}
   const char                 * GetVersion() const {return gsVersion.c_str();}
   const char                 * GetWebSite() const;
   bool                         is64Bit() const;
   FILE                       * openDebugFile(); 
   void                         SetRunUpdateOnTerminate(bool b) {gbRunUpdateOnTerminate = b;}
   void                         SetUpdateArchiveFilename(const char * sArchiveFile) {gsUpdateArchiveFilename = sArchiveFile;}

   static AppToolkit     &      getToolkit() {return *gpToolKit;}
   static void                  ToolKitCreate(const char * sApplicationFullPath);
   static void                  ToolKitDestroy();
};

#ifdef RPRTCMPT_RUNTIMES
  #define macroRunTimeManagerInit()     AppToolkit::getToolkit().GetRunTimeComponentManager().Initialize()
  #define macroRunTimeManagerPrint(p)   AppToolkit::getToolkit().GetRunTimeComponentManager().Print(p)
  #define macroRunTimeStartSerial(p)    AppToolkit::getToolkit().GetRunTimeComponentManager().StartSerialComponent(p)
  #define macroRunTimeStopSerial()      AppToolkit::getToolkit().GetRunTimeComponentManager().StopSerialComponent()
  #define macroRunTimeStartFocused(p)   AppToolkit::getToolkit().GetRunTimeComponentManager().StartFocused(p)
  #define macroRunTimeStopFocused(p)    AppToolkit::getToolkit().GetRunTimeComponentManager().StopFocused(p)
#else
  #define macroRunTimeManagerInit()     ((void)0)
  #define macroRunTimeManagerPrint(p)    ((void)0)
  #define macroRunTimeStartSerial(p)    ((void)0)
  #define macroRunTimeStopSerial()     ((void)0)
  #define macroRunTimeStartFocused(p)   ((void)0)
  #define macroRunTimeStopFocused(p)    ((void)0)
#endif

//******************************************************************************
#endif
