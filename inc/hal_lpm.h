#ifndef _HAL_LPM_H_
#define _HAL_LPM_H_

#define HAL_LPM_ENABLE 0

#if HAL_LPM_ENABLE
#define hal_lpm_enter()     __low_power_mode_3()
#define hal_lpm_exit()      __low_power_mode_off_on_exit()
#else
#define hal_lpm_enter();
#define hal_lpm_exit();
#endif

#endif // _HAL_LPM_H_
