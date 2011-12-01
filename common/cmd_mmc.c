/*
 * (C) Copyright 2003
 * Kyle Harris, kharris@nexus-tech.net
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
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

#include <common.h>
#include <command.h>
#include <malloc.h>	//kc1_quanta_river

#if (CONFIG_COMMANDS & CFG_CMD_MMC)

#include <mmc.h>

#define MMC_BLOCK_SIZE		512	//1 page <=> 512 bytes


typedef struct nvram_t {
    const char *name;
    unsigned int offset;
    unsigned int size;
} nvram_t;

static const struct nvram_t nvram_info[] = {
    {
	.name = "serial",
	.offset = 0x40,
	.size = 16,
    },
    {
	.name = "mac",
	.offset = 0x01,
	.size = 12,
    },
    {
	.name = "sec",
	.offset = 0x00,
	.size = 20,
    },
    {
	.name = "pcbsn",
	.offset = 0x03,
	.size = 16,
    },
    {
	.name = "bootmode",
	.offset = 0x0006,
	.size = 16,
    },
    {
	.name = "postmode",
	.offset = 0x2060,
	.size = 16,
    },
    {
	.name = "bootcounter",
	.offset = 0x2040,
	.size = 16,
    },
    {
	.name = "lpddr2",
	.offset = 0x3060,
	.size = 16,
    },
    {
	.name = "emmc",
	.offset = 0x3050,
	.size = 16,
    },
    {
	.name = "product",
	.offset = 0x6070,
	.size = 16,
    },
};

#define CONFIG_NUM_NV_VARS (sizeof(nvram_info)/sizeof(nvram_info[0]))


int mmc_flag[2] = {0, 0} ;

unsigned long getbootmode(void);

unsigned long getbootmode(void){
	int slot_no = 1;	//1 for EMMC;
	unsigned char* buffer = NULL;
	int offset;
	unsigned long value;
	//1. init mmc_slot 1 (emmc)
	if (mmc_init(slot_no) != 0){
		printf("init mmc failed!");
		return 1;
	}
			
	//2. switch to access boot partition 1
	mmc_sw_part(slot_no, 1);
		
	//3. allocate memory 	
	buffer = (unsigned char*)malloc(MMC_BLOCK_SIZE);
	
	//printf("address of buffer = 0x%x\n",buffer);	//debug
		
	mmc_read(slot_no, (nvram_info[4].offset / MMC_BLOCK_SIZE)/*page_number*/,buffer, 1/*1 page*/);
	offset=nvram_info[4].offset % MMC_BLOCK_SIZE;
	//printf("============%x\n",simple_strtoul(buffer, NULL, 16));
	value=simple_strtoul(buffer+offset, NULL, 16);
	free(buffer);
	return value;
}

unsigned long get_bootcounter(void);

unsigned long get_bootcounter(void){
        int slot_no = 1;        //1 for EMMC;
        unsigned char* buffer = NULL;
        int offset;
        unsigned long value;
        //1. init mmc_slot 1 (emmc)
        if (mmc_init(slot_no) != 0){
                printf("init mmc failed!");
                return 1;
        }

        //2. switch to access boot partition 1
        mmc_sw_part(slot_no, 1);

        //3. allocate memory
        buffer = (unsigned char*)malloc(MMC_BLOCK_SIZE);

        //printf("address of buffer = 0x%x\n",buffer);  //debug

        mmc_read(slot_no, (nvram_info[6].offset / MMC_BLOCK_SIZE)/*page_number*/,buffer, 1/*1 page*/);
        offset=nvram_info[6].offset % MMC_BLOCK_SIZE;
        //printf("============%x\n",simple_strtoul(buffer, NULL, 16));
        value=simple_strtoul(buffer+offset, NULL, 16);
        free(buffer);
        return value;
}


