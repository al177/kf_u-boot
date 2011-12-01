/*
 * (C) Copyright 2009
 * Texas Instruments, <www.ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <config.h>
#ifdef CONFIG_BATTERY_BQ27541
#include <bq27541.h>
#include <asm/io.h>
static inline int bq27_i2c_read_u16(u8 chip_no, u16 *val, u8 reg)
{
    int mbid=0;
    mbid=get_mbid();
    if(mbid>=4)
        select_bus(0,100);
    void *data[2]={0};
    if(i2c_read(chip_no, reg, 1, data, 2)==0){
        *val=*(u16 *)data;
        return 0;
    }else
        return -1;
}
static inline signed int bq27_i2c_read_int(u8 chip_no, short *val, u8 reg)
{
    int mbid=0;
    mbid=get_mbid();
    if(mbid>=4)
        select_bus(0,100);
    void *data[2]={0};
    if(i2c_read(chip_no, reg, 1, data, 2)==0){
        *val=*(short *)data;
        return 0;
    }else
        return -1;
}
int get_bat_voltage(){
    u16 bat =0;
    if(bq27_i2c_read_u16(BQ27541_ADDRESS,&bat,0x08)==0){
    //printf("Battery voltage=%d mV\n",bat);
        return bat;
    }else
        return -1;
}
int get_bat_temperature(){
    u16 temp =0;
    short celsius =0;
    if(bq27_i2c_read_u16(BQ27541_ADDRESS,&temp,0x06)==0){
        //Kelvin to Celsius = K -273.15
        celsius=temp-2731;
        return celsius/10;
    }else
        return -1;
}
int get_bat_current(){
    short current =0;
    bq27_i2c_read_int(BQ27541_ADDRESS,&current,0x14);
    //printf("Battery current=%d mA\n",current);
    return current;
}
int get_bat_capacity(){
    short capacity =0;
    bq27_i2c_read_int(BQ27541_ADDRESS,&capacity,0x2c);
    //printf("Battery current=%d mA\n",current);
    return capacity;
}
#endif
