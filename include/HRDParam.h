#ifndef _HDPARAM_H
#define _HDPARAM_H
#include "BaseType.h"
//#include "stdio.h"
//#include "math.h"
#include <stdbool.h>


typedef struct tagALEParam
{
// ALE Parameter
// pFs = 1000
//     HRDParam.ALE.mu = 0.00005;
//    HRDParam.ALE.ncoef = 2560;
//    HRDParam.ALE.offset = 50;

//pFs = 250
//    HRDParam.ALE.mu = 0.00005;
//    HRDParam.ALE.ncoef = 256;
//    HRDParam.ALE.offset = 50;
    
// pFs = 20 (NLMS)
    float  mu;
    UInt16 ncoef;
    UInt16 offset;    

// pFs = 20 (GNGD)
//    HRDParam.ALE.mu = 0.001;
//    HRDParam.ALE.ncoef = 50;
//    HRDParam.ALE.offset = 0.5;  
}sALE;

typedef struct tagPeakDetectionaram
{
      
// Peak Detection Parameter

// oFs = 1000
//    HRDParam.PeakDetection.MaxCounter = 2000;
//    HRDParam.PeakDetection.MinCounter = 240;
//    HRDParam.PeakDetection.PeakDetDB_Scale = 24;
//    HRDParam.PeakDetection.DecayFactor = 0.95;
//    HRDParam.PeakDetection.PeakDetDB = 10;
//    HRDParam.PeakDetection.MediaFilter_Enh = 1;
//    HRDParam.PeakDetection.MediaFilter_Num = 4;    
    
// oFs = 100
    UInt16 MaxCounter;
    UInt16 MinCounter;
    UInt16 PeakDetDB_Scale;    
    float DecayFactor;    
    UInt8  PeakDetDB;       
    UInt8  MediaFilter_Enh;
    UInt8  MediaFilter_Num;
	float fMinPeakRationTh;

	int	PeakCounterOutputThHi;
	int PeakCounterOutputThLo;
	int PeakCounterOutputUpTh;
	int PeakCounterOutputDnTh;
	float DecayFactorHi;
	float DecayFactorLo;
	float PeakDetDB_ScaleHi;
	float PeakDetDB_ScaleLo;
	UInt8  PeakDetDBHi;       
	UInt8  PeakDetDBLo;       
	
	float m_fPeakCountRatioTh;

}sPeakDetectionParam;

typedef struct tagNIPeakDetectionaram
{
// NI Paramter
    UInt16 Fs;
    UInt16 MaxCounter;
    UInt16 MinCounter;
    UInt16 PeakDetDB_Scale;
    float DecayFactor;
    UInt16 PeakDetDB;      
    UInt8 MediaFilter_Enh;
    UInt8 MediaFilter_Num;
}sNIPeakDetectionParam;

