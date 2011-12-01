/*
 */

/* I2C chip addresses */
#define SMB347_ADDRESS        0x6

/*  REGISTERS */
#define SUMMIT_SMB347_CHARGE_CURRENT	0x0
#define SUMMIT_SMB347_INPUT_CURR_LIMIT	0x1
#define SUMMIT_SMB347_FUNCTIONS		0x2
#define SUMMIT_SMB347_FLOAT_VOLTAGE	0x3
#define SUMMIT_SMB347_CHARGE_CONTROL	0x4
#define SUMMIT_SMB347_STAT_TIMERS	0x5
#define SUMMIT_SMB347_ENABLE_CONTROL	0x6
#define SUMMIT_SMB347_THERMAL_CONTROL	0x7
#define SUMMIT_SMB347_SYSOK_USB30	0x8
#define SUMMIT_SMB347_OTHER_CONTROL_A	0x9
#define SUMMIT_SMB347_OTG_THERM_CONTROL	0xA
#define SUMMIT_SMB347_CELL_TEMP		0xB
#define SUMMIT_SMB347_FAULT_INTERRUPT	0xC
#define SUMMIT_SMB347_INTERRUPT_STAT	0xD
#define SUMMIT_SMB347_SLAVE_ADDR	0xE

/*  Command register*/
#define SUMMIT_SMB347_COMMAND_REG_A	0x30
#define SUMMIT_SMB347_COMMAND_REG_B	0x31
#define SUMMIT_SMB347_COMMAND_REG_C	0x33

#define SUMMIT_SMB347_INTSTAT_REG_A	0x35
#define SUMMIT_SMB347_INTSTAT_REG_B	0x36
#define SUMMIT_SMB347_INTSTAT_REG_C	0x37
#define SUMMIT_SMB347_INTSTAT_REG_D	0x38
#define SUMMIT_SMB347_INTSTAT_REG_E	0x39
#define SUMMIT_SMB347_INTSTAT_REG_F	0x3A

#define SUMMIT_SMB347_STATUS_REG_A	0x3B
#define SUMMIT_SMB347_STATUS_REG_B	0x3C
#define SUMMIT_SMB347_STATUS_REG_C	0x3D
#define SUMMIT_SMB347_STATUS_REG_D	0x3E
#define SUMMIT_SMB347_STATUS_REG_E	0x3F

/*R0[7:5]Fast Charge Current*/
#define FCC_700mA  (0)
#define FCC_900mA  (1 << 5)
#define FCC_1200mA (2 << 5)
#define FCC_1500mA (3 << 5)
#define FCC_1800mA (4 << 5)
#define FCC_2000mA (5 << 5)
#define FCC_2200mA (6 << 5)
#define FCC_2500mA (7 << 5)
#define FCC_CLEAN  (7 << 5)
/*R0[4:3]Pre-charge Current*/
#define PCC_100mA  (0)
#define PCC_150mA  (1 << 3)
#define PCC_200mA  (2 << 3)
#define PCC_250mA  (3 << 3)
#define PCC_CLEAN  (3 << 3)
/*R0[2:0]Termination Current*/
#define TC_37mA    (0)
#define TC_50mA    (1)
#define TC_100mA   (2)
#define TC_150mA   (3)
#define TC_200mA   (4)
#define TC_250mA   (5)
#define TC_500mA   (6)
#define TC_600mA   (7)
#define TC_CLEAN   (7)
#define CONFIG_NO_CHANGE  (-1)

