//*****************************************************************************
#ifndef __PrintQueue_H
#define __PrintQueue_H
//*****************************************************************************
#include "BasePrint.h"
#include <deque>
#include <memory>
#include "TimeStamp.h"

class PrintQueue : public BasePrint
{
public:
   class threshold_policy_i
   {
   public:
      virtual ~threshold_policy_i() {}

      virtual threshold_policy_i * Clone() const = 0;
      virtual long GetRecommendedThresholdValue_OnConstruction() = 0; // called on PrintQueue construction
      virtual long GetRecommendedThresholdValue(long lCurrentThreshold, long lCurrentSize) = 0;
      virtual long GetRecommendedThresholdValue_OnDestruction(long lCurrentThreshold, long lCurrentSize) = 0; // called on PrintQueue destruction
   };

   class default_threshold_policy : public threshold_policy_i
   {
   public:
      virtual ~default_threshold_policy() {}
      virtual default_threshold_policy * Clone() const { return new default_threshold_policy(*this); }
      virtual long GetRecommendedThresholdValue_OnConstruction() { return 0; }
      virtual long GetRecommendedThresholdValue(long lCurrentThreshold, long lCurrentSize) { return lCurrentThreshold; }
      virtual long GetRecommendedThresholdValue_OnDestruction(long lCurrentThreshold, long lCurrentSize) { return 0; }
   };

private:
   BasePrint & gTarget;
   std::auto_ptr<PrintQueue::threshold_policy_i> gpThresholdPolicy;
   std::deque< std::pair<BasePrint::PrintType, std::string> > gOutputLines; // holds lines, along with an indicator that tells whether or not the line is a "warning" line (true==is).
   long glThreshold;

   void PrintWarningQualifiedLine(BasePrint::PrintType, const char * s);
   void PrintWarningQualifiedLineToTarget(BasePrint::PrintType, const char * s);
   void UpdateThreshold();

protected:
   virtual void PrintError(const char * sMessage);
   virtual void PrintNotice(const char * sMessage);
   virtual void PrintStandard(const char * sMessage);
   virtual void PrintWarning(const char * sMessage);

public:
   PrintQueue(BasePrint & Target, bool bSuppressWarnings);
   PrintQueue(BasePrint & Target, threshold_policy_i const & ThresholdPolicy, bool bSuppressWarnings);
   ~PrintQueue();

   inline bool GetIsCanceled() const { const_cast<PrintQueue&>(*this).UpdateThreshold(); return gTarget.GetIsCanceled(); }

   long GetThreshold() const { return glThreshold; }
   void SetThreshold(long lNewThreshold); // ensure: GetThreshold() == lNewThreshold;
   void Release() { SetThreshold(0); } // ensure: GetThreshold() == 0;
   void Hold() { SetThreshold(std::numeric_limits<long>::max()); } // ensure: GetThreshold() == std::numeric_limits<long>::max;
   void SetThresholdPolicy(threshold_policy_i const & NewThresholdPolicy) { gpThresholdPolicy.reset(NewThresholdPolicy.Clone()); UpdateThreshold(); }
};


/** On construction, a Timestamp, release_time, is passed.
    Until systemtime >= release_time, the recommended threshold value is
    the maximum (virtually infinite). Thereafter it is 0 (i.e. the queue is
    released).
*/
class TimedReleaseThresholdPolicy : public PrintQueue::threshold_policy_i
{
private:
   TreeScan::Timestamp gtsReleaseTime; // the time after which the RecommendedThresholdValue goes to 0.

public:
   TimedReleaseThresholdPolicy(TreeScan::Timestamp tsReleaseTime) : gtsReleaseTime(tsReleaseTime) {  }
   virtual ~TimedReleaseThresholdPolicy() {}

   void SetFinalizationTime(TreeScan::Timestamp tsNewTime) { gtsReleaseTime = tsNewTime; }

   virtual TimedReleaseThresholdPolicy * Clone() const { return new TimedReleaseThresholdPolicy(*this); }
   virtual long GetRecommendedThresholdValue_OnConstruction();
   virtual long GetRecommendedThresholdValue(long lCurrentThreshold, long lCurrentSize);
   virtual long GetRecommendedThresholdValue_OnDestruction(long lCurrentThreshold, long lCurrentSize);
};

//*****************************************************************************
#endif