//#pragma pack(4)
typedef struct tagHRDParam
{
// HRD Parameter
	UInt16 iFs;
	UInt16 rFs;
	UInt16 pFs;
	UInt16 oFs;
	float rDownSampleRatio_float;  // for 350Hz only
	UInt16 rDownSampleRatio;
	UInt16 iDownSampleRatio;
	UInt16 oUpSampleRatio;
	UInt16 oPeakRatio;
	UInt8 iLPF_Enh;
	UInt8 pLPF_Enh;
	UInt8 pHPF_Enh;
	UInt8 oLPF_Enh;
	UInt16 oLPF_StartPoint;
	UInt8 pALE_Enh;
	UInt8 pALE_Type;
	UInt16 pALE_StartPoint;
	UInt8  InitDataBuffering_Enh;
	UInt16 InitDataBuffering_Data_Num;
	UInt8 InitDataBuffering_Repeat_Num;
	
	UInt8	FixedTimeInterval_Enh;
	UInt16 FixedTimeInterval_Data_Num;
	UInt8 FixedTimeInterval_Repeat_Num;
	//float *FixedTimeIntervalDataBuffer;

	float  *InitDataBuffer;
	UInt8  oLPFData_Inverse;
	float oFrequencyOffset;
	
	sALE ALE;
	sPeakDetectionParam PeakDetectionParam;
	sPeakDetectionParam RRI_PeakDetectionParam;
	sNIPeakDetectionParam NIPeakDetectionParam;
	UInt8 ConfidenceLevel;
	
	UInt8 HR_MedianFilter_Enh;
	UInt8 HR_MAFilter_Enh;
	UInt8 SDNN_MedianFilter_Enh;
	UInt8 SDNN_MAFilter_Enh;
	UInt8 NI_SDNN_MedianFilter_Enh;
	
	UInt8 Peak_Counter_Filter_Enh;
	UInt8 Peak_Counter_Data_Num;
	float Peak_Counter_AvgMaxRatio;
	float Peak_Counter_AvgMinRatio;

	UInt8  HRCheckReady_Enh;
	UInt8  CheckReady_Data_Num;
	UInt32 HRSTD_TH0;
	UInt32 HRSTD_TH1;
	UInt32 HRSTD_TH2;
	UInt32 HRSTD_TH3;
	UInt32 HRSTD_TH4;
	UInt16 HRLevel_TH0;
	UInt16 HRLevel_TH1;
	UInt16 HRLevel_TH2;
	UInt16 HRLevel_TH3;
	UInt16 HRLevel_TH4;
	UInt16 ReadyCounter_Th0;
	UInt16 ReadyCounter_Th1;
	UInt16 ReadyCounter_Th2;
	UInt16 ReadyCounter_Th3;
	UInt16 ReadyCounter_Th4;
	UInt16 ReadyCounter_Th5;
	bool   ISHRReady;	

	UInt8 HR_MedianFilter_Data_Num;
	UInt8 SDNN_MedianFilter_Data_Num;
	UInt8 fft_MedianFilter_Data_Num;  // added by yuanhsin
	UInt8 NI_SDNN_MedianFilter_Data_Num;
	
	UInt8 HR_MAFilter_Data_Num;
	UInt16 HR_MAFilter_InitValue;
	UInt8 SDNN_MAFilter_Data_Num;
	UInt16 SDNN_MAFilter_InitValue;

	UInt8 FreqOffseet_MAFilter_Data_Num;
	UInt16 FreqOffseet_MAFilter_InitValue;

	UInt8 MDKurtosis_Enh;
	UInt16 MDKurtosis_Data_Num;
	float MDKurtosis_Th;
	UInt16 MDKurtosis_RunHigh_Th;
	UInt16 MDKurtosis_RunLow_Th;
	UInt16 MDKurtosis_ReCaculationNum_Th;

	UInt8 SkipCounterTh;
	UInt8 MDKurtosisALE_Enh;
	//for SDNN
	UInt8 SDNN_Enh;
	UInt8 OutOfSTDFilter_Enh;
	UInt16 PeakNum;
	float PeakValidRatio;
	int SkipPeakCounter_Th;
	bool m_bSupporLHRatioHampelFilter;
	bool m_RunLHRatioHampelFilterFlag;
	bool m_bSupporSDNNHampelFilter;	
	bool m_RunSDNNHampelFilterFlag;
	bool m_bSupprotPeakCounterOutputInterpolation;

	bool m_bSupport_xCorrelation;
	bool m_bSupport_xCorrelation_MixedMode;
	bool m_bSupport_xCorrelation_ReplacedMode;
	float xCorrelation_PeakCounterOutput;
	float xCorrelation_Quality;
	int	xCorrelation_MinP;
	int	xCorrelation_MaxP;
	int xCorrelation_Quality_Th;
// UI Control    
//    HRDParam.DrawFigure_Enh = 0;
	UInt8 DisplayFilter_Enh;
	UInt8 iDisplayFs;
	UInt8 rDisplayFs;
	UInt8 pDisplayFs;
	UInt8 oDisplayFs;
	UInt8 pDisplayLPF_Enh;
	UInt8 pDisplayHPF_Enh;
	UInt8 oDisplayLPF_Enh;
	UInt8 iDislpayDownSampleRatio;
	UInt8 oDislpayUpSampleRatio;

	float PI_Calculate_dc_offset;
	UInt16 PI_Calculate_process_interval; 
 	UInt8	PeakDiffRatio;
	UInt8	m_bSupport_xCorrelation_ReplacedMode_10s;

	float StdInputDataBuffer[20];
	float InitBufferStdTh;

	float m_FFTValueRatio_Th;
	float Diff_FFTPeakCounterOutputRatio_Th;
	float Diff_FFT_xCorrelation_PeakCounterOutputRatio_Th;
	float Diff_xCorrelation_PeakCounterOutputRatio_Th;
	int  m_PeakProcessReady3Counter_Th0;
	int  m_PeakProcessReady3Counter_Th1;
	int  m_PeakProcessReady3Counter_Th2;
	int  m_PeakProcessReady3Counter_Th3;
	int  m_PeakProcessReady3Counter_HeartRate_Th1;
	int  m_PeakProcessReady3Counter_HeartRate_Th2;
	int  m_PeakProcessReady3Counter_HeartRate_Th3;
	int MotionDiffPeakCouter_Th;
	int m_PeakProcessNotReady3Counter_Th;
	int m_PeakProcessNotReady1Counter_Th;
	int m_PeakProcessNotReadyMotionCounter_Th;
	int Diff_FFT_MAFilter_Th;
	int Diff_PC_MAFilter_Th;
	int Diff_xPC_MAFilter_Th;

	bool	m_bSupport2PeakCounterOutuptRatio;
	int	HeartRateNotReadyUpBound;
	int HeartRateNotReadyLowBound;
	//For RRI	
	UInt16 oUp_RRI_SampleRatio;
	UInt8  oLPF_RRI_Enh;
	UInt16 oLPF_RRI_StartPoint;
	bool m_bSupprot_RRI_PeakCounterOutputInterpolation;
	UInt8 m_iSelRRIInputDataType;
	UInt8 LHRatio_Enh;
	float FFTMaxValueRatio_Th;
	bool m_bHighNoiseFlag;
	bool DEBUG_MODE;
}sHRDParam;
//#pragma pack()
#endif