/*R1[7:4] Max. DCIN Input current limit*/
#define DC_ICL_300mA (0)
#define DC_ICL_500mA (1 <<4)
#define DC_ICL_700mA (2 <<4)
#define DC_ICL_900mA (3 <<4)
#define DC_ICL_1200mA (4 <<4)
#define DC_ICL_1500mA (5 <<4)
#define DC_ICL_1800mA (6 <<4)
#define DC_ICL_2000mA (7 <<4)
#define DC_ICL_2200mA (8 <<4)
#define DC_ICL_2500mA (9 <<4)
#define DC_ICL_CLEAN (15 <<4)
/*R1[3:0] Max. USBIN HC Input Current Limit*/
#define USBHC_ICL_500mA (1)
#define USBHC_ICL_700mA (2)
#define USBHC_ICL_900mA (3)
#define USBHC_ICL_1200mA (4)
#define USBHC_ICL_1500mA (5)
#define USBHC_ICL_1800mA (6)
#define USBHC_ICL_2000mA (7)
#define USBHC_ICL_2200mA (8)
#define USBHC_ICL_2500mA (9)
#define USBHC_ICL_CLEAN  (15)
/*R2[7]Input to system FET(suspend) on/off control, SUSP pin or register*/
#define SUS_CTRL_BY_SUS_PIN  (0)
#define SUS_CTRL_BY_REGISTER (1<<7)
/*R2[6]Battery to system power control ,Normal or FET is turn off*/
#define BAT_TO_SYS_NORMAL 0
#define FET_TURN_OFF (1<<6)
/*R2[5]Maximum system voltage,Vflt+0.1V or Vflt+0.2V*/
#define VFLT_PLUS_100mV (0)
#define VFLT_PLUS_200mV (1<<5)
/*R2[4]Automatic Input Current LImit,*/
#define AICL_DISABLE (0)
#define AICL_ENABLE (1<<4)
#define SET_AICL_DISABLE(X) CLEAR_BIT(X,4)
#define SET_AICL_ENABLE(X) SET_BIT(X,4)
#define IS_AICL_ENABLE(X) IS_BIT_SET(X,4)
#define IS_AICL_DISABLE(X) IS_BIT_CLEAR(X,4)
/*R2[3]Automatic Input Current Limit Threshold,4.25 or 4.5*/
#define AIC_TH_4200mV (0)
#define AIC_TH_4500mV (1<<3)
#define SET_AIC_TH_4200mV(X) CLEAR_BIT(X,3)
#define SET_AIC_TH_4500mV(X) SET_BIT(X,3)
/*R2[2]Input Source Priority(When both DCIN and USBIN are valid)*/
#define DC_IN_FIRST  (0)
#define USB_IN_FIRST (1<<2)
/*R2[1]Battery OV,*/
#define BAT_OV_DOES_NOT_END_CHARGE (0)
#define BAT_OV_END_CHARGE (1<<1)
/*R2[0]VCHG*/
#define VCHG_DISABLE (0)
#define VCHG_ENABLE (1)
/*R3[7:5]Pre_charge to Fast-charge voltage threshold*/
#define PRE_CHG_VOLTAGE_THRESHOLD_2_4 (0)
#define PRE_CHG_VOLTAGE_THRESHOLD_2_6 (1<<6)
#define PRE_CHG_VOLTAGE_THRESHOLD_2_8 (2<<6)
#define PRE_CHG_VOLTAGE_THRESHOLD_3_0 (3<<6)
#define PRE_CHG_VOLTAGE_CLEAN         (3<<6)
/*R3[7:5]Float_charge_voltage*/
#define FLOAT_VOLTAGE_3_5_0 (0)
#define FLOAT_VOLTAGE_3_5_2 (1)
#define FLOAT_VOLTAGE_3_5_4 (2)
#define FLOAT_VOLTAGE_4_0_0 (25)
#define FLOAT_VOLTAGE_4_2_0 (35)
#define FLOAT_VOLTAGE_CLEAN (63)

/*R4[7]Automatic Recharge*/
#define AUTOMATIC_RECHARGE_ENABLE  (0)
#define AUTOMATIC_RECHARGE_DISABLE (1<<7)
/*R4[6]Current Termination*/
#define CURRENT_TERMINATION_ENABLE  (0)
#define CURRENT_TERMINATION_DISABLE (1<<6)
/*R4[5:4]Battery Missing Detection*/
#define BMD_DISABLE              (0)
#define BMD_EVERY_3S             (1<<4)
#define BMD_ONCE                 (2<<4)
#define BMD_VIA_THERM_IO         (3<<4)
/*R4[3]Auto Recharge Threshold*/
#define AUTO_RECHARGE_50mV      (0)
#define AUTO_RECHARGE_100mV     (1<<3)
/*R4[2]Automatic Power Source Detection*/
#define APSD_DISABLE      (0)
#define APSD_ENABLE       (1<<2)
#define SET_APSD_DISABLE(X) CLEAR_BIT(X,2)
#define SET_APSD_ENABLE(X) SET_BIT(X,2)
#define IS_APSD_ENABLE(X) IS_BIT_SET(X,2)
/*R4[1]Non-conforming(NC) Charger Detection via APSD(1 second timeput)*/
#define NC_APSD_DISABLE          (0)
#define NC_APSD_ENABLE         (1<<1)
#define SET_NC_APSD_DISABLE(X) CLEAR_BIT(X,1)
#define SET_NC_APSD_ENABLE(X) SET_BIT(X,1)
#define IS_NC_APSD_ENABLE(X) IS_BIT_SET(X,1)
/*R4[0]Primary Input OVLO Select*/
#define SECONDARY_INPUT_NOT_ACCEPTED_IN_OVLO     (0)
#define SECONDARY_INPUT_ACCEPTED_IN_OVLO         (1)

