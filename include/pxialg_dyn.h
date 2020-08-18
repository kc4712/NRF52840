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
#include "HRDParam.h"

#include "pah8series_data_c.h"
PXIALG_API sHRDParam HRD_GetHRDParam(void);
PXIALG_API int HRD_ALG_VERSION(void);
PXIALG_API bool HRDProcessTop(UInt8 HRDOpMode,bool DataReadyFlag,float OFMRawData, float OFMRawData2);
PXIALG_API unsigned int HRDOpen_i1000Hz(void * p);
PXIALG_API unsigned int HRDOpen_i350Hz(void *p);
PXIALG_API unsigned int HRDOpen_i200Hz(void *p);
PXIALG_API unsigned int HRDOpen_i100Hz(void *p);
PXIALG_API unsigned int HRDOpen_i20Hz(void *p);
PXIALG_API unsigned int HRDOpenParam(sHRDParam *sParam, void *p);
PXIALG_API void HRDReset(void);
PXIALG_API void HRDClose(void);
PXIALG_API void HRD_SDNNProcess(void);
PXIALG_API void NI_HRDProcess(float OFMRawData);
PXIALG_API int HRD_GetSDNN(void);
PXIALG_API int HRD_GetSDNNSNR(void);		//20150326
PXIALG_API int HRD_GetSDNNRawDat(void);
PXIALG_API float HRD_Get_NI_SDNN(void);
PXIALG_API int HRD_Get_SDNN_HRAvg(void);
PXIALG_API float HRD_Get_NI_SDNN_HRAvg(void);
PXIALG_API int HRD_Get_HRAvg(void);
PXIALG_API bool HRD_Get_ReadyFlag(void);
PXIALG_API int HRD_Get_ReadyTime(void);
PXIALG_API float HRD_Get_SDNNKurtosis(void);
PXIALG_API float HRD_Get_NI_SDNNKurtosis(void);
PXIALG_API float HRD_Get_Kurtosis(void);
PXIALG_API void HRD_Set_FreqOffset(float FreqOffset);
PXIALG_API void HRD_Set_Diff_Time(float DiffTime,int SampleNum);
PXIALG_API float HRD_Get_HR_STD(void);
PXIALG_API bool HRD_Get_MotionFlag(void);
PXIALG_API void HRD_Set_SDNN_On(void);
PXIALG_API void HRD_Set_SDNN_Off(void);
PXIALG_API void HRD_Set_LHRatio_On(void);
PXIALG_API void HRD_Set_LHRatio_Off(void);
PXIALG_API void HRD_Set_SkipCounterTh(int SkipCounterTh);
PXIALG_API void HRD_Set_ReadyCounter_Th(int ReadyCounter_Th0,int ReadyCounter_Th1,int ReadyCounter_Th2,int ReadyCounter_Th3,int ReadyCounter_Th4,int ReadyCounter_Th5);
PXIALG_API void HRD_GetSDNNBoundary(int m_iAge, double *pdSameAgeSDNN, double *pdDetectMax, double *pdDetectMin) ;
PXIALG_API int HRD_GetSDNNLevel(int m_iAge, double dSDNN);
PXIALG_API void HRD_Set_SkipCounterTh(int SkipCounterTh);
PXIALG_API void HRD_Set_PI_Calculate(float PI_dc_offset, float PI_scale);
PXIALG_API float HRD_Get_PI_Index(void);
PXIALG_API int HRD_GetRRIReportFlag(void);
PXIALG_API float HRD_GetRRISingleRRI(void);
PXIALG_API int HRD_GetPeakAmplitude(void);
PXIALG_API int HRD_GetRRIBufferStatus(void);
PXIALG_API int HRD_GetRRIDataNumber(void);
PXIALG_API int HRD_GetRRIDataBuffer(float *pBuffer);
PXIALG_API int HRD_GetSDNNRRIDataNumber(void);
PXIALG_API int HRD_GetSDNNRRIDataBuffer(float *pBuffer);
//float HRD_GetLHRatio(float *InputBuffer);
PXIALG_API int HRD_GetSignalQualtiy(void);
PXIALG_API void HRD_Set_PeakProcessReadyCounter_Th(	int PeakProcessReady3Counter_Th0,
											int PeakProcessReady3Counter_Th1,
											int PeakProcessReady3Counter_Th2,
											int PeakProcessReady3Counter_Th3,
											int m_PeakProcessReady3Counter_HeartRate_Th1,
											int m_PeakProcessReady3Counter_HeartRate_Th2,
											int m_PeakProcessReady3Counter_HeartRate_Th3);
