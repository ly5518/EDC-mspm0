#include "ADC.h"
#include "string.h"

/* 单次 ADC 读取
 * ADC 保持使能状态，每次只清标志→触发→等标志→读结果
 * 不做 disable/enable 切换，避免状态不稳定
 */
static unsigned short adc_read_single(void)
{
    DL_ADC12_clearInterruptStatus(ADC12_0_INST,
        DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED);
    DL_ADC12_startConversion(ADC12_0_INST);
    while (!(DL_ADC12_getRawInterruptStatus(ADC12_0_INST,
        DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED)))
        ;
    return (unsigned short)DL_ADC12_getMemResult(ADC12_0_INST,
        ADC12_0_ADCMEM_ADC_CH0);
}

/* 多次采样取平均 */
unsigned int adc_getValue(unsigned int number)
{
    unsigned int sum = 0;
    for (unsigned int i = 0; i < number; i++)
        sum += adc_read_single();
    return sum / number;
}