/*R5[7]STAT and Timers Control*/
#define STAT_ACTIVE_LOW      (0)
#define STAT_ACTIVE_HIGH     (1<<7)
/*R5[6]STAT Output Mode*/
#define STAT_CHARGEING_STATE      (0)
#define STAT_USB_FAIL         (1<<6)
/*R5[5]STAT Output Control*/
#define STAT_ENABLE          (0)
#define STAT_DISABLE         (1<<5)
/*R5[4]NC Charger Input Current Limit*/
#define NC_INPUT_100mA      (0)
#define NC_INPUT_HC_SETTING     (1<<4)
/*R5[3:2]Complete Charge Timeout*/
#define CC_TIMEOUT_382MIN      (0)
#define CC_TIMEOUT_764MIN     (1<<2)
#define CC_TIMEOUT_1527MIN     (2<<2)
#define CC_TIMEOUT_DISABLED     (3<<2)
/*R5[1:0]Pre-Charge Timeout*/
#define PC_TIMEOUT_48MIN      (0)
#define PC_TIMEOUT_95MIN     (1)
#define PC_TIMEOUT_191MIN     (2)
#define PC_TIMEOUT_DISABLED     (3)

/*R6[7]LED Blinking function*/
#define LED_BLINK_DISABLE      (0)
#define LED_BLINK_ENABLE      (1<<7)
/*R6[6:5]Enable (EN) Pin Control */
#define CHARGE_EN_I2C_0         (0)
#define CHARGE_EN_I2C_1         (1<<5)
#define EN_PIN_ACTIVE_HIGH     (2<<5)
#define EN_PIN_ACTIVE_LOW     (3<<5)
/*R6[4]USB/HC Control */
#define USB_HC_CONTROL_BY_REGISTER    (0)
#define USB_HC_CONTROL_BY_PIN          (1<<4)
/*R6[3]USB/HC Input State */
#define USB_HC_TRI_STATE        (0)
#define USB_HC_DUAL_STATE          (1<<3)
/*R6[2]Charger Error  */
#define CHARGER_ERROR_NO_IRQ        (0)
#define CHARGER_ERROR_IRQ          (1<<2)
/*R6[1]APSD Done  */
#define APSD_DONE_NO_IRQ        (0)
#define APSD_DONE_IRQ              (1<<1)
/*R6[0]DCIN Input Pre-bias  */
#define DCIN_INPUT_PRE_BIAS_DISABLE    (0)
#define DCIN_INPUT_PRE_BIAS_ENABLE     (1)

/*R7[6]Minimum System Voltage*/
#define MIN_SYS_3_4_5_V      (0)
#define MIN_SYS_3_6_V          (1<<6)
/*R7[5]THERM Monitor Selection*/
#define THERM_MONITOR_USBIN      (0)
#define THERM_MONITOR_VDDCAP      (1<<5)
/*R7[4]Thermistor Monitor*/
#define THERM_MONITOR_ENABLE      (0)
#define THERM_MONITOR_DISABLE      (1<<4)
/*R7[3:2]Soft Cold Temp Limit Behavior*/
#define SOFT_COLD_NO_RESPONSE          (0)
#define SOFT_COLD_CC_COMPENSATION      (1<<2)
#define SOFT_COLD_FV_COMPENSATION      (2<<2)
#define SOFT_COLD_CC_FV_COMPENSATION      (3<<2)
/*R7[1:0]Soft Hot Temp Limit Behavior*/
#define SOFT_HOT_NO_RESPONSE          (0)
#define SOFT_HOT_CC_COMPENSATION      (1)
#define SOFT_HOT_FV_COMPENSATION      (2)
#define SOFT_HOT_CC_FV_COMPENSATION      (3)