int do_mmc (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong src_addr, dst_addr, size;
	char *cmd;
	/*Default Setting to SLOT-0*/
	int slot_no = 0, mmc_cont = 0;

	if (argc < 2) {
		goto mmc_cmd_usage;
	} else if (argc == 2) {
                if (strncmp(argv[0],"mmcinit",7) !=0) {
                        goto mmc_cmd_usage;
                } else {
			slot_no = simple_strtoul(argv[1], NULL, 16);
			if ((slot_no != 0) && (slot_no != 1))
				goto mmc_cmd_usage;
			if (mmc_init(slot_no) != 0) {
				printf("No MMC card found\n");
                                return 1;
			} else {
				mmc_flag[slot_no] = 1;
			}
                }
	} else {
		mmc_cont = simple_strtoul(argv[1], NULL, 16);
		if ((mmc_cont != 0) && (mmc_cont != 1))
			goto mmc_cmd_usage;

		if (!mmc_flag[mmc_cont]) {
			printf("Try to do init First b4 read/write\n");
			goto mmc_cmd_usage;
		}

		cmd = argv[2];
                if (	strncmp(cmd, "read", 4) != 0 
                     && strncmp(cmd, "write", 5) != 0
                     && strncmp(cmd, "erase", 5) != 0
                     && strncmp(cmd, "sw_part", 7) != 0)
                goto mmc_cmd_usage;

                if (strcmp(cmd, "erase") == 0) {
			if (argc != 5) {
				goto mmc_cmd_usage;
			} else {
				src_addr = simple_strtoul(argv[3], NULL, 16);
				size = simple_strtoul(argv[4], NULL, 16);
				mmc_erase(mmc_cont, src_addr, size);
			}
		}
                if (strcmp(cmd, "read") == 0) {
			if (argc != 6) {
                                goto mmc_cmd_usage;
                        } else {
				src_addr = simple_strtoul(argv[3], NULL, 16);
				dst_addr = simple_strtoul(argv[4], NULL, 16);
				size = simple_strtoul(argv[5], NULL, 16);
				mmc_read(mmc_cont, src_addr,
					(unsigned char *)dst_addr, size);
			}
		}
		if (strcmp(cmd, "write") == 0) {
			if (argc != 6) {
				goto mmc_cmd_usage;
			} else {
				src_addr = simple_strtoul(argv[3], NULL, 16);
				dst_addr = simple_strtoul(argv[4], NULL, 16);
				size = simple_strtoul(argv[5], NULL, 16);
				mmc_write(mmc_cont, (unsigned char *)src_addr,
							dst_addr, size);
			}
		}
		//kc1_quanta
		if (strcmp(cmd, "sw_part") == 0) {
			if (argc != 4) {
				goto mmc_cmd_usage;
			} else {
				src_addr = simple_strtoul(argv[3], NULL, 16);
				//printf("sw_part src_addr = %d\n",src_addr);	//debug
				mmc_sw_part(mmc_cont, src_addr);
			}
		}
		
	}
	return 0;

mmc_cmd_usage:
	//printf("Usage:\n%s\n", cmdtp->usage);		//kc1_quanta_river
	printf("Usage:\n%s\n", cmdtp->help);
	return 1;
}

// mem_disp : dump "length" bytes of data in "buf" to console
int mem_disp (unsigned char* buf ,int length)
{
	int i = 0; 
	
	if (length < 0 || length > MMC_BLOCK_SIZE) return -1;
	
	for (i = 0 ; i< length ; i++){

		if (*(buf+i) < 0x20 || *(buf+i) > 0x7e )
			putc('.');
		else
			printf("%c",*(buf+i));
		
		if ((i+1)%16 == 0)
			printf("\n");
	
	}
	
	return 0;
}

// string_show : show string stored in the buffer
int string_show (unsigned char* buf , int num)
{
	int i = 0; 
	int fine_offset = 0 ;
	
	if (num > CONFIG_NUM_NV_VARS) return -1;

	fine_offset = nvram_info[num].offset % MMC_BLOCK_SIZE;

	printf("%s",nvram_info[num].name);
	for (i=0; i<= (15-strlen(nvram_info[num].name)); i++){
		putc(' ');
	}
	printf("0x%04x",nvram_info[num].offset);

	for (i=1; i<=12; i++){
		putc(' ');
	}

	for (i = 0 ; i< nvram_info[num].size ; i++){

		if (*(buf+i+fine_offset) < 0x20 || *(buf+i+fine_offset) > 0x7e )
			putc('.');
		else
			printf("%c",*(buf+i+fine_offset));
	}

	printf("\n");
	return 0;
}

