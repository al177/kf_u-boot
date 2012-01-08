/*
 * (C) Copyright 2011
 * Eric
 *
 */
#include <config.h>
#include <otter.h>
#ifdef CONFIG_SMB347
#include <smb347.h>
#include <bq27541.h>
#include <asm/io.h>
#define TEMPERATURE_COLD_LIMIT -19
#define TEMPERATURE_HOT_LIMIT 59
#define LOW_BATTERY_CAPACITY_LIMIT 3
#define LOW_BATTERY_VOLTAGE_LIMIT 3450
#define LOW_LCD_VOLTAGE_LIMIT 3300
#define NO_VBUS -2
#define mdelay(n) ({ unsigned long msec = (n); while (msec--) udelay(1000); })
int get_bat_temperature();
int get_bat_voltage();
int get_bat_current();
void initialize_lcd(int);
void turn_off_lcd();
int twl6030_get_power_button_status();
static int kc1_chargerdetect_setting[] = {
/*0*/FCC_2500mA|PCC_150mA|TC_250mA,                                                     //0xed,
/*1*/DC_ICL_1800mA|USBHC_ICL_1800mA,                                                    //0x66,
/*2*/SUS_CTRL_BY_REGISTER|BAT_TO_SYS_NORMAL|VFLT_PLUS_200mV|AICL_ENABLE|AIC_TH_4200mV|USB_IN_FIRST|BAT_OV_END_CHARGE,    //0xb6,
/*3*/PRE_CHG_VOLTAGE_THRESHOLD_3_0|FLOAT_VOLTAGE_4_2_0,                                        //0xe3,
/*4*/AUTOMATIC_RECHARGE_ENABLE|CURRENT_TERMINATION_ENABLE|BMD_VIA_THERM_IO|AUTO_RECHARGE_100mV|APSD_ENABLE|NC_APSD_ENABLE|SECONDARY_INPUT_NOT_ACCEPTED_IN_OVLO,//0x3e,
/*5*/STAT_ACTIVE_LOW|STAT_CHARGEING_STATE|STAT_ENABLE|NC_INPUT_HC_SETTING|CC_TIMEOUT_764MIN|PC_TIMEOUT_48MIN,//0x1f,
/*6*/LED_BLINK_DISABLE|EN_PIN_ACTIVE_LOW|USB_HC_CONTROL_BY_PIN|USB_HC_DUAL_STATE|CHARGER_ERROR_NO_IRQ|APSD_DONE_IRQ|DCIN_INPUT_PRE_BIAS_ENABLE,//0x7B
/*7*/0x80|MIN_SYS_3_4_5_V|THERM_MONITOR_VDDCAP|THERM_MONITOR_ENABLE|SOFT_COLD_CC_FV_COMPENSATION|SOFT_HOT_CC_FV_COMPENSATION,//0xaf,THERM Control A
/*8*/INOK_OPERATION|USB_2|VFLT_MINUS_240mV|HARD_TEMP_CHARGE_SUSPEND|PC_TO_FC_THRESHOLD_ENABLE|INOK_ACTIVE_LOW,//0x18,SYSOK and USB3.0 Selection
/*9*/RID_DISABLE_OTG_I2C_CONTROL|OTG_PIN_ACTIVE_HIGH|LOW_BAT_VOLTAGE_3_4_6_V,//0x0e,
/*a*/CCC_700mA|DTRT_130C|OTG_CURRENT_LIMIT_500mA|OTG_BAT_UVLO_THRES_2_7_V,//0x28,OTG, TLIM and THERM Control
/*b*/0x61,//xxxxxxx
/*c*/AICL_COMPLETE_TRIGGER_IRQ,//0x02
/*d*/INOK_TRIGGER_IRQ|LOW_BATTERY_TRIGGER_IRQ,//0x5,
};
static int kc1_chargerdetect_setting_pvt[] = {
/*0*/FCC_2500mA|PCC_150mA|TC_200mA,                                                     //0xed,
/*1*/DC_ICL_1800mA|USBHC_ICL_1800mA,                                                    //0x66,
/*2*/SUS_CTRL_BY_REGISTER|BAT_TO_SYS_NORMAL|VFLT_PLUS_200mV|AICL_ENABLE|AIC_TH_4200mV|USB_IN_FIRST|BAT_OV_END_CHARGE,    //0xb6,
/*3*/PRE_CHG_VOLTAGE_THRESHOLD_3_0|FLOAT_VOLTAGE_4_2_0,                                        //0xe3,
/*4*/AUTOMATIC_RECHARGE_ENABLE|CURRENT_TERMINATION_ENABLE|BMD_VIA_THERM_IO|AUTO_RECHARGE_100mV|APSD_ENABLE|NC_APSD_ENABLE|SECONDARY_INPUT_NOT_ACCEPTED_IN_OVLO,//0x3e,
/*5*/STAT_ACTIVE_LOW|STAT_CHARGEING_STATE|STAT_ENABLE|NC_INPUT_HC_SETTING|CC_TIMEOUT_764MIN|PC_TIMEOUT_48MIN,//0x14,
/*6*/LED_BLINK_DISABLE|EN_PIN_ACTIVE_LOW|USB_HC_CONTROL_BY_PIN|USB_HC_DUAL_STATE|CHARGER_ERROR_NO_IRQ|APSD_DONE_IRQ|DCIN_INPUT_PRE_BIAS_ENABLE,//0x7B
/*7*/0x80|MIN_SYS_3_4_5_V|THERM_MONITOR_VDDCAP|THERM_MONITOR_ENABLE|SOFT_COLD_CC_FV_COMPENSATION|SOFT_HOT_FV_COMPENSATION,//0xaf,THERM Control A
/*8*/INOK_OPERATION|USB_2|VFLT_MINUS_240mV|HARD_TEMP_CHARGE_SUSPEND|PC_TO_FC_THRESHOLD_ENABLE|INOK_ACTIVE_LOW,//0x18,SYSOK and USB3.0 Selection
/*9*/RID_DISABLE_OTG_I2C_CONTROL|OTG_PIN_ACTIVE_HIGH|LOW_BAT_VOLTAGE_3_4_6_V,//0x0e,
/*a*/CCC_700mA|DTRT_130C|OTG_CURRENT_LIMIT_500mA|OTG_BAT_UVLO_THRES_2_7_V,//0x78,OTG, TLIM and THERM Control
/*b*/0xf5,//0xf5
/*c*/AICL_COMPLETE_TRIGGER_IRQ,//0x02
/*d*/INOK_TRIGGER_IRQ|LOW_BATTERY_TRIGGER_IRQ,//0x5,
};
static int kc1_phydetect_setting[] = {
FCC_2500mA|PCC_150mA|TC_150mA,                                                     //0xe0,
DC_ICL_1800mA|USBHC_ICL_1800mA,                                                   //0x66,
SUS_CTRL_BY_REGISTER|BAT_TO_SYS_NORMAL|VFLT_PLUS_200mV|AICL_ENABLE|AIC_TH_4200mV|USB_IN_FIRST|BAT_OV_END_CHARGE,    //0x17,
//SUS_CTRL_BY_REGISTER|BAT_TO_SYS_NORMAL|VFLT_PLUS_200mV|AICL_DISABLE|AIC_TH_4200mV|USB_IN_FIRST|BAT_OV_END_CHARGE,    //0x17,
PRE_CHG_VOLTAGE_THRESHOLD_3_0|FLOAT_VOLTAGE_4_2_0,                                        //0xe3,
//detect by omap
AUTOMATIC_RECHARGE_ENABLE|CURRENT_TERMINATION_ENABLE|BMD_VIA_THERM_IO|AUTO_RECHARGE_100mV|APSD_DISABLE|NC_APSD_DISABLE|SECONDARY_INPUT_NOT_ACCEPTED_IN_OVLO,//0xf0,
STAT_ACTIVE_LOW|STAT_CHARGEING_STATE|STAT_DISABLE|NC_INPUT_HC_SETTING|CC_TIMEOUT_DISABLED|PC_TIMEOUT_DISABLED,//0x1f,
LED_BLINK_DISABLE|CHARGE_EN_I2C_0|USB_HC_CONTROL_BY_REGISTER|USB_HC_TRI_STATE|CHARGER_ERROR_NO_IRQ|APSD_DONE_IRQ|DCIN_INPUT_PRE_BIAS_ENABLE,//0x79
0X80|MIN_SYS_3_4_5_V|THERM_MONITOR_VDDCAP|SOFT_COLD_NO_RESPONSE|SOFT_HOT_NO_RESPONSE,//0xa0,
INOK_OPERATION|USB_2|VFLT_MINUS_60mV|PC_TO_FC_THRESHOLD_ENABLE|HARD_TEMP_CHARGE_SUSPEND|INOK_ACTIVE_LOW,//0x04,
RID_DISABLE_OTG_I2C_CONTROL|OTG_PIN_ACTIVE_HIGH|LOW_BAT_VOLTAGE_3_5_8_V,//0x0e,
CCC_700mA|DTRT_130C|OTG_CURRENT_LIMIT_500mA|OTG_BAT_UVLO_THRES_2_7_V,//0x78,
0x61,
TEMP_OUTSIDE_COLD_HOT_HARD_LIMIITS_TRIGGER_IRQ|TEMP_OUTSIDE_COLD_HOT_SOFT_LIMIITS_TRIGGER_IRQ|USB_OVER_VOLTAGE_TRIGGER_IRQ|USB_UNDER_VOLTAGE_TRIGGER_IRQ|AICL_COMPLETE_TRIGGER_IRQ,
CHARGE_TIMEOUT_TRIGGER_IRQ|TERMINATION_OR_TAPER_CHARGING_TRIGGER_IRQ|FAST_CHARGING_TRIGGER_IRQ|INOK_TRIGGER_IRQ|MISSING_BATTERY_TRIGGER_IRQ| LOW_BATTERY_TRIGGER_IRQ,//0x83,
};