/*R8[7:6]SYSOK/CHG_DET_N Output Operation*/
#define INOK_OPERATION          (0)
#define SYSOK_OPERATION_A          (1<<6)
#define SYSOK_OPERATION_B          (2<<6)
#define CHG_DET_N_OPERATION           (3<<6)
/*R8[5]USB2.0/USB3.0 Input Current Limit*/
#define USB_2                  (0)
#define USB_3                   (1<<5)
/*R8[4:3]Float Voltage Compensation*/
#define VFLT_MINUS_60mV          (0)
#define VFLT_MINUS_120mV          (1<<3)
#define VFLT_MINUS_180mV          (2<<3)
#define VFLT_MINUS_240mV          (3<<3)
/*R8[2]Hard temp Limit Behavior*/
#define HARD_TEMP_CHARGE_SUSPEND     (0)
#define HARD_TEMP_CHARGE_NO_SUSPEND     (1<<2)
/*R8[1]Pre-charge to fast-charge Threshold*/
#define PC_TO_FC_THRESHOLD_ENABLE     (0)
#define PC_TO_FC_THRESHOLD_DISABLE     (1<<1)
/*R8[0]INOK Polarity*/
#define INOK_ACTIVE_LOW             (0)
#define INOK_ACTIVE_HIGH         (1<<0)

/*R9[7:6]OTG/ID Pin Control*/
#define RID_DISABLE_OTG_I2C_CONTROL     (0)
#define RID_DISABLE_OTG_PIN_CONTROL     (1<<6)
#define RID_ENABLE_OTG_I2C_CONTROL     (2<<6)
#define RID_ENABLE_OTG_AUTO         (3<<6)
/*R9[5]OTG Pin Polarity*/
#define OTG_PIN_ACTIVE_HIGH          (0)
#define OTG_PIN_ACTIVE_LOW         (1<<5)
/*R9[3:0]Low-Battery / SYSOK Voltage Threshold*/
#define LOW_BAT_VOLTAGE_DISABLE          (0)
#define LOW_BAT_VOLTAGE_3_4_6_V          (14)
#define LOW_BAT_VOLTAGE_3_5_8_V          (0xf)
/*RA[7:6]Charge Current Compensation */
#define CCC_250mA             (0)
#define CCC_700mA              (1<<6)
#define CCC_900mA              (2<<6)
#define CCC_1200mA              (3<<6)
/*RA[5:4]Digital Thermal Regulation Temperature Threshold */
#define DTRT_100C             (0)
#define DTRT_110C              (1<<4)
#define DTRT_120C              (2<<4)
#define DTRT_130C              (3<<4)
/*RA[3:2]OTG Current Limit at USBIN */
#define OTG_CURRENT_LIMIT_100mA         (0)
#define OTG_CURRENT_LIMIT_250mA          (1<<2)
#define OTG_CURRENT_LIMIT_500mA          (2<<2)
#define OTG_CURRENT_LIMIT_750mA          (3<<2)
/*RA[1:0]OTG Battery UVLO Threshold */
#define OTG_BAT_UVLO_THRES_2_7_V         (0)
#define OTG_BAT_UVLO_THRES_2_9_V          (1)
#define OTG_BAT_UVLO_THRES_3_1_V          (2)
#define OTG_BAT_UVLO_THRES_3_3_V          (3)

/*RC FAULT Interrupt Register ,Setting what condition assert an interrupt ,*/
#define TEMP_OUTSIDE_COLD_HOT_HARD_LIMIITS_TRIGGER_IRQ (1<<7)
#define TEMP_OUTSIDE_COLD_HOT_SOFT_LIMIITS_TRIGGER_IRQ (1<<6)
#define OTG_BAT_FAIL_ULVO_TRIGGER_IRQ            (1<<5)
#define OTG_OVER_CURRENT_LIMIT_TRIGGER_IRQ            (1<<4)
#define USB_OVER_VOLTAGE_TRIGGER_IRQ            (1<<3)
#define USB_UNDER_VOLTAGE_TRIGGER_IRQ            (1<<2)
#define AICL_COMPLETE_TRIGGER_IRQ                (1<<1)
#define INTERNAL_OVER_TEMP_TRIGGER_IRQ            (1)
/*RD STATUS Interrupt Register,Setting what condition assert an interrupt ,*/
#define CHARGE_TIMEOUT_TRIGGER_IRQ                (1<<7)
#define OTG_INSERTED_REMOVED_TRIGGER_IRQ            (1<<6)
#define BAT_OVER_VOLTAGE_TRIGGER_IRQ            (1<<5)
#define TERMINATION_OR_TAPER_CHARGING_TRIGGER_IRQ       (1<<4)
#define FAST_CHARGING_TRIGGER_IRQ                   (1<<3)
#define INOK_TRIGGER_IRQ                    (1<<2)
#define MISSING_BATTERY_TRIGGER_IRQ                (1<<1)
#define LOW_BATTERY_TRIGGER_IRQ                (1)

