#include "TreeScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "PrintQueue.h"

//for _sleep():
#if defined(_WINDOWS_)
#include <dos.h>
#else
#include <unistd.h>
#endif

//ClassDesc PrintQueue
// A PrintQueue is a BasePrint which operates on another BasePrint object (its
// "target").  The PrintQueue holds up to GetThreshold() printlines in a queue,
// forwarding them to "target".
// A print queue's threshold value can be "automated" by assigning a "threshold
// policy" to it.  The 'threshold_policy_i' contains one major function:
// GetRecommendedThresholdPolicy, which passes a "current threshold" and a
// "current size".  Every member function of PrintQueue calls this function
// and sets the threshold with the result (except for threshold manipulation
// functions).
// The function, SetThreshold, disregards the threshold policy.  If the new
// threshold is lower than the current threshold, lines are forwarded to target'
// until just 'new_threshold' lines remain in the queue.
//
//ClassDesc End PrintQueue


//constructor
//ensure
//  is_released : GetThreshold() == 0
PrintQueue::PrintQueue(BasePrint & Target, bool bSuppressWarnings)
 : BasePrint(bSuppressWarnings)
 , gTarget(Target)
 , gpThresholdPolicy(new default_threshold_policy())
 , glThreshold(0)
{
   SetThreshold(gpThresholdPolicy->GetRecommendedThresholdValue_OnConstruction());
}

//constructor
//ensure
//  threshold_set : GetThreshold() == lThreshold
PrintQueue::PrintQueue(BasePrint & Target, threshold_policy_i const & ThresholdPolicy, bool bSuppressWarnings)
 : BasePrint(bSuppressWarnings),
   gTarget(Target)
 , gpThresholdPolicy(ThresholdPolicy.Clone())
 , glThreshold(0)
{
   SetThreshold(gpThresholdPolicy->GetRecommendedThresholdValue_OnConstruction());
}

//destructor
//Dump all queued lines to gTarget via Release().
PrintQueue::~PrintQueue()
{
   try
   {
      SetThreshold(gpThresholdPolicy->GetRecommendedThresholdValue_OnDestruction(glThreshold, static_cast<long>(gOutputLines.size())));
   }
   catch (...)
   {
      //log that an exception was thrown.
      //do not rethrow exception from this destructor.
   }
}

//Set the threshold to 'lNewThreshold'.  If 'lNewThreshold' < GetThreshold() then
//forward lines to 'target' until 'lNewThreshold' lines remain queued.
void PrintQueue::SetThreshold(long lNewThreshold)
{
   if (lNewThreshold < 0)//make threshold "virtually infinite"
     lNewThreshold = std::numeric_limits<long>::max();

   while (gOutputLines.size() > static_cast<unsigned>(lNewThreshold))
   {
      PrintWarningQualifiedLineToTarget(gOutputLines.front().first, gOutputLines.front().second.c_str());
      gOutputLines.pop_front();
   }
   glThreshold = lNewThreshold;
}

//Print a (non-warning) line.
void PrintQueue::PrintStandard(const char * sMessage)
{
   UpdateThreshold();
   PrintWarningQualifiedLine(BasePrint::P_STDOUT, sMessage);
}

//Print a "error" line.
void PrintQueue::PrintError(const char * sMessage) {
   UpdateThreshold();
   PrintWarningQualifiedLine(BasePrint::P_ERROR, sMessage);
}

//Print a "notice" line.
void PrintQueue::PrintNotice(const char * sMessage) {
   UpdateThreshold();
   PrintWarningQualifiedLine(BasePrint::P_NOTICE, sMessage);
}

//Print a "warning" line.
void PrintQueue::PrintWarning(const char * sMessage) {
   UpdateThreshold();
   PrintWarningQualifiedLine(BasePrint::P_WARNING, sMessage);
}

//Print a line. 'ePrintType' indicates type of message being printed.
void PrintQueue::PrintWarningQualifiedLine(BasePrint::PrintType ePrintType, const char * s)
{
   std::pair<BasePrint::PrintType, std::string> arg_line(ePrintType, std::string(s));
   if ((gOutputLines.size() < static_cast<unsigned>(GetThreshold())) || (GetThreshold() < 0))
   {
      gOutputLines.push_back(arg_line);
   }
   else
   {
      if (gOutputLines.size() > 0)
      {
         std::pair<BasePrint::PrintType, std::string> dequeued_line(gOutputLines.front());
         gOutputLines.push_back(arg_line);
         try{ PrintWarningQualifiedLineToTarget(dequeued_line.first, dequeued_line.second.c_str());
         }catch(...){ gOutputLines.pop_back(); throw; }
         gOutputLines.pop_front();
      }
      else
      {
         PrintWarningQualifiedLineToTarget(arg_line.first, arg_line.second.c_str());
      }
   }
}

//Send a line to gTarget. 'ePrintType' indicates type of message being printed.
void PrintQueue::PrintWarningQualifiedLineToTarget(BasePrint::PrintType ePrintType, const char * s)
{
   gTarget.Printf(s, ePrintType);
}

//Set the threshold to the value recommended by the threshold policy.
void PrintQueue::UpdateThreshold()
{
   SetThreshold(gpThresholdPolicy->GetRecommendedThresholdValue(glThreshold, static_cast<long>(gOutputLines.size())));
}



//ClassDesc Begin
//This policy is:
// --until FinalizationTime is past, ThresholdValue is "infinite".
// --thereafter, ThresholdValue is 0.
//ClassDesc End

long TimedReleaseThresholdPolicy::GetRecommendedThresholdValue_OnConstruction()
{
   if (TreeScan::Timestamp::Current(false) > gtsReleaseTime)
   {
      return 0;
   }
   else
   {
      return -1;
   }
}

long TimedReleaseThresholdPolicy::GetRecommendedThresholdValue(long lCurrentThreshold, long lCurrentSize)
{
   if (TreeScan::Timestamp::Current(false) > gtsReleaseTime)
   {
      return 0;
   }
   else
   {
      return std::numeric_limits<long>::max();
   }
}

long TimedReleaseThresholdPolicy::GetRecommendedThresholdValue_OnDestruction(long, long)
{
   TreeScan::Timestamp CurrentTime;
   CurrentTime.Now(false);
   if (CurrentTime < gtsReleaseTime)
   {
      unsigned long ulTimeDifference(gtsReleaseTime.GetTimeInMilliseconds() - CurrentTime.GetTimeInMilliseconds());
      //how many seconds to sleep?  round to the nearest:
      unsigned u((ulTimeDifference + 500) / 1000);
      //sleep...
      #if defined(_WINDOWS_)
      Sleep(u);
      #else
      sleep(u);
      #endif
   }
   return 0;//whether or not we wait to return, we want the new threshold to be 0.
}



