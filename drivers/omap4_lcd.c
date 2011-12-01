// Seven modifies source codes from WinCE EBoot

#include <common.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <command.h>
#include <i2c.h>
#include <otter.h>

#define INREG32(x)          readl((unsigned int *)(x))
#define OUTREG32(x, y)      __raw_writel((unsigned int )(y),(unsigned int *)(x))
#define SETREG32(x, y)      OUTREG32(x, INREG32(x)|(y))
#define CLRREG32(x, y)      OUTREG32(x, INREG32(x)&~(y))

//Physical address of registers
#define DSS_SYSCONFIG			0x48050010
#define DSS_SYSSTATUS			0x48050014
#define DSS_CONTROL			0x48050040

#define DISPC_SYSCONFIG			0x48050410
#define DISPC_SYSSTATUS			0x48050414
#define DISPC_IRQENABLE			0x4805041C
#define DISPC_CONTROL			0x48050440
#define DISPC_CONFIG			0x48050444
#define DISPC_DEFAULT_COLOR0		0x4805044C
#define DISPC_TRANS_COLOR0		0x48050454
#define DISPC_TIMING_H			0x48050464
#define DISPC_TIMING_V			0x48050468
#define DISPC_POL_FREQ			0x4805046C
#define DISPC_DIVISOR			0x48050470
#define DISPC_SIZE_LCD			0x4805047C
#define DISPC_GFX_BA0			0x48050480
#define DISPC_GFX_POSITION		0x48050488
#define DISPC_GFX_SIZE			0x4805048C
#define DISPC_GFX_ATTRIBUTES		0x480504A0
#define DISPC_GFX_FIFO_THRESHOLD	0x480504A4
#define DISPC_GFX_ROW_INC		0x480504AC
#define DISPC_GFX_PIXEL_INC		0x480504B0
#define DISPC_GFX_WINDOW_SKIP		0x480504B4

#define CONTROL_DSS_DPLL_SPREADING		0x48002450 //qvx_spreading
#define CONTROL_CORE_DPLL_SPREADING		0x48002454
#define CONTROL_PER_DPLL_SPREADING		0x48002458
#define CONTROL_USBHOST_DPLL_SPREADING		0x4800245C

#define CONTROL_DSS_DPLL_SPREADING_FREQ		0x48002544 //qvx_spreading
#define CONTROL_CORE_DPLL_SPREADING_FREQ	0x48002548
#define CONTROL_PER_DPLL_SPREADING_FREQ		0x4800254C
#define CONTROL_USBHOST_DPLL_SPREADING_FREQ	0x48002550

#define DISPC_GFX_FIFO_THRESHOLD_R(low,high)	((low << 0) | (high << 16))
#define DISPC_CONTROL_GOLCD                     (1 << 5)
#define DISPC_CONTROL_LCDENABLE                 (1 << 0)
#define DISPC_SYSCONFIG_AUTOIDLE                (1 << 0)
#define DISPC_SYSCONFIG_SOFTRESET               (1 << 1)
#define DISPC_SYSSTATUS_RESETDONE               (1 << 0)

#define DSS_CONTROL_DISPC_CLK_SWITCH_DSS1_ALWON (0 << 0)
#define DSS_CONTROL_DSI_CLK_SWITCH_DSS1_ALWON   (0 << 1)

#define SYSCONFIG_NOIDLE                    	(1 << 3)
#define SYSCONFIG_NOSTANDBY                 	(1 << 12)

#define DISPC_GFX_ATTR_GFXENABLE                (1 << 0)
#define DISPC_GFX_ATTR_GFXFORMAT(fmt)           ((fmt) << 1)
#define DISPC_PIXELFORMAT_RGB24                 (0x9)
#define LOGO_GFX_ATTRIBUTES         		(DISPC_GFX_ATTR_GFXENABLE | DISPC_GFX_ATTR_GFXFORMAT(DISPC_PIXELFORMAT_RGB24) )// RGB24 packed, enabled

#define CM_CLKEN_DSS_MASK                   	(0x3)
#define CM_CLKEN_DSS1                       	(1 << 0)
#define CM_CLKEN_DSS2                       	(1 << 1)
#define CM_CLKEN_TV                         	(1 << 2)
#define CM_CLKEN_DSS                        	(1 << 0)

#define DISPC_CONTROL_GPOUT0                    (1 << 15)
#define DISPC_CONTROL_GPOUT1                    (1 << 16)
#define DISPC_CONTROL_TFTDATALINES_18           (2 << 8)
#define DISPC_CONTROL_TFTDATALINES_24           (3 << 8)
#define DISPC_CONTROL_STNTFT                    (1 << 3)
#define DISPC_CONFIG_FUNCGATED                  (1 << 9)
#define DISPC_CONFIG_PALETTEGAMMATABLE          (1 << 3)
#define DISPC_POL_FREQ_ONOFF                    (1 << 17)

#define DISPC_TIMING(sw,fp,bp)			((sw << 0) | (fp << 8) | (bp << 20))
#define DISPC_DIVISOR_R(pcd,lcd)		((pcd << 0) | (lcd << 16))
#define DISPC_SIZE_LCD_R(lpp,ppl)		(((ppl - 1) << 0) | ((lpp - 1) << 16))

#define DISPC_CONFIG_LOADMODE(mode)             ((mode) << 1)
#define BSP_LCD_CONFIG				(DISPC_CONFIG_FUNCGATED | DISPC_CONFIG_LOADMODE(2) ) //Color phase rotation
#define BSP_GFX_POS(x,y)			(((x) << 0) | ((y) << 16) )
#define BSP_GFX_SIZE(x,y)			(((x - 1) << 0) | ((y - 1) << 16) )

#define LCD_WIDTH			1024
#define LCD_HEIGHT			600
#define LCD_HSW          		10
#define LCD_HFP          		160 
#define LCD_HBP          		160
#define LCD_VSW          		2
#define LCD_VFP          		10
#define LCD_VBP          		23