/*Interrupt Status Register A*/
#define HOT_TEMP_HARD_LIMIT_IRQ                (1<<7)
#define HOT_TEMP_HARD_LIMIT_STATUS                (1<<6)
#define COLD_TEMP_HARD_LIMIT_IRQ                (1<<5)
#define COLD_TEMP_HARD_LIMIT_STATUS                (1<<4)
#define HOT_TEMP_SOFT_LIMIT_IRQ                (1<<3)
#define HOT_TEMP_SOFT_LIMIT_STATUS                (1<<2)
#define COLD_TEMP_SOFT_LIMIT_IRQ                   (1<<1)
#define COLD_TEMP_SOFT_LIMIT_STATUS                (1)
/*Interrupt Status Register B*/
#define BAT_OVER_VOLTAGE_IRQ                (1<<7)
#define BAT_OVER_VOLTAGE_STATUS                (1<<6)
#define MISSING_BAT_IRQ                (1<<5)
#define MISSING_BAT_STATUS                (1<<4)
#define LOW_BAT_VOLTAGE_IRQ                (1<<3)
#define LOW_BAT_VOLTAGE_STATUS                (1<<2)
#define PRE_TO_FAST_CHARGE_VOLTAGE_IRQ                   (1<<1)
#define PRE_TO_FAST_CHARGE_VOLTAGE_STATUS                (1)
/*Interrupt Status Register C*/
#define INTERNAL_TEMP_LIMIT_IRQ                (1<<7)
#define INTERNAL_TEMP_LIMIT_STATUS                (1<<6)
#define RE_CHARGE_BAT_THRESHOLD_IRQ                (1<<5)
#define RE_CHARGE_BAT_THRESHOLD_STATUS                (1<<4)
#define TAPER_CHARGER_MODE_IRQ                (1<<3)
#define TAPER_CHARGER_MODE_STATUS                (1<<2)
#define TERMINATION_CC_IRQ                   (1<<1)
#define TERMINATION_CC_STATUS                (1)
/*Interrupt Status Register D*/
#define APSD_COMPLETED_IRQ                (1<<7)
#define APSD_COMPLETED_STATUS                (1<<6)
#define AICL_COMPLETED_IRQ                (1<<5)
#define AICL_COMPLETED_STATUS                (1<<4)
#define COMPLETE_CHARGE_TIMEOUT_IRQ                (1<<3)
#define COMPLETE_CHARGE_TIMEOUT_STATUS                (1<<2)
#define PC_TIMEOUT_IRQ                   (1<<1)
#define PC_TIMEOUT_STATUS                (1)
/*Interrupt Status Register E*/
#define DCIN_OVER_VOLTAGE_IRQ                (1<<7)
#define DCIN_OVER_VOLTAGE_STATUS                (1<<6)
#define DCIN_UNDER_VOLTAGE_IRQ                (1<<5)
#define DCIN_UNDER_VOLTAGE_STATUS                (1<<4)
#define USBIN_OVER_VOLTAGE_IRQ                (1<<3)
#define USBIN_OVER_VOLTAGE_STATUS                (1<<2)
#define USBIN_UNDER_VOLTAGE_IRQ                (1<<1)
#define USBIN_UNDER_VOLTAGE_STATUS                (1)
/*Interrupt Status Register F*/
#define OTG_OVER_CURRENT_IRQ                (1<<7)
#define OTG_OVER_CURRENT_STATUS                (1<<6)
#define OTG_BAT_UNDER_VOLTAGE_IRQ                (1<<5)
#define OTG_BAT_UNDER_VOLTAGE_STATUS                (1<<4)
#define OTG_DETECTION_IRQ                (1<<3)
#define OTG_DETECTION_STATUS                (1<<2)
#define POWER_OK_IRQ                (1<<1)
#define POWER_OK_STATUS                (1)
/* Bit operator */
#define IS_BIT_SET(reg, x)        (((reg)&(0x1<<(x)))!=0)
#define IS_BIT_CLEAR(reg, x)        (((reg)&(0x1<<(x)))==0)
//#define BIT(x)                  (1 << (x))
#define CLEAR_BIT(reg, x)         ((reg)&=(~(0x1<<(x))))
#define SET_BIT(reg, x)           ((reg)|=(0x1<<(x)))