static int aicl_results[]={
    300,500,700,900,1200,1500,1800,2000,2200,2500,2500,2500,2500,2500,2500,2500
};
/* Functions to read and write from SMB347 */
static inline int smb347_i2c_write_u8( u8 val, u8 reg)
{
//    printf("Charger: reg=0x%x value=0x%x \n",reg,val);
    int mbid=0;
    mbid=get_mbid();
    if(mbid>=4)
        select_bus(3,100);
	return i2c_write(SMB347_ADDRESS, reg, 1, &val, 1);
}

static inline int smb347_i2c_read_u8( u8 *val, u8 reg)
{
    int mbid=0;
    mbid=get_mbid();
    if(mbid>=4)
        select_bus(3,100);
	return i2c_read(SMB347_ADDRESS, reg, 1, val, 1);
}
/*
It is a 0.5 sec loop.
work_index	second	function
0		0	voltage,capacity
1		0.5	power_button
2		1	vbus
3		1.5	power_button
4		2	temperature
5		2.5	power_button
6		3	vbus
7		3.5	power_button
8		4	current
9		4.5	power_button
*/
static int low_bat_charge()
{
    int result=0;
    int voltage=0;
    int current=0;
    int temperature=0;
    int vbus=0;
    int ms=0;
    int sec=0;
    int value=0;
    int capacity=0;
    int show_low_bat=0;
    int show_low_bat_ptw=0;
    int power_button=0;
    int work_index=0;
    while(result==0){
        work_index=sec%10;
        //printf("sec=%d work_index=%d\n",sec,work_index);
        switch(work_index){
            case 0:
                voltage=get_bat_voltage();
                capacity=get_bat_capacity();
                if((voltage > LOW_BATTERY_VOLTAGE_LIMIT) && (capacity > LOW_BATTERY_CAPACITY_LIMIT)){
                    result=1;
                    break;
                }
                if( show_low_bat==1){ 
                    show_low_bat=2;
                    turn_off_lcd();
                 
                }
                if((voltage > LOW_LCD_VOLTAGE_LIMIT) && (show_low_bat==0)){
                   show_low_bat=1;
                   initialize_lcd(OTTER_LCD_DISPLAY_LOW_BATT_SCREEN);
                }
                printf("Battery voltage=%d capacity=%d \n",voltage,capacity);
                break;
            case 4:
                temperature=get_bat_temperature();
                //printf("Battery temperature=%d\n",temperature);
                if( (temperature>TEMPERATURE_HOT_LIMIT) || (temperature<TEMPERATURE_COLD_LIMIT)){
                     printf("shutdown due to temperature protect %d\n",temperature);
                     twl6030_shutdown();
                }
                break;
            case 8:
                current=get_bat_current();
                printf("Battery current=%d\n",current);
        	/*
	 	 * Shutdown if there is drain current, and we've displayed the LCD
	 	 * and shut it down after 10 seconds
	 	 */
                if(current<0 && (show_low_bat<1 || show_low_bat>15)){
                    printf("shutdown due to there is discharge %d mA\n",current);
                    twl6030_shutdown();
                }
                break;
            case 2:
            case 6:
                vbus=twl6030_get_vbus_status();
                //printf("vbus=%d\n",vbus);
                if(vbus==0){
                    printf("shutdown due to there is no VBUS\n");
                    twl6030_shutdown();
                }
                break;
            default :
                power_button=twl6030_get_power_button_status(); 
                if( (power_button ==0 )&& (voltage > LOW_LCD_VOLTAGE_LIMIT) && (show_low_bat==2)&& (show_low_bat_ptw==0)){
                    show_low_bat_ptw=1;
                    initialize_lcd(OTTER_LCD_DISPLAY_LOW_BATT_SCREEN);
                }
                if((power_button ==1)&& (show_low_bat_ptw==1)){
                    show_low_bat_ptw=0;
                    turn_off_lcd();           
                }
                break;
        }
        //software time protect
        if(sec > 3600){ //30 min
            result=0;
            printf("shutdown due to the charge time out\n");
            twl6030_shutdown();
            break;
        }
        //delay 0.5s
        sec++;
        mdelay(200);
    }
    //check vbus again
    vbus=twl6030_get_vbus_status();
    if((vbus==0)){
        run_command("setgreenled ff", 0);
        run_command("setamberled 0", 0);
    }
    return result;
}
void check_low_bat()
{
    int power_source=0;
    int input_limit=0;
    int voltage=0;
    int capacity=0;
    int temperature=0;
    int ms=0;
    int sec=0;
    int mbid=0;
    int value=0;

    //turn off the phy
    __raw_writel(0x100, 0x4A0093E0);	  //disable ocp2scp_usb_phy_ick
    __raw_writel(0x30000, 0x4A0093E0);	  //disable ocp2scp_usb_phy_phy_48m
    __raw_writel(0x0, 0x4A008640);	  //disable usb_phy_cm_clk32k
    __raw_writel(1, 0x4A002300);          //power down the usb phy

    mbid=get_mbid();
    if(mbid!=0){
        __raw_writel(0x40000000,0x4a100620);     //disable the detect charger fuction
    }

    temperature=get_bat_temperature();
    voltage=get_bat_voltage();
    capacity=get_bat_capacity();
    printf("Battery capacity =%d voltage=%d temperature=%d\n",capacity,voltage,temperature);
    //Temperature protect 
    if( (temperature>TEMPERATURE_HOT_LIMIT) || (temperature<TEMPERATURE_COLD_LIMIT)){
        printf("shutdown due to temperature protect %d\n",temperature);
        twl6030_shutdown();
    }
    if( (voltage <= LOW_BATTERY_VOLTAGE_LIMIT) || (capacity <= LOW_BATTERY_CAPACITY_LIMIT)){
        //Prepare to display the low battery message. 
        run_command("lbtinit", 0);
        if(twl6030_get_vbus_status()){
            run_command("setgreenled 0", 0);
            run_command("setamberled 5", 0);

            //enable the VUSB 3.3 V
            twl6030_disable_vusb();
            twl6030_init_vusb();
            summit_write_config(1);
            //enable charge time out
            smb347_i2c_read_u8(&value,SUMMIT_SMB347_STAT_TIMERS);
            //enable Pre-charge Timeout 48 min
            CLEAR_BIT(value,1);CLEAR_BIT(value,0);
            //enable complete charge timeout 1527 min
            CLEAR_BIT(value,2);SET_BIT(value,3);
            smb347_i2c_write_u8(value,SUMMIT_SMB347_STAT_TIMERS);  
            summit_write_config(0);

            //Detect the power source
            if(mbid==0){//EVT 1.0 DETECT_BY_PHY
                printf("DETECT_BY_PHY\n");
                power_source=detect_usb();
            }else{//DETECT_BY_CHARGER
                printf("DETECT_BY_CHARGER\n");
                //check is this first boot?
                smb347_i2c_read_u8(&value,SUMMIT_SMB347_STAT_TIMERS);
                if(IS_BIT_CLEAR(value,5)){//FIRST BOOT
                    printf("FIRST BOOT\n");
                    //disable stat 
                    summit_write_config(1);
                    SET_BIT(value,5);
                    smb347_i2c_write_u8(value,SUMMIT_SMB347_STAT_TIMERS);
                    summit_write_config(0);
                    //redo the apsd
                    summit_config_apsd(0);
                    summit_config_apsd(1);
                }
                power_source=summit_detect_usb();
                if(power_source==NO_VBUS)
                        goto LOW_BAT_TURN_OFF;
                if(power_source==2){
                    if(mbid==0){
                        //Need to fixed,this is bug of summit
                        summit_switch_mode(USB5_OR_9_MODE);
                        summit_init(mbid);
                        summit_charge_enable(1);
                    }
                    /* Enter low battery charge loop */
                    low_bat_charge();
                    if(mbid==0)
                        summit_charge_enable(0);
                }else if(power_source==3||power_source==1){
                    if(mbid==0){
                        summit_switch_mode(HC_MODE);
                        summit_init(mbid);
                        summit_charge_enable(1);
                    }
                    if(mbid>=4){  //For DVT
                        input_limit=summit_is_aicl_complete();
                        if(input_limit==NO_VBUS)
                            goto LOW_BAT_TURN_OFF;
                        printf("Power:AICL=%d mA\n",input_limit);
                        /* Enter low battery charge loop */
                        if(input_limit>0 && input_limit<=900)
                           low_bat_charge();
                    }
                }
            }
        }else{
LOW_BAT_TURN_OFF:            
            printf("shutdown due to there is weak battery \n");
            if(voltage > LOW_LCD_VOLTAGE_LIMIT){
                initialize_lcd(OTTER_LCD_DISPLAY_LOW_BATT_SCREEN);
                for (ms=0;ms<1000;ms++)   
                    udelay(2000);//2ms         
                turn_off_lcd();  
            }
            twl6030_shutdown();
        }
    }
}