PXIALG_API void HRD_Set_HeartRateNotReadyBoundary(int HeartRateNotReadyUpBound,int HeartRateNotReadyLowBound);
PXIALG_API void HRD_Set_Support2PeakCounterOutuptRatio(bool bSupport2PeakCounterOutuptRatio);

PXIALG_API int HRV_GetMemorySize(void);

PXIALG_API void HRD_HRVProcess(float *InputBuffer);

PXIALG_API float HRD_GetHRV_LHRatio(void);
PXIALG_API float HRD_GetHRV_LF(void);
PXIALG_API float HRD_GetHRV_HF(void);
PXIALG_API float HRD_GetHRV_CVRR(void);
PXIALG_API float HRD_GetHRV_SDNN(void);

PXIALG_API int HRD_GetSignalQualtiy(void);
PXIALG_API int HRV_GetMemorySize(void);
PXIALG_API void HRV_SetMemory(void *pBuffer);


PXIALG_API void OFM_DisplayReset(void);
PXIALG_API void OFM_DisplayOpen(void);
PXIALG_API void OFM_DisplayClose(void);
PXIALG_API void OFM_DisplayProcess(float OFMRawData,double *pDisplayBuffer,int MaxBufferSize);
PXIALG_API void OFM_DisplayProcess_int(float OFMRawData,int *pDisplayBuffer,int MaxBufferSize);
PXIALG_API float OFM_GetDisplayData(void);

PXIALG_API void HRD_HRVProcess(float *InputBuffer);

//-----------------------------------------
// added by yuanhsin (v1.001)
//-----------------------------------------
PXIALG_API int HRV_Initialize(float *rri_buf); 
PXIALG_API int HRV_Operate(void);  
PXIALG_API float HRV_GetSDNN(void);
PXIALG_API int HRV_GetPowerSpectrumNumber(void);
PXIALG_API int HRV_GetPowerSpectrumBuffer(float *pBuffer);
PXIALG_API float HRV_GetVLFPeak(void);
PXIALG_API float HRV_GetVLFPower(void);
PXIALG_API float HRV_GetLFPeak(void);
PXIALG_API float HRV_GetLFPower(void);
PXIALG_API float HRV_GetLFNormalizedPower(void);
PXIALG_API float HRV_GetHFPeak(void);
PXIALG_API float HRV_GetHFPower(void);
PXIALG_API float HRV_GetHFNormalizedPower(void);
PXIALG_API float HRV_GetLHRatio(void);
PXIALG_API float HRV_GetRMSSD(void);
PXIALG_API int HRV_GetSleepQuality(int age);
PXIALG_API float HRV_GetNoiseRate(void);
PXIALG_API float HRV_GetRRIStability(void);
PXIALG_API float HRV_GetRRIQuality(void);
PXIALG_API int HRV_GetMemoryAmount(int rri_num, int test_seconds);
PXIALG_API int HRV_SetMemorySpace(void *addr);
PXIALG_API int HRV_SetHampelThreshold(float sdnn_threshold, float lhr_threshold, float rmssd_threshold);
//-----------------------------------------

PXIALG_API float HRD_GetHRV_LHRatio(void);
PXIALG_API float HRD_GetHRV_LF(void);
PXIALG_API float HRD_GetHRV_HF(void);
PXIALG_API float HRD_GetHRV_CVRR(void);
PXIALG_API float HRD_GetHRV_SDNN(void);
#if(SUPPORT_FILE_INPUT==1)
PXIALG_API	void HRD_SetHRDPARAM_IFS(int iFs);
#endif

PXIALG_API void HRD_Set_Debug_On(void);
PXIALG_API void HRD_Set_Debug_Off(void);

#ifdef __cplusplus
}
#endif

#endif /* _PXIALG_DYN_ */