// Select DSS1 clock divider based on display mode
#define BSP_DSS_CLKSEL_DSS1		((1 << 12) | (2 << 0))	// DSS1 = DPLL4/A/2 = 43200Mhz
#define BSP_LCD_PIXCLKDIV		0x4		// PCD pixel clock = 50.8Mhz

#define BSP_CM_CLKSEL_DSS		(BSP_DSS_CLKSEL_DSS1)
// DSS1 = DPLL4/9 = 96MHz, divide by 4 = 24MHz pixel clock
#define LCD_LOGCLKDIV			1
// Minimum value for LCD_PIXCLKDIV is 2
#define LCD_PIXCLKDIV			BSP_LCD_PIXCLKDIV

#define BYTES_PER_PIXEL			3
#define DELAY_COUNT			100

extern int do_fat_fsload(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
extern int do_mmc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

unsigned int g_LogoX,g_LogoY,g_LogoW,g_LogoH;

/*******************************************************
 * Routine: delay
 * Description: spinning delay to use before udelay works
 ******************************************************/
static inline void delay(unsigned long loops){
	__asm__ volatile ("1:\n" "subs %0, %1, #1\n"
			  "bne 1b":"=r" (loops):"0"(loops));
}

void LcdPdd_LCD_Initialize(void){
	unsigned int val = 0;

	val = INREG32(0x4a009120);//CM_DSS_DSS_CLKCTRL
	val = val & ~(0x0502);
	// Setup the DSS1 clock divider - disable DSS1 clock, change divider, enable DSS clock
	OUTREG32(0x4a009120, val);//CM_DSS_DSS_CLKCTRL
	udelay(10000);

	SETREG32(0x4a00815c, 8<<0);//CM_DIV_M5_DPLL_PER
	udelay(10000);
	//printf("CM_CLKSEL_DSS= %x\n",INREG32(CM_CLKSEL_DSS));

	val = INREG32(0x4a009120) ;//CM_DSS_DSS_CLKCTRL
	val = val | 0x0502;
	OUTREG32(0x4a009120, 0x00000502);
	udelay(10000);

	// LCD control xxxx xxxx xxxx 0000 0000 0010 0000 1001
	OUTREG32(0x48041238, DISPC_CONTROL_TFTDATALINES_24 | DISPC_CONTROL_STNTFT);//DISPC_CONTROL2
				
	// Default Color
	OUTREG32(0x480413ac, 0x00000000);//DISPC_DEFAULT_COLOR2

	// LCD control xxxx xxxx xxxx 0000 0000 0010 0000 1001
	SETREG32(0x48041238, 0<<12);//DISPC_CONTROL2 OVERLAYOPTIMIZATION

	// Default Transparency Color
	OUTREG32(0x480413b0, 0);//DISPC_TRANS_COLOR2

	SETREG32(0x48041044, 0x4<<0);//DISPC_CONFIG1 LOAD_MODE: Frame data only loaded every frame
////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Signal configuration
//	OUTREG32(0x48041408,DISPC_POL_FREQ_ONOFF);//DISPC_POL_FREQ2
	OUTREG32(0x48041408,0x00003000);//DISPC_POL_FREQ2
	udelay(10000);

	// Configure the divisor
	OUTREG32(0x4804140c,DISPC_DIVISOR_R(LCD_PIXCLKDIV,LCD_LOGCLKDIV));//DISPC_DIVISOR2 (PCD 4,LCD 1)
	// Configure the panel size
	OUTREG32(0x480413cc,DISPC_SIZE_LCD_R(LCD_HEIGHT,LCD_WIDTH));//DISPC_SIZE_LCD2 (1024,600)
	// Timing logic for HSYNC signal
	OUTREG32(0x48041400,DISPC_TIMING(LCD_HSW-1,LCD_HFP-1,LCD_HBP-1));//DISPC_TIMING_H2
	// Timing logic for VSYNC signal
	OUTREG32(0x48041404,DISPC_TIMING(LCD_VSW-1,LCD_VFP,LCD_VBP));//DISPC_TIMING_V2
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

void LcdPdd_SetPowerLevel(void){
	unsigned char val;

	// LCD control xxxx xxxx xxxx 0000 0000 0010 0000 1001
	OUTREG32(0x48041238, DISPC_CONTROL_TFTDATALINES_24 | DISPC_CONTROL_STNTFT | 1<<0);//DISPC_CONTROL2

	// Apply display configuration
	SETREG32(0x48041238, DISPC_CONTROL_GOLCD);//DISPC_CONTROL2
	// Start scanning
	SETREG32(0x48041238, DISPC_CONTROL_LCDENABLE );//DISPC_CONTROL2        
	// Add delay to prevent blinking screen.
	udelay(10000); 
}

//  Function:  reset_display_controller
//  This function resets the display subsystem
void reset_display_controller(void){
	unsigned int reg_val,timeout,fclk, iclk;
	unsigned short count;

	// Enable all display clocks
	fclk = INREG32(0x4a009120);//CM_DSS_DSS_CLKCTRL
//	iclk = INREG32(CM_ICLKEN_DSS);

//	SETREG32(0x4a009120, 0x0502);//enable dss fclk
//	OUTREG32(0x4a009120, 0x00000502);//enable dss fclk
//	SETREG32(CM_ICLKEN_DSS, CM_CLKEN_DSS);

	// Reset the display controller
	OUTREG32(0x48041010, 1<<1);//DSI_SYSCONFIG

	// Wait until reset completes or timeout occurs
	timeout=10000;
	
	while(!((reg_val=INREG32(0x48041014)) & 1<<0) && (timeout > 0)){ //sys state
		for(count=0; count<DELAY_COUNT; ++count);		
			timeout--;
	}

	if(!(reg_val & 1<<0)){
		//puts("reset_display_controller: DSS reset timeout.\n");
	}

	reg_val=INREG32(0x48041010);//DSI_SYSCONFIG
	reg_val &=~(1<<1);
	OUTREG32(0x48041010,reg_val);

	// Restore old clock settings
	OUTREG32(0x4a009120, fclk);//CM_DSS_DSS_CLKCTRL
	//OUTREG32(CM_ICLKEN_DSS, iclk);
}
     
//  Function:  enable_lcd_power
//  This function enables the power for the LCD controller
void enable_lcd_power( void ){
	OUTREG32(0x4a009120, (0x00000502));//CM_DSS_DSS_CLKCTRL
}

//  Function:  configure_dss
//  This function configures the display sub-system
void configure_dss( unsigned int framebuffer ){

	// Configure the clock source
	OUTREG32(0x48045040, 1<<8 | 0<<9 | 0<<10 | 0<<12 | 0<<17);//DSI_CTRL
	udelay(10000);

	// Configure interconnect parameters

//	OUTREG32(0x48041010, 1<<0 | 1<<3 | 1<<12 );
	OUTREG32(0x48041010, 0x00002015);
	udelay(10000);

	// Disable any interrupts
	OUTREG32(0x4804501c,0);//DSI_IRQENABLE
	udelay(10000);

	// Over-ride default LCD config
	//OUTREG32(DISPC_CONFIG,BSP_LCD_CONFIG);
	//OUTREG32(0x48041620, );//DISPC_CONFIG2 in omap4

	// Configure graphic window
	OUTREG32(0x48041080,framebuffer);//DISPC_GFX_BA_0
	// Configure the position of graphic window
	OUTREG32(0x48041088,BSP_GFX_POS(g_LogoX,g_LogoY));//DISPC_GFX_POSITION
	// Configure the size of graphic window
	OUTREG32(0x4804108c,BSP_GFX_SIZE(g_LogoW,g_LogoH));//DISPC_GFX_SIZE

	// GW Enabled, RGB24 packed, little Endian
    //OUTREG32(0x480410a0,LOGO_GFX_ATTRIBUTES);//DISPC_GFX_ATTRIBUTES
//	OUTREG32(0x480410a0,1<<0 | 9<<1);//DISPC_GFX_ATTRIBUTES no crash
//	OUTREG32(0x480410a0,1<<30 | 0x3<<26 | 1<<25 | 1<<14 | 1<<0 | 9<<1);//DISPC_GFX_ATTRIBUTES
//	OUTREG32(0x480410a0,1<<30 | 0x3<<26 | 1<<25 | 1<<7 | 1<<0 | 9<<1);//DISPC_GFX_ATTRIBUTES
	OUTREG32(0x480410a0,1<<30);
	udelay(10000); 
	SETREG32(0x480410a0,0x3<<26);
	udelay(10000); 
	SETREG32(0x480410a0,1<<25);
	udelay(10000); 
	SETREG32(0x480410a0,1<<14);
	udelay(10000); 
	SETREG32(0x480410a0,1<<7);
	udelay(10000); 
	SETREG32(0x480410a0,1<<0 | 6<<1);//DISPC_GFX_ATTRIBUTES
	udelay(10000); 
      //0000 0000 0000 0000 0000 0000 0001 0011
	//OUTREG32(0x480410a0,0x42000099);//DISPC_GFX_ATTRIBUTES

	OUTREG32(0x480410a4,DISPC_GFX_FIFO_THRESHOLD_R(192,252));//DISPC_GFX_FIFO_THRESHOLD
//	OUTREG32(0x480410a4,0x04ff0480);//DISPC_GFX_FIFO_THRESHOLD

	//!!! should be modify??
	OUTREG32(0x480410ac,1);//DISPC_GFX_ROW_INC
	OUTREG32(0x480410b0,1);//DISPC_GFX_PIXEL_INC
	OUTREG32(0x480410b4,0);//DISPC_GFX_WINDOW_SKIP
	// Configure the LCD
	LcdPdd_LCD_Initialize();

}

//  Function:  display_lcd_image
//  This function displays the image in the frame buffer on the LCD
void display_lcd_image(void){
	unsigned int  count, ctrl, timeout = 1000;

	// Apply display configuration
	SETREG32(0x48041238,DISPC_CONTROL_GOLCD);//DISPC_CONTROL2

	// wait for configuration to take effect
	do{
		for(count=0;count<DELAY_COUNT;++count);

		ctrl=INREG32(0x48041238);//DISPC_CONTROL2
		timeout-- ;
	}while((ctrl & DISPC_CONTROL_GOLCD) && (timeout > 0));

	// Power up and start scanning
	LcdPdd_SetPowerLevel();	
}

unsigned int framebuffer=(unsigned int *)0x82000000; //Data shown on display.

//  Function:  lcd_config
//  This function configures the LCD
void lcd_config(void){    

	reset_display_controller();

	// Enable LCD clocks
	enable_lcd_power();

	// Configure the DSS registers
	configure_dss(framebuffer);

	// Display data on LCD
	display_lcd_image();
}

void show_black_data(void){ //kc1 black data

	u_int16_t *target_addr = (u_int16_t *)framebuffer;
    unsigned int x, y;
    //printf("show_black_data!!\n");
    //  Fill screen with black data as LG pannel spec.
    g_LogoX = 0;
    g_LogoY = 0;
    g_LogoW = LCD_WIDTH;
    g_LogoH = LCD_HEIGHT;
    for (y= 0; y < LCD_HEIGHT; y++){
        for( x = 0; x < LCD_WIDTH; x++ ){
            *target_addr++ = 0x0000;
        }
    }
}

void show_white_data(void){ //kc1 black data

	u_int16_t *target_addr = (u_int16_t *)framebuffer;
    unsigned int x, y;
    //printf("show_white_data!!\n");
    //  Fill screen with white data as LG pannel spec.
    g_LogoX = 0;
    g_LogoY = 0;
    g_LogoW = LCD_WIDTH;
    g_LogoH = LCD_HEIGHT;
    for (y= 0; y < LCD_HEIGHT; y++){
        for( x = 0; x < LCD_WIDTH; x++ ){
            *target_addr++ = 0xffff;
        }
    }
}

/**********************************************************
 * Routine: turn_off_lcd
 * Description: Turns off the LCD and backlight
 *
 **********************************************************/
void turn_off_lcd()
{
	char *pwm_on[2]  = {"pwm", "00"};
	/* Turn on backlight */
	set_omap_pwm(NULL, 0, 2, pwm_on);
	/* Disable GPIO_37 (OMAP_RGB_SHTDOWN, switch to LVDS mode) */
	__raw_writew(0x1B,0x4a10005A);
	__raw_writew(__raw_readw(0x48055134) & 0xFFDF, 0x48055134);//gpio_OE gpio37
	__raw_writew(__raw_readw(0x4805513C) & ~(0x0020), 0x4805513C);//gpio_dataout gpio37
	udelay(2000);
	/* Disable GPIO_47 (OMAP_3V_ENABLE, 3.3 V rail) */
	__raw_writel(0x001B0007,0x4a10006C);
	__raw_writew(__raw_readw(0x48055134) & 0x7FFF, 0x48055134);//gpio_OE gpio47
	__raw_writew(__raw_readw(0x4805513C) & ~( 0x8000), 0x4805513C);//gpio_dataout gpio47
	udelay(10000);
	/* Enable GPIO_47 (OMAP_3V_ENABLE, 3.3 V rail) */
	__raw_writel(0x001B0007,0x4a10006C);
	__raw_writew(__raw_readw(0x48055134) & 0x7FFF, 0x48055134);//gpio_OE gpio47
	__raw_writew(__raw_readw(0x4805513C) & ~( 0x8000), 0x4805513C);//gpio_dataout gpio47

	/* Enable GPIO_45 (LCD_PWR_ON) */
	__raw_writew(0x1B,0x4a10006A);
	__raw_writew(__raw_readw(0x48055134) & 0xDFFF, 0x48055134);//gpio_OE gpio45
	__raw_writew(__raw_readw(0x4805513C) & ~( 0x2000), 0x4805513C);//gpio_dataout gpio45
	//gpio 119 ADO_SPK_ENABLE
	//gpio_119 gpio_120 pull high
	__raw_writew(__raw_readw(0x48059134) & 0xFF7F, 0x48059134);
	__raw_writew(__raw_readw(0x4805913C) & ~( 0x0080), 0x4805913C);
	__raw_writew(__raw_readw(0x48059134) & 0xFEFF, 0x48059134);
	__raw_writew(__raw_readw(0x4805913C) & ~( 0x0100), 0x4805913C);

}
/**********************************************************
 * Routine: initialize_lcd
 * Description: Initializes the LCD and displays the
 *              splash logo
 **********************************************************/
void initialize_lcd(int show_what)
{
	char *pwm_on[2]  = {"pwm", "7F"};
	//gpio 119 ADO_SPK_ENABLE
	//
	//__raw_writew(0x1B,0x4a100110);
	//gpio_119 gpio_120 pull high
	__raw_writel(0x001B001B,0x4a100110);
	__raw_writew(__raw_readw(0x48059134) & 0xFEFF, 0x48059134);
	__raw_writew(__raw_readw(0x4805913C) | 0x0100, 0x4805913C);

	__raw_writew(__raw_readw(0x48059134) & 0xFF7F, 0x48059134);
	__raw_writew(__raw_readw(0x4805913C) | 0x0080, 0x4805913C);

	spi_init();

	/* Enable GPIO_45 (LCD_PWR_ON) */
	__raw_writew(0x1B,0x4a10006A);
	__raw_writew(__raw_readw(0x48055134) & 0xDFFF, 0x48055134);//gpio_OE gpio45
	__raw_writew(__raw_readw(0x4805513C) | 0x2000, 0x4805513C);//gpio_dataout gpio45

	/* Enable GPIO_47 (OMAP_3V_ENABLE, 3.3 V rail) */
	__raw_writel(0x001B0007,0x4a10006C);
	__raw_writew(__raw_readw(0x48055134) & 0x7FFF, 0x48055134);//gpio_OE gpio47
	__raw_writew(__raw_readw(0x4805513C) | 0x8000, 0x4805513C);//gpio_dataout gpio47

	udelay(10000);
	//kc1 lcd initialize
	spi_command();

	udelay(2000);
	/* Enable GPIO_37 (OMAP_RGB_SHTDOWN, switch to LVDS mode) */
	__raw_writew(0x1B,0x4a10005A);
	__raw_writew(__raw_readw(0x48055134) & 0xFFDF, 0x48055134);//gpio_OE gpio37
	__raw_writew(__raw_readw(0x4805513C) | 0x0020, 0x4805513C);//gpio_dataout gpio37

	show_black_data();
        if(show_what==OTTER_LCD_DISPLAY_LOW_BATT_SCREEN)
            show_lowbattery();
        else
	    show_splash();
	lcd_config();

	/* Turn on backlight */
	set_omap_pwm(NULL, 0, 2, pwm_on);
}
extern char const _binary_lowbattery_gz_start[];
extern char const _binary_lowbattery_gz_end[];

void lbt_datainit(void)
{
    unsigned char *src_addr = (unsigned char *)0x81000000;
    unsigned long src_len = ~0UL, dst_len = 1024*600*2;
    if (gunzip(src_addr, dst_len, (void *) _binary_lowbattery_gz_start, &src_len) != 0)
		return 1;
}
void show_lowbattery(void)
{
	u_int16_t *target_addr = (u_int16_t *)framebuffer;
    unsigned long dst_len = 1024*600*2;
    unsigned char *src_addr = (unsigned char *)0x81000000;
    memcpy(target_addr ,src_addr , dst_len);

	return;
}

extern char const _binary_initlogo_rle_start[];
extern char const _binary_initlogo_rle_end[];

void show_splash(void)
{
	u_int16_t *target_addr = (u_int16_t *)framebuffer;
	u_int16_t *start = (u_int16_t *)_binary_initlogo_rle_start;
	u_int16_t *end = (u_int16_t *)_binary_initlogo_rle_end;

	/* Convert the RLE data into RGB565 */
	for (; start != end; start += 2) {
		u_int16_t count = start[0];

		while (count--) {
			*target_addr++ = start[1];
		}
	}

	/* Compute position and size of logo */
	g_LogoX = 0;
	g_LogoY = 0;
	g_LogoW = LCD_WIDTH;
	g_LogoH = LCD_HEIGHT;

	/*
        CM_CLKSEL2_PLL (0x48004D44)
        (b19~b8)PERIPH_DPLL_MULT = 432 (M)
        (b6~b0)PERIPH_DPLL_DIV = 12 (N)

        CM_CLKSEL_DSS (0x48004E40) 
        (b5~b0)CLKSEL_DSS1 = 5 (M4)

	Fin = 26MHz
        Fout = fc = 26(MHz)*M/(N+1)/M4 = 172.8MHz (PER_PLL)
	Fm (max) < Fref / 70 = 28.57KHz
	Set Fm = 28KHz
	Deviation = (fm/fc) x 10^(PPR/10) = (28k/172.8M) x 10^1 = 0.16%
	Δf = 0.28MHz
	ΔM = M (Δf/fc) = 432 x (1.6/1000) = 0.7
	0.7 x 2^18 = 183500.8 ~ 183501
	Fref/(4 x fm) = 2M/(4x0.028) = 17.857 = 18 x 2^0
	
	R_PER_DELTA_M_INT = 0
	R_PER_DELTA_M_FRACT = 0x2CCCD
	R_PER_MOD_FREQ_EXP = 0x12
	R_PER_MOD_FREQ_MANT = 0
	*/
        //printf("Last 3 \n");
	//SETREG32(0x4a008168, 0x00000000); //CM_SSC_DELTAMSTEP_DPLL_PER
        //printf("Last 2 \n");
	//SETREG32(0x4a00816c, 0x00000000 ); //CM_SSC_MODFREQDIV_DPLL_PER
        //printf("Last 1 \n");
	//SETREG32(0x4a008140, 1<<12);//CM_CLKMODE_DPLL_PER
        //printf("Last 0 \n");
	/* 	f_inp=38.4MHz,M=20,N=0,M5=10,LCD=1,PCD=3
		DISPC2_PCLK=38.4*20/(0+1)*2/10/1/3=51.2MHz
		f_c=f_inp*M*2/((N+1)*M5)=38.4M*20*2/((0+1)*10)
	*/
	/* CM_DIV_M5_DPLL_PER Set bit8 = 1, force HSDIVIDER_CLKOUT2 clock enabled*/
	__raw_writew(__raw_readw(0x4A00815C) | 0x100, 0x4A00815C);
	/* CM_SSC_DELTAMSTEP_DPLL_PER */
	__raw_writew(0XCC , 0x4A008168);
	/* CM_SSC_MODFREQDIV_DPLL_PER */
	__raw_writew(0X264 , 0x4A00816C);
	/* CM_CLKMODE_DPLL_PER Set bit12 = 1, force DPLL_SSC_EN enabled*/
	__raw_writew(__raw_readw(0x4A008140) | 0x1000 , 0x4A008140);

	return;
}

void showlogo(void){ //qvx_logo

	unsigned char * target_addr;
	unsigned char * load_addr;
	unsigned short signature = 0;
	unsigned int x, y, offset,length,logoW,logoH ;
	unsigned int bpp,row,col,boot_device; //bpp -> bits per pixel
	char * tmp[6];
	target_addr=(unsigned char *)framebuffer; //Pointer to calculate data.
	load_addr=(unsigned char *)0x81000000; //Load logo.bmp to this address

//printf("showlogo!!\n");
//	boot_device = INREG32(0x480029C0) & 0xff;
#if 0 //qvx_logo Move read process to X-loader
//	if(boot_device == 0x05){ //Boot from EMMC
		tmp[0]="mmcinit";
		tmp[1]="1";
		do_mmc(NULL, 0, 2, &tmp[0]); //mmcinit 1

		tmp[0]="mmc";
		tmp[1]="1";
		tmp[2]="read";
		tmp[3]="0x3EC1";   
		tmp[4]="0x91000000";   
		tmp[5]="0x1D0000"; //1024*600*3(data)+54(header) < 2MB
		printf("Load logo from EMMC sector %s to address=%s \n",tmp[3],tmp[4]);
		do_mmc(NULL, 0, 6, &tmp[0]); //mmc 1 read 0x3EC1 0x91000000 0x1D0000;
//	}
#endif		
#if 0
	else if(boot_device == 0x06){ //Boot from SDcard
		puts("Load logo from SDcard\n");	
		tmp[0]="mmcinit";
		tmp[1]="0";
		do_mmc(NULL, 0, 2, &tmp[0]); //mmcinit 0

		tmp[0]="fatload";
		tmp[1]="mmc";
		tmp[2]="0";
		tmp[3]="0x91000000";
		//tmp[4]="Logo_1024x600.bmp";
		tmp[4]="Logo.bmp";
		do_fat_fsload(NULL, 0, 5, &tmp[0]); //fatload mmc 0 0x91000000 Logo_1024x600.bmp	
	}
	else{
		puts("Boot from unknown device ... Exit\n");
		return ;
	}

	memcpy(&signature,load_addr, sizeof(signature));

	if( signature != 0x4D42 ){
		puts("No logo found.\n");
		goto NoLogo;
	}
	
	//Get Width/Height/bpp from header
	memcpy( &logoW,load_addr+0x12, sizeof(logoW));
	memcpy( &logoH,load_addr+0x16, sizeof(logoH));
	memcpy( &bpp,load_addr+0x1C, sizeof(bpp)); 

	printf("Logo found. %d * %d * %d \n",logoW,logoH,bpp);

	if ( (logoW>LCD_WIDTH) || (logoH>LCD_HEIGHT) ){
		puts("Logo W/H is out of supported resolution.\n");
		signature = 0;
		goto NoLogo;
	}

	bpp = (bpp & 0xFFFF)>>3;
	length = logoW * logoH * bpp;
	
	row = logoW  * bpp;
	col = logoH;

	if( bpp != BYTES_PER_PIXEL)
		printf("Warning! bpp (%d) != BYTES_PER_PIXEL (%d). \n",bpp,BYTES_PER_PIXEL);

	//Check for 'BM' signature
	if( signature == 0x4D42 )  {
		//Move to data 
		memcpy( &offset,load_addr+0xA, sizeof(offset));

		//Read data with given offset
		memcpy( target_addr,load_addr+offset, length);

		//Copy pix data from left-bottom to right-top, so that we don't have to rotate Logo.bmp .
		offset = load_addr + offset ;

		while(col){
				memcpy( (unsigned char *)target_addr+((logoH - col)*row) ,(unsigned char *)(offset + ((col-1)*row)), row);
		        col--;
		}
	}
#if 0
	else{
		puts("Logo format is not supported.\n");
		goto NoLogo;	
	}

	// Compute position and size of logo. 
	g_LogoX = (LCD_WIDTH > logoW) ? ((LCD_WIDTH - logoW)/2) : 0;
	g_LogoY = (LCD_HEIGHT > logoH) ? ((LCD_HEIGHT - logoH)/2) : 0;                
	g_LogoW = logoW;
	g_LogoH = logoH;
#endif
#endif
NoLogo:

    //  If bitmap signature is valid, display the logo, otherwise fill screen with pattern
    if( 1 ){

        //  Adjust color bars to LCD size
        g_LogoX = 0;
        g_LogoY = 0;
        g_LogoW = LCD_WIDTH;
        g_LogoH = LCD_HEIGHT;

        //printf(" W=%d,H=%d, Lx=%d,y=%d, FB=0x%x \n",g_LogoW,g_LogoH,g_LogoX,g_LogoY,framebuffer);
        
        for (y= 0; y < LCD_HEIGHT; y++){
            for( x = 0; x < LCD_WIDTH; x++ ){
#if 0
                //Same color
                *target_addr++ = 0xFF;    // Blue
                *target_addr++ = 0x00;    // Green
                *target_addr++ = 0x00;    // Red
                //*target_addr++ = 0x00;    // None
#else
                if( y < LCD_HEIGHT/2 ){
                    if( x < LCD_WIDTH/2 ){
                        *target_addr++ = 0xFF;    // Blue
                        *target_addr++ = 0x00;    // Green
                        *target_addr++ = 0x00;    // Red
                        //*target_addr++ = 0x00;    // None
                    }
                    else{
                        *target_addr++ = 0x00;    // Blue
                        *target_addr++ = 0xFF;    // Green
                        *target_addr++ = 0x00;    // Red
                        //*target_addr++ = 0x00;    // None
                    }
                }
                else{
                    if( x < LCD_WIDTH/2 ){
                        *target_addr++ = 0x00;    // Blue
                        *target_addr++ = 0x00;    // Green
                        *target_addr++ = 0xFF;    // Red
                        //*target_addr++ = 0x00;    // None
                    }
                    else if( x < (LCD_WIDTH*3)/4 ){
                        *target_addr++ = 0xFF;    // Blue
                        *target_addr++ = 0xFF;    // Green
                        *target_addr++ = 0xFF;    // Red
                        //*target_addr++ = 0x00;    // None
                    }
                    else{
                        *target_addr++ = 0xFF;    // Blue
                        *target_addr++ = 0xFF;    // Green
                        *target_addr++ = 0xFF;    // Red
                        //*target_addr++ = 0x00;    // None
                    }
                }
#endif                
            }
        }
    }
        //printf("Before lcd config \n");

    //  Fire up the LCD
    //lcd_config();
        //printf("After lcd config \n");
#if 1 //qvx_spreading
	/*
        CM_CLKSEL2_PLL (0x48004D44)
        (b19~b8)PERIPH_DPLL_MULT = 432 (M)
        (b6~b0)PERIPH_DPLL_DIV = 12 (N)

        CM_CLKSEL_DSS (0x48004E40) 
        (b5~b0)CLKSEL_DSS1 = 5 (M4)

	Fin = 26MHz
        Fout = fc = 26(MHz)*M/(N+1)/M4 = 172.8MHz (PER_PLL)
	Fm (max) < Fref / 70 = 28.57KHz
	Set Fm = 28KHz
	Deviation = (fm/fc) x 10^(PPR/10) = (28k/172.8M) x 10^1 = 0.16%
	Δf = 0.28MHz
	ΔM = M (Δf/fc) = 432 x (1.6/1000) = 0.7
	0.7 x 2^18 = 183500.8 ~ 183501
	Fref/(4 x fm) = 2M/(4x0.028) = 17.857 = 18 x 2^0
	
	R_PER_DELTA_M_INT = 0
	R_PER_DELTA_M_FRACT = 0x2CCCD
	R_PER_MOD_FREQ_EXP = 0x12
	R_PER_MOD_FREQ_MANT = 0
	*/
        //printf("Last 3 \n");
	//SETREG32(0x4a008168, 0x00000000); //CM_SSC_DELTAMSTEP_DPLL_PER
        //printf("Last 2 \n");
	//SETREG32(0x4a00816c, 0x00000000 ); //CM_SSC_MODFREQDIV_DPLL_PER
        //printf("Last 1 \n");
	//SETREG32(0x4a008140, 1<<12);//CM_CLKMODE_DPLL_PER
        //printf("Last 0 \n");
	/* 	f_inp=38.4MHz,M=20,N=0,M5=10,LCD=1,PCD=3
		DISPC2_PCLK=38.4*20/(0+1)*2/10/1/3=51.2MHz
		f_c=f_inp*M*2/((N+1)*M5)=38.4M*20*2/((0+1)*10)
	*/
	/* CM_DIV_M5_DPLL_PER Set bit8 = 1, force HSDIVIDER_CLKOUT2 clock enabled*/
	__raw_writew(__raw_readw(0x4A00815C) | 0x100, 0x4A00815C);
	/* CM_SSC_DELTAMSTEP_DPLL_PER */
	__raw_writew(0XCC , 0x4A008168);
	/* CM_SSC_MODFREQDIV_DPLL_PER */
	__raw_writew(0X264 , 0x4A00816C);
	/* CM_CLKMODE_DPLL_PER Set bit12 = 1, force DPLL_SSC_EN enabled*/
	__raw_writew(__raw_readw(0x4A008140) | 0x1000 , 0x4A008140);
#endif

}

void gpio_37_high(void)
{
//kc1 RGB to LVDS
//gpio_37
//printf("The gpio 37 value is %x!\n",__raw_readw(0x4a10005A));
__raw_writew(0x1B,0x4a10005A);
//printf("The gpio 37 value is %x!\n",__raw_readw(0x4a10005A));
__raw_writew(__raw_readw(0x48055134) & 0xFFDF, 0x48055134);//gpio_OE gpio37
//printf("The gpio 37 GPIO_OE value is %x!\n",__raw_readl(0x48055134));
__raw_writew(__raw_readw(0x4805513C) | 0x0020, 0x4805513C);//gpio_dataout gpio37
//printf("The gpio 37 GPIO_DATAOUT value is %x!\n",__raw_readl(0x4805513C));
}

void gpio_37_low(void)
{
//kc1 RGB to LVDS
//gpio_37
//printf("The gpio 37 value is %x!\n",__raw_readw(0x4a10005A));
__raw_writew(0x0B,0x4a10005A);
//printf("The gpio 37 value is %x!\n",__raw_readw(0x4a10005A));
__raw_writew(__raw_readw(0x48055134) & 0xFFDF, 0x48055134);//gpio_OE gpio37
//printf("The gpio 37 GPIO_OE value is %x!\n",__raw_readl(0x48055134));
__raw_writew(__raw_readw(0x4805513C) & 0xFFDF, 0x4805513C);//gpio_dataout gpio37
//printf("The gpio 37 GPIO_DATAOUT value is %x!\n",__raw_readl(0x4805513C));
}

void gpio_121_high(void)
{
//Turn on kc1 backlight for A1A
//gpio_121
//printf("The gpio 121 value is %x!\n",__raw_readw(0x4a100114));
__raw_writew(0x1B,0x4a100114);
//printf("The gpio 121 value is %x!\n",__raw_readw(0x4a100114));
__raw_writel(__raw_readl(0x48059134) & 0xFDFFFFFF, 0x48059134);//gpio_OE gpio_121
//printf("The gpio 121 GPIO_OE value is %x!\n",__raw_readl(0x48059134));
__raw_writel(__raw_readl(0x4805913C) | 0x02000000, 0x4805913C);//gpio_dataout gpio_121
//printf("The gpio 121 GPIO_DATAOUT value is %x!\n",__raw_readl(0x4805913C));
}

void gpio_121_low(void)
{
//Turn off kc1 backlight for A1A
//gpio_121
//printf("The gpio 121 value is %x!\n",__raw_readw(0x4a100114));
__raw_writew(0x0B,0x4a100114);
//printf("The gpio 121 value is %x!\n",__raw_readw(0x4a100114));
__raw_writel(__raw_readl(0x48059134) & 0xFDFFFFFF, 0x48059134);//gpio_OE gpio_121
//printf("The gpio 121 GPIO_OE value is %x!\n",__raw_readl(0x48059134));
__raw_writel(__raw_readl(0x4805913C) & 0xFDFFFFFF, 0x4805913C);//gpio_dataout gpio_121
//printf("The gpio 121 GPIO_DATAOUT value is %x!\n",__raw_readl(0x4805913C));
}

void gpio_94_high(void)
{
//Turn on kc1 backlight
//gpio_94
//printf("The gpio 94 value is %x!\n",__raw_readw(0x4a1000D6));
__raw_writew(0x1B,0x4a1000D6);
//printf("The gpio 94 value is %x!\n",__raw_readw(0x4a1000D6));
__raw_writel(__raw_readl(0x48057134) & 0xBFFFFFFF, 0x48057134);//gpio_OE gpio_94
//printf("The gpio 94 GPIO_OE value is %x!\n",__raw_readl(0x48057134));
__raw_writel(__raw_readl(0x4805713C) | 0x40000000, 0x4805713C);//gpio_dataout gpio_94
//printf("The gpio 94 GPIO_DATAOUT value is %x!\n",__raw_readl(0x4805713C));

//gpio_119
__raw_writel(0x001B001B,0x4a100110);//gpio_119 and gpio_120
__raw_writel(__raw_readl(0x48059134) & 0xFF7FFFFF, 0x48059134);//gpio_OE gpio_119
__raw_writel(__raw_readl(0x4805913C) | 0x00800000, 0x4805913C);//gpio_dataout gpio_119

//gpio_120
__raw_writel(__raw_readl(0x48059134) & 0xFEFFFFFF, 0x48059134);//gpio_OE gpio_120
__raw_writel(__raw_readl(0x4805913C) | 0x01000000, 0x4805913C);//gpio_dataout gpio_120

}

void gpio_94_low(void)
{
//Turn off kc1 backlight
//gpio_94
//printf("The gpio 94 value is %x!\n",__raw_readw(0x4a1000D6));
__raw_writew(0x0B,0x4a1000D6);
//printf("The gpio 94 value is %x!\n",__raw_readw(0x4a1000D6));
__raw_writel(__raw_readl(0x48057134) & 0xBFFFFFFF, 0x48057134);//gpio_OE gpio_94
//printf("The gpio 94 GPIO_OE value is %x!\n",__raw_readl(0x48057134));
__raw_writel(__raw_readl(0x4805713C) & 0xBFFFFFFF, 0x4805713C);//gpio_dataout gpio_94
//printf("The gpio 94 GPIO_DATAOUT value is %x!\n",__raw_readl(0x4805713C));

//gpio_119
__raw_writel(0x000B000B,0x4a100110);//gpio_119 and gpio_120
__raw_writel(__raw_readl(0x48059134) & 0xFF7FFFFF, 0x48059134);//gpio_OE gpio_119
__raw_writel(__raw_readl(0x4805913C) & 0xFF7FFFFF, 0x4805913C);//gpio_dataout gpio_119

//gpio_120
__raw_writel(__raw_readl(0x48059134) & 0xFEFFFFFF, 0x48059134);//gpio_OE gpio_120
__raw_writel(__raw_readl(0x4805913C) & 0xFEFFFFFF, 0x4805913C);//gpio_dataout gpio_120


}

void set_omap_pwm(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int i, counterR;
    long brightness;

    if (argc < 2) {
        printf("Usage:\n%s\n", cmdtp->usage);
        return;
    }

    brightness = simple_strtol(argv[1], NULL, 16);
    if (brightness < 0 || brightness > 255) {
        printf("Usage:\nsetbacklight 0x0 - 0xFF\n");
        return;
    }
    //printf("brightness = %d\n", brightness);

    //printf("GPT10 PWM Set \n");
#if 0
    //kc1 backlight PWM enable for B1B
    //printf("The GPTIMER10 pin mux is %x!\n",__raw_readw(0x4a1000D6));
    __raw_writew(0x01,0x4a1000D6);
    udelay(10000);

    //printf("The GPTIMER10 pin mux is %x!\n",__raw_readw(0x4a1000D6));
    __raw_writew(__raw_readw(0x48057134) | 0x4000, 0x48057134);//gpio_OE - Dissable gpio_94_OE
    udelay(10000);
    //printf("The gpio 94 GPIO_OE value is %x!\n",__raw_readl(0x48057134));
    __raw_writew(__raw_readw(0x4805713C) & 0xBFFF, 0x4805713C);//gpio_dataout - Disable gpio_94_dataout
    udelay(10000);
    //printf("The gpio 94 GPIO_DATAOUT value is %x!\n",__raw_readl(0x4805713C));
#endif

    /* Pull up GPIO 119 & 120 */
    __raw_writel(0x001B001B, 0x4a100110);
    //GPIO_OE
    __raw_writel(__raw_readl(0x48059134) & ~(0x11 << 23) , 0x48059134);
    //GPIO_DATAOUT
    __raw_writel(__raw_readl(0x4805913C) | (0x11 << 23), 0x4805913C);


    /* CM_CLKSEL_CORE to select 32KHz (0) or CM_SYS_CLK=26Mhz (1) as clock source */
    *(int*)0x4A009428 &= ~(0x1 << 24);

    /* GPT10 clock enable */
    *(int*)0x4A009428 |= 0x2;

    /* Set autoreload mode */
    *(int*)0x48086024 |= 0x2;

    /* Enable prescaler */
    *(int*)0x48086024 |= (0x1 << 5);

    /* GPT10 PWM configuration */

    *(int*)0x48086040 = 0x00000004;   /* TSICR */
    //*(int*)0x48086038 = 0xFFFF7FFF;   /* TMAR */
    if (brightness == 0xFF) {
        //Set brightness
        *(int*)0x48086038 = 0xFFFFFF00;   /* TMAR */
        //*(int*)0x4808602C = 0xFFFF3FFF;   /* TLDR */
        *(int*)0x4808602C = 0xFFFFFF00;   /* TLDR */
        *(int*)0x48086030 = 0x00000001;    /* TTGR */

        *(int*)0x48086028 = 0xFFFFFF00;    /* TCRR */

        //*(int*)0x48086024 = 0x000018E3;   /* TCLR */
        *(int*)0x48086024 = 0x000018e3;   /* TCLR */
    }
    else {
        //Set brightness
        *(int*)0x48086038 = 0xFFFFFF00 | (brightness & 0xFF);   /* TMAR */
        //*(int*)0x4808602C = 0xFFFF3FFF;   /* TLDR */
        *(int*)0x4808602C = 0xFFFFFF00;   /* TLDR */
        *(int*)0x48086030 = 0x00000001;    /* TTGR */

        *(int*)0x48086028 = 0xFFFFFF00;    /* TCRR */

        //*(int*)0x48086024 = 0x000018E3;   /* TCLR */
        *(int*)0x48086024 = 0x00001863;   /* TCLR */
    }

    counterR = (*(int*)0x48086028);
    //printf(" TCRR = %x \n", (int) counterR);

    //printf("GPT10 PWM Set Completed\n");
}


U_BOOT_CMD(
	gpio_121_high, 1, 0, gpio_121_high, "gpio_121_high - gpio_121_high.\n", NULL
);

U_BOOT_CMD(
	gpio_121_low, 1, 0, gpio_121_low, "gpio_121_low - gpio_121_low.\n", NULL
);

U_BOOT_CMD(
	gpio_37_high, 1, 0, gpio_37_high, "gpio_37_high - gpio_37_high.\n", NULL
);

U_BOOT_CMD(
	gpio_37_low, 1, 0, gpio_37_low, "gpio_37_low - gpio_37_low.\n", NULL
);

U_BOOT_CMD(
	gpio_94_high, 1, 0, gpio_94_high, "gpio_94_high - gpio_94_high.\n", NULL
);

U_BOOT_CMD(
	gpio_94_low, 1, 0, gpio_94_low, "gpio_94_low - gpio_94_low.\n", NULL
);

U_BOOT_CMD(
	setbacklight, 2, 0, set_omap_pwm, "setbacklight - set omap pwm.\n", "setbacklight 0x0 - 0xFF\n"
);

U_BOOT_CMD(
	show_white_data, 1, 0, show_white_data, "show_white_data - show white data.\n", NULL
);

U_BOOT_CMD(
	show_black_data, 1, 0, show_black_data, "show_black_data - show black data.\n", NULL
);

U_BOOT_CMD(
	showlogo, 1, 0, showlogo, "showlogo - show quanta logo.\n", NULL
);

U_BOOT_CMD(
	showlowbattery, 1, 0, show_lowbattery, "showlowbattery - show lowbattery image.\n", NULL
);

U_BOOT_CMD(
	lbtinit, 1, 0, lbt_datainit, "lbtinit - prepare lowbattery image.\n", NULL
);
