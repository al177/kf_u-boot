#include <asm/byteorder.h>
#include <common.h>
#include <command.h>
#include <asm/io.h>
#include <twl6030.h>

#define LED_PWM1ON           0x00
#define LED_PWM1OFF          0x01

#define LED_PWM2ON		            0x03
#define LED_PWM2OFF		            0x04
#define TWL6030_TOGGLE3		        0x92

int do_setgreenled (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_setamberled (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

int do_setamberled (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int ret = 0;
	long brightness;
	long tmp;
	unsigned char data;

	if(get_i2c_bus() != 0 ) {
		if (select_bus(0,400) == 0 ) { /* configure I2C1 */
			printf("====   Change to bus 1.  ====\n");
		}
	}
	if (argc < 2) {
		printf("Usage[%d]:\n%s\n", argc, cmdtp->usage);
		return 1;
	}
	tmp=0xff;
	i2c_write(TWL6030_CHIP_PWM, LED_PWM2ON, 1, &tmp, 1);
	tmp=0x7f;
	i2c_write(TWL6030_CHIP_PWM, LED_PWM2OFF, 1, &tmp, 1);
	//select_bus(0,400);
	brightness = simple_strtol(argv[1], NULL, 16);
	if (brightness > 1) {
		if (brightness == 255)
			brightness = 0x7f;
		else
			brightness = (~(brightness/2)) & 0x7f;

		data = 0x30;
		i2c_write(TWL6030_CHIP_PWM, TWL6030_TOGGLE3, 1, &data, 1);
		i2c_write(TWL6030_CHIP_PWM, LED_PWM2ON, 1, &brightness, 1);
	} else if (brightness <= 1) {
		data = 0x8;
		i2c_write(TWL6030_CHIP_PWM, TWL6030_TOGGLE3, 1, &data, 1);
		data = 0x38;
		i2c_write(TWL6030_CHIP_PWM, TWL6030_TOGGLE3, 1, &data, 1);
	}
	return ret;
}

int do_setgreenled (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int ret = 0;
	long brightness;
	long tmp;
	unsigned char data;

	if(get_i2c_bus() != 0 ) {
		if (select_bus(0,400) == 0 ) { /* configure I2C1 */
			printf("====   Change to bus 1.  ====\n");
		}
	}
	if (argc < 2) {
		printf("Usage[%d]:\n%s\n", argc, cmdtp->usage);
		return 1;
	}
	tmp=0xff;
	i2c_write(TWL6030_CHIP_PWM, LED_PWM1ON, 1, &tmp, 1);
	tmp=0x7f;
	i2c_write(TWL6030_CHIP_PWM, LED_PWM1OFF, 1, &tmp, 1);
	//select_bus(0,400);
	brightness = simple_strtol(argv[1], NULL, 16);
	if (brightness > 1) {
		if (brightness == 255)
			brightness = 0x7f;
		else
			brightness = (~(brightness/2)) & 0x7f;

		data = 0x6;
		i2c_write(TWL6030_CHIP_PWM, TWL6030_TOGGLE3, 1, &data, 1);
		i2c_write(TWL6030_CHIP_PWM, LED_PWM1ON, 1, &brightness, 1);
	} else if (brightness <= 1) {
		data = 0x1;
		i2c_write(TWL6030_CHIP_PWM, TWL6030_TOGGLE3, 1, &data, 1);
		data = 0x7;
		i2c_write(TWL6030_CHIP_PWM, TWL6030_TOGGLE3, 1, &data, 1);
	}
	return ret;
}

U_BOOT_CMD(
	setgreenled,	2,	0,	do_setgreenled,
	"setgreenled - set green led brightness.\n",
	NULL
);


U_BOOT_CMD(
	setamberled,	2,	0,	do_setamberled,
	"setamberled - set amber led brightness.\n",
	NULL
);