/*COMMAND A*/
#define COMMAND_NOT_ALLOW_WRITE_TO_CONFIG_REGISTER(X)       CLEAR_BIT(X,7)
#define COMMAND_ALLOW_WRITE_TO_CONFIG_REGISTER(X)           SET_BIT(X,7)
#define COMMAND_FORCE_PRE_CHARGE_CURRENT_SETTING(X)         CLEAR_BIT(X,6)
#define COMMAND_ALLOW_FAST_CHARGE_CURRENT_SETTING(X)        SET_BIT(X,6)
#define COMMAND_OTG_DISABLE(X)                              CLEAR_BIT(X,4)
#define COMMAND_OTG_ENABLE(X)                               SET_BIT(X,4)
#define COMMAND_BAT_TO_SYS_NORMAL(X)                        CLEAR_BIT(X,3)
#define COMMAND_TURN_OFF_BAT_TO_SYS(X)                      SET_BIT(X,3)
#define COMMAND_SUSPEND_MODE_DISABLE(X)                     CLEAR_BIT(X,2)
#define COMMAND_SUSPEND_MODE_ENABLE(X)                      SET_BIT(X,2)
#define COMMAND_CHARGING_DISABLE(X)                         CLEAR_BIT(X,1)
#define COMMAND_CHARGING_ENABLE(X)                          SET_BIT(X,1)
#define COMMAND_STAT_OUTPUT_ENABLE(X)                       CLEAR_BIT(X,0)
#define COMMAND_STAT_OUTPUT_DISABLE(X)                      SET_BIT(X,0)        

/*COMMAND B*/
#define COMMAND_POWER_ON_RESET(X)        SET_BIT(X,7)
#define COMMAND_USB1(X)                  CLEAR_BIT(X,1)
#define COMMAND_USB5(X)                  SET_BIT(X,1)
#define COMMAND_USB_MODE(X)              CLEAR_BIT(X,0)
#define COMMAND_HC_MODE(X)               SET_BIT(X,0)

#define ACTUAL_FLOAT_VOLTAGE(X)    (X & 0x3F)


/*INTERRUPT_STATUS_B*/
#define BAT_OVER_V_IRQ_BIT(X)        ( (X & 0x80) >> 7)
    #define NO_BAT_OVER_V_IRQ    0        
    #define BAT_OVER_V_IRQ        1
#define BAT_OVER_V_STATUS_BIT(X)        ( (X & 0x40) >> 6)
    #define NO_BAT_OVER_V        0        
    #define BAT_OVER_V        1
#define BAT_MISS_IRQ_BIT(X)        ( (X & 0x20) >> 5)
    #define NO_BAT_MISS_IRQ    0        
    #define BAT_MISS_IRQ        1
#define BAT_MISS_STATUS_BIT(X)        ( (X & 0x10) >> 4)
    #define NO_BAT_MISS        0        
    #define BAT_MISS        1
#define LOW_BAT_IRQ_BIT(X)        ( (X & 0x8) >> 3)
    #define NO_LOW_BAT_IRQ        0        
    #define LOW_BAT_IRQ        1
#define LOW_BAT_STATUS_BIT(X)        ( (X & 0x4) >> 2)
    #define NO_LOW_BAT        0        
    #define LOW_BAT        1
#define PRE_TO_FAST_V_BIT(X)        ( (X & 0x2) >> 1)
    #define NO_PRE_TO_FAST_V_IRQ        0        
    #define PRE_TO_FAST_V_IRQ   