void summit_read_status_e(){
    int value=0;
    int temp=0;
    smb347_i2c_read_u8(&value,SUMMIT_SMB347_STATUS_REG_E);
    temp=value;
    if(USB15_HC_MODE(temp)==USB1_OR_15_MODE)
        printf("USB1_MODE\n");

    temp=value;
    if(USB15_HC_MODE(temp)==USB5_OR_9_MODE)
        printf("USB5_MODE\n");
    
    temp=value;
    if(USB15_HC_MODE(temp)==HC_MODE)
        printf("HC_MODE\n");

    temp=value;
    if(AICL_STATUS(temp))
        printf("AICL Completed\n");
    else
        printf("AICL NOt Completed\n");
    temp=value;
    printf("AICL_RESULT=%d mA\n",aicl_results[AICL_RESULT(temp)]);
}
void summit_read_status_c(){
    int value=0;
    int temp=0;
    smb347_i2c_read_u8(&value,SUMMIT_SMB347_STATUS_REG_C);
    temp=value;
    if(CHARGEING_CYCLE_STATUS(value))
        printf("At least one charge cycle\n");
    else
        printf("No charge cycle\n");
    temp=value;
    if(BATTERY_VOLTAGE_LEVEL_STATUS(value))
        printf("Bat < 2.1V\n");
    else
        printf("Bat > 2.1V\n");
    temp=value;
    if(HOLD_OFF_STATUS(value))
        printf("Not in hold-off status\n");
    else
        printf("in hold-off status\n");
    temp=value;
    CHARGEING_STATUS(temp);
    if(temp==NO_CHARGING_STATUS)
        printf("NO_CHARGING\n");
    else if(temp==PRE_CHARGING_STATUS)
        printf("PRE_CHARGING\n");
    else if(temp==FAST_CHARGING_STATUS)
        printf("FAST_CHARGING\n");
    else if(temp==TAPER_CHARGING_STATUS)
        printf("TAPER_CHARGING\n");
    temp=value;
    if(CHARGEING_ENABLE_DISABLE_STATUS(temp))
        printf("Charger Enabled\n");
    else
        printf("Charger Disabled\n");
}
void summit_write_config(int enable){
    int command=0;
    smb347_i2c_read_u8(&command,SUMMIT_SMB347_COMMAND_REG_A);
    if(enable){
        COMMAND_ALLOW_WRITE_TO_CONFIG_REGISTER(command);
    }else{
        COMMAND_NOT_ALLOW_WRITE_TO_CONFIG_REGISTER(command);
    }    
    smb347_i2c_write_u8(command,SUMMIT_SMB347_COMMAND_REG_A);
}
void summit_config_apsd(int enable){
    int config=0;
    summit_write_config(1);
    
    smb347_i2c_read_u8(&config,SUMMIT_SMB347_CHARGE_CONTROL);    
    if(enable){
        SET_APSD_ENABLE(config);
    }else{
        SET_APSD_DISABLE(config);
    }
    smb347_i2c_write_u8(config,SUMMIT_SMB347_CHARGE_CONTROL);
   
    summit_write_config(0);
}    
void summit_config_aicl(int enable,int aicl_thres){
    int config=0;
    summit_write_config(1);
    smb347_i2c_read_u8(&config,SUMMIT_SMB347_FUNCTIONS); 
    printf("Reg=0x%x value=0x%x\n",SUMMIT_SMB347_FUNCTIONS,config);   
    if(enable){
        SET_AICL_ENABLE(config);
    }else{
        SET_AICL_DISABLE(config);
    }
    if(aicl_thres==4200){
        SET_AIC_TH_4200mV(config);
    }else if(aicl_thres==4500){
        SET_AIC_TH_4500mV(config);
    }
    smb347_i2c_write_u8(config,SUMMIT_SMB347_FUNCTIONS);
    smb347_i2c_read_u8(&config,SUMMIT_SMB347_FUNCTIONS); 
    printf("Reg=0x%x value=0x%x\n",SUMMIT_SMB347_FUNCTIONS,config); 
    summit_write_config(0);
}
int summit_is_aicl_complete()
{
    u8 value,temp=0;
    int i,vbus,result;
    for(i=0;i<=20;i++){
        smb347_i2c_read_u8(&value,SUMMIT_SMB347_FUNCTIONS);
        if(IS_AICL_DISABLE(value)){
            return -1;
        }
        smb347_i2c_read_u8(&value,SUMMIT_SMB347_STATUS_REG_E);
        temp=value;
        if(AICL_STATUS(temp)){
            temp=value;
            printf("AICL Complete ,result=%d \n",aicl_results[AICL_RESULT(temp)]);
            return aicl_results[AICL_RESULT(value)];
        }
        vbus=twl6030_get_vbus_status();
        if(vbus==0){
            return -2;
        }
    }
    return -1;
}