int do_rw_bp1 (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int slot_no = 1;	//1 for EMMC;
	int ret = 0 ;
	unsigned char* buffer = NULL;
	int i = 0;		
	int seq_no = -1;
	int rw_size, rw_page = 0;
	int rw_offset = 0;
	
	if (argc <= 1 || argc >= 4 ) 
		goto idme_cmd_usage;
	else{
		//0. check command validity
                if (strncmp(argv[0],"idme",4) !=0)
                        goto idme_cmd_usage;
		
		//1. init mmc_slot 1 (emmc)
		if (mmc_init(slot_no) != 0){
			printf("init mmc failed!");
			return 1;
		}
			
		//2. switch to access boot partition 1
		mmc_sw_part(slot_no, 1);	
		
		//3. allocate memory and initialize to 0x20	
		buffer = (unsigned char*)malloc(MMC_BLOCK_SIZE);
		memset(buffer, 0x20, MMC_BLOCK_SIZE);		//initialize to all 0x20
		//printf("add:qress of buffer = 0x%x\n",buffer);	//debug
		
		if ( argc == 2 ){ //dump value to console	
			if (strcmp(argv[1],"?") == 0){

				//mem_disp(buffer,MMC_BLOCK_SIZE);	//debug

				printf("<================== idme nvram info ==================\n");
				printf("Name		offset		  value    \n");
				printf("----------------------------------|---|---|---|---|---|\n");
				for(i=0; i<CONFIG_NUM_NV_VARS; i++){
					mmc_read(1, (nvram_info[i].offset / MMC_BLOCK_SIZE)/*page_number*/,buffer, 1/*1 page*/);
					string_show(buffer,i);
				}
				printf("=================== idme nvram info ==================>\n");
			}
			else{
				free(buffer);
				goto idme_cmd_usage;
			}
		}
		else {	//argc == 3 //write

			for(i=0; i<CONFIG_NUM_NV_VARS; i++){
				if (strcmp(argv[1],nvram_info[i].name) == 0){
					seq_no = i;
					//printf("seq_no = %d   name = %s\n",seq_no,nvram_info[seq_no].name);	//debug
					break;
				}
			}
			//4. check if the item is defined in the list
			if (seq_no < 0 || seq_no > CONFIG_NUM_NV_VARS){
				printf("<idme>\"%s\" not found!\n",argv[1]);
				goto clean; 
			}
			//5. check input data length 	
			if (strlen (argv[2]) > nvram_info[seq_no].size){
				printf("<idme> incorrect data length, please try again.\n");
				goto clean;
			}

			rw_offset = nvram_info[seq_no].offset % MMC_BLOCK_SIZE;
			rw_page = nvram_info[seq_no].offset / MMC_BLOCK_SIZE;
			rw_size = nvram_info[seq_no].size;
			
			//6. read target page(512 bytes) to buffer and replace correspond bytes with desired value 
			mmc_read(slot_no, rw_page/*page_number*/,buffer, 1/*1 page*/);
			memset(&buffer[rw_offset], 0x20, rw_size);
			memcpy(&buffer[rw_offset],argv[2],strlen(argv[2]));			

			//7. write back to EMMC page number "rw_page"
			if (mmc_write(slot_no, (unsigned char *)buffer, rw_page, 1/*1 page*/) < 0){
				printf("<idme> write mmc error \n ");
				goto clean;
			}
			else{	//write success
				printf("<idme> write %s to offset 0x%04x\n",argv[2],nvram_info[seq_no].offset);
			}

			
		}
		free(buffer);
	}	
	
	return 0;
	
idme_cmd_usage:	
	printf("Usage:\n%s\n", cmdtp->help);
	return 1;

clean:
	if (buffer) free(buffer);
	return 1;
	
}

int fastboot_idme(const char *cmd){
    char *token;
    char ver[32], value[32];
    char *idme[3]  = {"idme", NULL, NULL};
    int i=2;
    idme[1]=ver;
    idme[2]=value;
    printf("The token is:  \"%s\"\n", cmd);
    token = strtok(cmd," ");
    sprintf(ver, "%s", token);
    while (token != NULL){
        token = strtok(NULL," ");
        if(token != NULL){
            sprintf(value, "%s", token);
            i++;
        }else{
            break;
        }
        //printf("The token is:  %s\n", token);        
    }
    printf("The token is:  %d %s   %s %s\n",i, idme[0],idme[1],idme[2]);
    return do_rw_bp1 (NULL, 0, i, idme);
}

int increase_bootcounter(const char *cmd){
    unsigned long counter = get_bootcounter();
    char buffer[16];
    sprintf(buffer, "%x", counter+1);
    char *idme[3]  = {"idme", "bootcounter", buffer};
    int i=3;
    return do_rw_bp1 (NULL, 0, i, idme);
}

int set_lpddr2(const char *cmd){
    char *idme[3]  = {"idme", "lpddr2", cmd};
    return do_rw_bp1(NULL, 0, 3, idme);
}

int set_emmc(const char *cmd){
    char *idme[3]  = {"idme", "emmc", cmd};
    return do_rw_bp1(NULL, 0, 3, idme);
}

int set_product(const char *cmd){
    char *idme[3]  = {"idme", "product", cmd};
    return do_rw_bp1(NULL, 0, 3, idme);
}

U_BOOT_CMD(mmcinit, 6, 1, do_mmc,
	"mmcinit - init MMC sub system\n",
	"mmcinit <controller[0/1]>  0-> sdcard  1-> emmc\n"
	"mmc <controller[0/1]> read <src> <dst> <size>\n"
	"mmc <controller[0/1]> write <src> <dst> <size>\n"
	"mmc <controller[0/1]> erase <start> <size>\n"
	"mmc <controller[0/1]> sw_part <part>\n");	//kc1_quanta
	
	
U_BOOT_CMD(idme, 3, 1, do_rw_bp1,
	"idme    - idme control interface\n",
	"idme <var> <value> --- set a variable value \n"
	"idme ?             --- print out known values\n");	//kc1_quanta_river
	
	
	
#endif  /* CFG_CMD_MMC */
