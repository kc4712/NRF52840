#ifndef _PXIALG_DYN_
#define _PXIALG_DYN_

#if defined(WIN32)
#	ifdef PXIALG_EXPORTS
#		define PXIALG_API __declspec(dllexport)
#	else
#		define PXIALG_API __declspec(dllimport)
#	endif
#else
#	define PXIALG_API
#endif	// WIN32


#ifdef __cplusplus
extern "C"
{
#endif
#include <stdbool.h>
#include "HRDParam.h"

#include "pah8series_data_c.h"
PXIALG_API sHRDParam HRD_GetHRDParam();
PXIALG_API int HRD_ALG_VERSION();
PXIALG_API bool HRDProcessTop(UInt8 HRDOpMode,bool DataReadyFlag,float OFMRawData, float OFMRawData2);
PXIALG_API unsigned int HRDOpen_i1000Hz(void * p);
PXIALG_API unsigned int HRDOpen_i350Hz(void *p);
PXIALG_API unsigned int HRDOpen_i200Hz(void *p);
PXIALG_API unsigned int HRDOpen_i100Hz(void *p);
PXIALG_API unsigned int HRDOpen_i20Hz(void *p);
PXIALG_API unsigned int HRDOpenParam(sHRDParam *sParam, void *p);
PXIALG_API void HRDReset();
PXIALG_API void HRDClose();
PXIALG_API void HRD_SDNNProcess();
PXIALG_API void NI_HRDProcess(float OFMRawData);
PXIALG_API int HRD_GetSDNN();
PXIALG_API int HRD_GetSDNNSNR();		//20150326
PXIALG_API int HRD_GetSDNNRawDat();
PXIALG_API float HRD_Get_NI_SDNN();
PXIALG_API int HRD_Get_SDNN_HRAvg();
PXIALG_API float HRD_Get_NI_SDNN_HRAvg();
PXIALG_API int HRD_Get_HRAvg();
PXIALG_API bool HRD_Get_ReadyFlag();
PXIALG_API int HRD_Get_ReadyTime();
PXIALG_API float HRD_Get_SDNNKurtosis();
PXIALG_API float HRD_Get_NI_SDNNKurtosis();
PXIALG_API float HRD_Get_Kurtosis();
PXIALG_API void HRD_Set_FreqOffset(float FreqOffset);
PXIALG_API void HRD_Set_Diff_Time(float DiffTime,int SampleNum);
PXIALG_API float HRD_Get_HR_STD();
PXIALG_API bool HRD_Get_MotionFlag();
PXIALG_API void HRD_Set_SDNN_On();
PXIALG_API void HRD_Set_SDNN_Off();
PXIALG_API void HRD_Set_LHRatio_On();
PXIALG_API void HRD_Set_LHRatio_Off();
PXIALG_API void HRD_Set_SkipCounterTh(int SkipCounterTh);
PXIALG_API void HRD_Set_ReadyCounter_Th(int ReadyCounter_Th0,int ReadyCounter_Th1,int ReadyCounter_Th2,int ReadyCounter_Th3,int ReadyCounter_Th4,int ReadyCounter_Th5);
PXIALG_API void HRD_GetSDNNBoundary(int m_iAge, double *pdSameAgeSDNN, double *pdDetectMax, double *pdDetectMin) ;
PXIALG_API int HRD_GetSDNNLevel(int m_iAge, double dSDNN);
PXIALG_API void HRD_Set_SkipCounterTh(int SkipCounterTh);
PXIALG_API void HRD_Set_PI_Calculate(float PI_dc_offset, float PI_scale);
PXIALG_API float HRD_Get_PI_Index();
PXIALG_API int HRD_GetRRIReportFlag();
PXIALG_API float HRD_GetRRISingleRRI();
PXIALG_API int HRD_GetPeakAmplitude();
PXIALG_API int HRD_GetRRIBufferStatus();
PXIALG_API int HRD_GetRRIDataNumber();
PXIALG_API int HRD_GetRRIDataBuffer(float *pBuffer);
PXIALG_API int HRD_GetSDNNRRIDataNumber();
PXIALG_API int HRD_GetSDNNRRIDataBuffer(float *pBuffer);
//float HRD_GetLHRatio(float *InputBuffer);
PXIALG_API int HRD_GetSignalQualtiy();
PXIALG_API void HRD_Set_PeakProcessReadyCounter_Th(	int PeakProcessReady3Counter_Th0,
											int PeakProcessReady3Counter_Th1,
											int PeakProcessReady3Counter_Th2,
											int PeakProcessReady3Counter_Th3,
											int m_PeakProcessReady3Counter_HeartRate_Th1,
											int m_PeakProcessReady3Counter_HeartRate_Th2,
											int m_PeakProcessReady3Counter_HeartRate_Th3);
PXIALG_API void HRD_Set_HeartRateNotReadyBoundary(int HeartRateNotReadyUpBound,int HeartRateNotReadyLowBound);
PXIALG_API void HRD_Set_Support2PeakCounterOutuptRatio(bool bSupport2PeakCounterOutuptRatio);

PXIALG_API int HRV_GetMemorySize();

PXIALG_API void HRD_HRVProcess(float *InputBuffer);

PXIALG_API float HRD_GetHRV_LHRatio();
PXIALG_API float HRD_GetHRV_LF();
PXIALG_API float HRD_GetHRV_HF();
PXIALG_API float HRD_GetHRV_CVRR();
PXIALG_API float HRD_GetHRV_SDNN();

PXIALG_API int HRD_GetSignalQualtiy();
PXIALG_API int HRV_GetMemorySize();
PXIALG_API void HRV_SetMemory(void *pBuffer);


PXIALG_API void OFM_DisplayReset();
PXIALG_API void OFM_DisplayOpen();
PXIALG_API void OFM_DisplayClose();
PXIALG_API void OFM_DisplayProcess(float OFMRawData,double *pDisplayBuffer,int MaxBufferSize);
PXIALG_API void OFM_DisplayProcess_int(float OFMRawData,int *pDisplayBuffer,int MaxBufferSize);
PXIALG_API float OFM_GetDisplayData();

PXIALG_API void HRD_HRVProcess(float *InputBuffer);

//-----------------------------------------
// added by yuanhsin (v1.001)
//-----------------------------------------
PXIALG_API int HRV_Initialize(float *rri_buf); 
PXIALG_API int HRV_Operate();  
PXIALG_API float HRV_GetSDNN();
PXIALG_API int HRV_GetPowerSpectrumNumber();
PXIALG_API int HRV_GetPowerSpectrumBuffer(float *pBuffer);
PXIALG_API float HRV_GetVLFPeak();
PXIALG_API float HRV_GetVLFPower();
PXIALG_API float HRV_GetLFPeak();
PXIALG_API float HRV_GetLFPower();
PXIALG_API float HRV_GetLFNormalizedPower();
PXIALG_API float HRV_GetHFPeak();
PXIALG_API float HRV_GetHFPower();
PXIALG_API float HRV_GetHFNormalizedPower();
PXIALG_API float HRV_GetLHRatio();
PXIALG_API float HRV_GetRMSSD();
PXIALG_API int HRV_GetSleepQuality(int age);
PXIALG_API float HRV_GetNoiseRate();
PXIALG_API float HRV_GetRRIStability();
PXIALG_API float HRV_GetRRIQuality();
PXIALG_API int HRV_GetMemoryAmount(int rri_num, int test_seconds);
PXIALG_API int HRV_SetMemorySpace(void *addr);
PXIALG_API int HRV_SetHampelThreshold(float sdnn_threshold, float lhr_threshold, float rmssd_threshold);
//-----------------------------------------

#if(SUPPORT_FILE_INPUT==1)
PXIALG_API	void HRD_SetHRDPARAM_IFS(int iFs);
#endif

PXIALG_API void HRD_Set_Debug_On();
PXIALG_API void HRD_Set_Debug_Off();

#ifdef __cplusplus
}
#endif

#endif /* _PXIALG_DYN_ */