void summit_switch_mode(int mode)
{
    u8   command=0;
    switch(mode){
        case USB1_OR_15_MODE:
            COMMAND_USB1(command);
            printf("USB1_MODE\n");
        break;
        case USB5_OR_9_MODE:
            COMMAND_USB5(command);
            printf("USB5_MODE\n");
        break;
        case HC_MODE:
            COMMAND_HC_MODE(command);
            printf("HC_MODE\n");
        break;
    }
    printf("command=%d\n",command);
    smb347_i2c_write_u8(command,SUMMIT_SMB347_COMMAND_REG_B);
}

int summit_charge_enable(int enable){
    u8   command=0;
    int bmd=0;
    smb347_i2c_read_u8(&command,SUMMIT_SMB347_COMMAND_REG_A);
    if(enable){
        printf("enable\n");
        COMMAND_CHARGING_ENABLE(command);
    }else{
        printf("disable\n");
        COMMAND_CHARGING_DISABLE(command);
    }
    smb347_i2c_write_u8(command,SUMMIT_SMB347_COMMAND_REG_A);
}
int detect_usb()
{   
    int usb2phy,value=0;
    int i=0;
    usb2phy=__raw_readl(0x4a100620);
    if(usb2phy&0x40000000 ){            //ROM code disable detect charger function
        __raw_writel(0,0x4a100620);     //enable the detect charger fuction
        //printf("    Power: USB   Dedicated charger \n");
        //return 1;
    }
    //power up the phy    
    __raw_writel(~(1), 0x4A002300);
    while(1){
        usb2phy=__raw_readl(0x4a100620);
        value=usb2phy;
        value&=0xE00000;value=value>>21;
        i++;
        if(value!=0)
            break;               
        if(i>=20000000 || i<0)
            break;
    }
    printf("    Power: usb2phy=0x%x i=%d \n",usb2phy,i);
    usb2phy&=0xE00000;
    usb2phy=usb2phy>>21;
    
    if(usb2phy==4 ){
        printf("    PHY: USB   Dedicated charger \n");
        return 1;
    }
    if(usb2phy==5){
        printf("    PHY: USB   HOST charger\n");
        return 1;
    }
    if(usb2phy==6){
        printf("    PHY: USB   PC\n");
        return 2;
    }
    if(usb2phy==1 || usb2phy==3 || usb2phy==7){
        printf("No contact || Unknown error || interrupt\n");
        return 3;
    }
    return 1;
}
int summit_detect_usb()
{   
    int value=0;
    int i=0;
    int usb2phy,vbus=0;
    u8 command=0;
    int mbid=0;
    mbid=get_mbid();
    //Check APSD enable
    smb347_i2c_read_u8(&value,4);
    printf("    Charger: summit_detect_usb \n");
    if((value&0x06)==0){
        value=value|6;
        smb347_i2c_write_u8(value,4);
    }
    for(i=0;i<=20;i++){
        smb347_i2c_read_u8(&value,SUMMIT_SMB347_INTSTAT_REG_D);
        smb347_i2c_read_u8(&value,SUMMIT_SMB347_STATUS_REG_D);
        //printf("    Power: STATUS_D=0x%x\n",value);
        if(value&0x08){
            printf("    Charger: APSD Completed\n");
            value=value&7;
            break;
        }
        value=-1;
        vbus=twl6030_get_vbus_status();
        if(vbus==0){
            return -2;
        }
    }
    if(value==-1){
        printf("    Charger: APSD Not running \n");
    }
    //printf("    Power: i=%d status=0x%x \n",i,value);
    if(value==2 ){
        printf("    Charger: USB   Dedicated charger \n");
        return 1;
    }
    if(value==1){
        printf("    Charger: USB   HOST charger\n");
        if(mbid<4)
          return 2;
        else      //DVT
          return 1;
    }
    if(value==3){
        printf("    Charger: Other Charging Port\n");
        summit_switch_mode(HC_MODE); 
        summit_write_config(1);
        smb347_i2c_read_u8(&command,SUMMIT_SMB347_ENABLE_CONTROL);
        command&=~(0x1<<(4));
        smb347_i2c_write_u8(command,SUMMIT_SMB347_ENABLE_CONTROL);
        summit_write_config(0);
        if(mbid<4)
            return 2;
        else      //DVT
          return 1;
    }
    if(value==4){
        printf("    Charger: USB   PC\n");
        return 2;
    }
    if(value==6){
        printf("    Charger: TBD\n");
        if(mbid<4)
            return 2;
        else      //DVT
          return 3;
    }

    return value;
}

void summit_read_setting()
{
    u8 command=0;
    int index=0;
    int value=0;
    for(index=0;index<=0x0d;index++){
        smb347_i2c_read_u8(&value,index);
        printf("index=0x%x \n value=0x%x\n",index,value);
        udelay(5000); 
    }
}
void summit_init(int mbid)
{
    u8 command=0;
    int index=0;
    int value=0;
    summit_write_config(1);
    for(index=0;index<=0x0d;index++){
        smb347_i2c_read_u8(&value,index);
        udelay(1000); 
        if(mbid==0){
            if(value!=kc1_phydetect_setting[index]){
                smb347_i2c_write_u8(kc1_phydetect_setting[index],index);
	        }
        }else{
            if(mbid>=5){//For PVT
                if(value!=kc1_chargerdetect_setting_pvt[index]){
                    if(index!=0x05  && index!=0x3)
                        smb347_i2c_write_u8(kc1_chargerdetect_setting_pvt[index],index);
	        }                  
            }else{
                if(value!=kc1_chargerdetect_setting[index]){
                    if(index!=0x05  && index!=0x3)
                        smb347_i2c_write_u8(kc1_chargerdetect_setting[index],index);              
	        }
            }
        }
    }
    summit_write_config(0);
}
#endif