#define PRE_TO_FAST_V_STATUS_BIT(X)        ( X & 0x1)
    #define NO_PRE_TO_FAST_V        0        
    #define PRE_TO_FAST_V_        1

/*STATUS_B*/
#define USB_SUSPEND_MODE_STATUS(X)    ( (X & 0x80) >> 7)
    #define USB_SUSPEND_MODE_NOT_ACTIVE    0
    #define USB_SUSPEND_MODE_ACTIVE        1
#define ACTUAL_CHARGE_CURRENT(X)    ( (X & 0x40) >> 6)
    
/*STATUS_C*/
#define CHARGER_ERROR_IRQ_STATUS(X)    ( (X & 0x80) >> 7)
    #define CHARGER_ERROR_IRQ_NOT_ASSERT    0
    #define CHARGER_ERROR_IRQ_ASSERT    1
#define CHARGER_ERROR_STATUS(X)    ( (X & 0x40) >> 6)
    #define NO_CHARGER_ERROR    0
    #define CHARGER_ERROR        1
#define CHARGEING_CYCLE_STATUS(X)    ( (X & 0x20) >> 5)
    #define NO_CHARGER_CYCLE    0
    #define HAS_CHARGE_CYCLE    1
#define BATTERY_VOLTAGE_LEVEL_STATUS(X)    ( (X & 0x10) >> 4)
    #define ABOVE_2_1V        0
    #define BELOW_2_1V        1
#define HOLD_OFF_STATUS(X)    ( (X & 0x8) >> 3)
    #define NOT_IN_HOLD_OFF        0
    #define IN_HOLD_OFF        1
#define CHARGEING_STATUS(X)    ( (X & 0x6) >> 1)
    #define NO_CHARGING_STATUS        0
    #define PRE_CHARGING_STATUS        1
    #define FAST_CHARGING_STATUS        2
    #define TAPER_CHARGING_STATUS        3
#define CHARGEING_ENABLE_DISABLE_STATUS(X)    (X & 1)
    #define CHARGEING_DISABLED    0
    #define CHARGEING_ENABLE    1
/*STATUS_D*/
#define RID_STATUS(X)    ( (X & 0x80) >> 7)
    #define RID_NOT_DONE    0
    #define RID_DONE    1
#define ACA_STATUS(X)    ( (X & 0x70) >> 4)
    #define RID_A        0
    #define RID_B        1
    #define RID_C        2
    #define RID_FLOATING    3
    #define RID_NOT_USED    4
/*STATUS_D*/
#define APSD_STATUS(X)     ((X & 0x8) >> 3)
    #define APSD_NOT_COMPLETED              0
    #define APSD_COMPLETED                  1
#define APSD_RESULTS(X)    ( X & 0x7)
    #define APSD_NOT_RUN                    0
    #define APSD_CHARGING_DOWNSTREAM_PORT   1
    #define APSD_DEDICATED_DOWNSTREAM_PORT  2
    #define APSD_OTHER_DOWNSTREAM_PORT      3
    #define APSD_STANDARD_DOWNSTREAM_PORT   4
    #define APSD_ACA_CHARGER                5
    #define APSD_TBD	                    6

/*STATUS_E*/
#define USBIN(X)        ( (X & 0x80) >> 7)
    #define USBIN_NOT_IN_USE    0
    #define USBIN_IN_USE        1
#define USB15_HC_MODE(X)    ( (X & 0x60) >> 5)
    #define HC_MODE            0
    #define USB1_OR_15_MODE    1
    #define USB5_OR_9_MODE     2
    #define NA_MODE            3
#define AICL_STATUS(X)    ( (X & 0x10) >> 4)
    #define AICL_NOT_COMPLETED        0
    #define AICL_COMPLETED            1
#define AICL_RESULT(X)    (X & 0xF)
    #define AICL_300            0
    #define AICL_500            1
    #define AICL_700            2
    #define AICL_900            3
    #define AICL_1200            4
    #define AICL_1500            5
    #define AICL_1800            6
    #define AICL_2000            7
    #define AICL_2200            8

void summit_read_status_e();
int summit_check_bmd();
int summit_check_charge_status();
int summit_is_precharge_timeout();
int summit_is_complete_chg_timeout();
int summit_is_aicl_complete();
void summit_switch_mode(int mode);    
int summit_charge_enable(int enable);
int detect_usb();
void summit_init();
