
#include "nrf_adc.h"





/* adc level = 1313V) ~ 185(4.3V) */

unsigned char battery_adc_table[60]=
{
	0,	/* 131, 3.05V */
	0,	/* 132,   */
	0,	/* 133, 3.1V */
	1,	/* 134,  */
	1,	/* 135, 3.15 */
	1,	/* 136,  */
	1,	/* 137, 3.2 */
	1,	/* 138, */
	1,	/* 139, 3.25  */
	1,	/* 140,  */

	1,	/* 141,  */
	1,	/* 142, 3.3 */
	1,	/* 143, */
	1,	/* 144, 3.35 */
	2,	/* 145,  */
	3,	/* 146, 3.4 */
	5,	/* 147, */
	7,	/* 148, 3.45V */
	9,	/* 149,  */
	12,	/* 150,  3.5 */

	13,	/* 151,  */
	15,	/* 152,  */
	17,	/* 153,  3.55 */
	19,	/* 154,   */
	21,	/* 155,  3.6 */
	24,	/* 156,   */
	26,	/* 157,  3.65  */
	28,	/* 158,   */
	31,	/* 159,  3.7 */
	33,	/* 160,   */
	
	35,	/* 161,  3.75 */
	37,	/* 162,  */
	40,	/* 163,   3.8 */
	46,	/* 164,   */
	53,	/* 165,  3.85 */
	58,	/* 166,  */
	63,	/* 167,  3.9 */
	67,	/* 168,   */
	70,	/* 169,  3.95 */
	75,	/* 170,   */

	79,	/* 171,  4  */
	82,	/* 172,   */
	85,	/* 173,   4.05 */	
	88,	/* 174,   */	
	90,	/* 175,   */	
	93,	/* 176,   4.1 */	
	96,	/* 177,   */	
	98,	/* 178,   4.15 */	
	99,	/* 179,   */	
	100,	/* 180,   4.2 */	

	100,	/* 181,    */
	100,	/* 182,   4.25 */
	100,	/* 183,    */
	100,	/* 184,   */
	100,	/* 185,   4.3 */
	100,	/* 186,   */
	100,	/* 187,   */
	100,	/* 188,   */
	100,	/* 189,   */
	100,	/* 190,   */
	
};



unsigned char get_battery_level(unsigned char adc)
{
	short conv;

	conv=adc-131;

	if(conv<0)
		conv=0;

	if(conv>59)
		conv=59;
	
	return battery_adc_table[conv];
}



void nrf_adc_init(ADC_Res_t ADC_res, ADC_input_selection_t ADC_input_selection, ADC_interrupt_enabled_t ADC_interrupt_enabled)
{
    if(ADC_input_selection <= 7)
    {
        NRF_ADC->CONFIG = ADC_res << ADC_CONFIG_RES_Pos | ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos |
                          ADC_CONFIG_REFSEL_SupplyOneThirdPrescaling << ADC_CONFIG_REFSEL_Pos | (1 << ADC_input_selection) << ADC_CONFIG_PSEL_Pos;
        NRF_ADC->ENABLE = 1;  
        if(ADC_interrupt_enabled)
        {
            NRF_ADC->INTENSET = ADC_INTENSET_END_Msk;
            NVIC_SetPriority(ADC_IRQn, 1);
            NVIC_EnableIRQ(ADC_IRQn);
        }
    }
}

/*
void nrf_adc_init_vbat(void)
{
    NRF_ADC->CONFIG = ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos | ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos |
                      ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos | ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos;    
    NRF_ADC->ENABLE = 1;
}
*/

uint32_t nrf_adc_read(void)
{
  uint32_t adc_data;
  NRF_ADC->TASKS_START = 1;
  while(NRF_ADC->EVENTS_END == 0);
  NRF_ADC->EVENTS_END = 0;
  adc_data = NRF_ADC->RESULT;
  return adc_data;
}

/*
float nrf_adc_read_vbat_f(void)
{
    return (float)nrf_adc_read() * 3.0f * 1.2f / 1024.0f;
}

uint32_t nrf_adc_read_vbat_mv(void)
{
    // The non-optimized math is: (ADC / 1024) * 3 * 1.2 * 1000
    return nrf_adc_read() * (3 * 6 * 1000 / 5) / 1024;
}
*/



