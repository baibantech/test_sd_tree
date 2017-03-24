/*************************************************************************
	> File Name: test_case_main.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: Tue 21 Feb 2017 07:35:16 PM PST
 ************************************************************************/

#include<stdio.h>
#include "data_set.h"
#include "data_cache.h"
char test_case_name[64] = {'t','e','s','t','_','c','a','s','e'};

typedef void (*test_proc_pfn)(void *args);

enum cmd_index
{
	SET_NAME,
	DATA_SIZE ,
	DATA_NUM,
	RANDOM_SET,
	FILE_SIZE,
	MAP_START_ADDR,
	MAP_READ_START,
	MAP_READ_LEN,
	CACHE_UNIT_SIZE,
};

struct cmdline_dsc
{
	int cmd_id;
	char *cmd;
	char *dsc;
};

struct cmdline_dsc cmdline[] = 
{
	{.cmd_id = SET_NAME,.cmd = "set_name",.dsc = "data set name ,type string"},
	
	{.cmd_id = DATA_SIZE,.cmd = "instance_size",.dsc = "data instance len,type value"},
	{.cmd_id = DATA_NUM,.cmd = "instance_num",.dsc = "data instance num,type value"},
	
	{.cmd_id = RANDOM_SET,.cmd = "random_set",.dsc = "1 for random data construct ,otherwise specfic data set files"},
	{.cmd_id = FILE_SIZE,.cmd = "data_set_file_size",.dsc = "per data set file len,type value"},

	{.cmd_id = MAP_START_ADDR ,.cmd = "map_start_addr",.dsc = "data set map to memory from file,type value"},
	{.cmd_id = MAP_READ_START,.cmd = "map_read_start",.dsc = "the start data instance maped to memory start from 0 to instance_num-1",},
	{.cmd_id = MAP_READ_LEN,.cmd = "map_read_len",.dsc = "the instance num maped to memory from 1 to instance_num"},

	{.cmd_id = CACHE_UNIT_SIZE,.cmd = "read_cache_unit_size",.dsc = "the unit len read from map to test"},
};

enum test_proc_type
{
	find_proc,
	insert_proc,
	delete_proc,
};

struct proc_type
{
	int type;
	test_proc_pfn pfn;
};



struct proc_type test_proc_array[] = 
{
	{.type = insert_proc,.pfn = test_insert_proc},
	
	{.type = delete_proc,.pfn = test_delete_proc},
};

#define get_cmd_id 0
#define get_cmd_value_begin 1
#define get_cmd_value 2
#define get_cmd_end 3
void print_cmd_help_info(void)
{
	int i = 0;
	printf("\r\ncmd style is xxx=xxx deperated by one space\r\n");
	
	for(i = 0; i < sizeof(cmdline)/sizeof(struct cmdline_dsc); i++)
	{
		printf("%s : %s \r\n",cmdline[i].cmd,cmdline[i].dsc);
	}
	return ;
}


int  parse_cmdline(int argc,char *argv[])
{
	int i ,j, k ;
	int cmd_id = -1;
	int step = get_cmd_id;

	for(i = 1; i < argc ; i++){
		j = 0;
		if(1 == i)
		{
			if(argv[i][0] == '?')
			{
				print_cmd_help_info();
				return 1;
			}
		}
		step = get_cmd_id;
		while(argv[i][j] != '\0')
		{
			if(argv[i][j]!= ' '){
				switch(step)
				{
					case get_cmd_id:
					{
						for(k = 0; k < sizeof(cmdline)/sizeof(struct cmdline_dsc); k++)
						{
							if(strncmp(&argv[i][j],cmdline[k].cmd,strlen(cmdline[k].cmd))== 0)
							{
								cmd_id = cmdline[k].cmd_id;
								j += strlen(cmdline[k].cmd);
								step = get_cmd_value_begin;
								break;
							}
						}
						if(-1 == cmd_id){
							goto error_cmd;
						}
						break;
					}
					
					case get_cmd_value_begin:
					{
						if(argv[i][j] != '=')
						{
							goto error_cmd;	
						}
						else
						{
							step = get_cmd_value;
							j++;
						}
						break;
					}

					case get_cmd_value:
					{
						long long value = -1;
						
						switch (cmd_id)
						{
							case SET_NAME:
							{
								strncpy(test_case_name,&argv[i][j],64);
								printf(test_case_name);
								break;
							}
							
							case DATA_SIZE:
							{
								value = strtol(&argv[i][j],NULL,10);
								printf("value is %lld\r\n",value);
								set_instance_len_config(value);
								break;
							}

							case DATA_NUM :
							{
								value = strtol(&argv[i][j],NULL,10);
								printf("value is %lld\r\n",value);
								set_instance_num_config(value);
								break;
							}
							case RANDOM_SET:
							{
								value = strtol(&argv[i][j],NULL,10);
								printf("value is %lld\r\n",value);
								set_instance_random(value);
								break;
							}
							
							case FILE_SIZE:
							{
								value = strtol(&argv[i][j],NULL,10);
								printf("value is %lld\r\n",value);
								set_data_file_len_config(value);
								break;
							}

							case MAP_START_ADDR:
							{
								value = strtol(&argv[i][j],NULL,10);
								printf("value is %lld\r\n",value);
								set_data_map_start_addr(value);
								break;
							}
							case MAP_READ_START:
							{
								value = strtol(&argv[i][j],NULL,10);
								printf("value is %lld\r\n",value);
								set_map_read_start(value);
								break;
							}
							case MAP_READ_LEN:
							{
								value = strtol(&argv[i][j],NULL,10);
								printf("value is %lld\r\n",value);
								set_map_read_len(value);
								break;
							}
						
							case CACHE_UNIT_SIZE:
							{
								value = strtol(&argv[i][j],NULL,10);
								printf("value is %lld\r\n",value);
								set_read_cache_unit_size(value);
								break;
							}	
							default :
							{
								printf("error cmd id in get value\r\n");
								goto error_cmd;
							}
						
						}
						step = get_cmd_end;

						break;
					}
					
					case get_cmd_end:
					{
						j++;
						break;
					}

					default :
						printf("error parse cmd step\r\n");
						goto error_cmd;
				}
			}
			else
			{
				j++;
			}
		}
	}
	
	return 0;

error_cmd:
	printf("error cmdline %s\r\n",argv[i]);
	return -1;
}

struct data_set_file *set_file_list = NULL;
int main(int argc,char *argv[])
{
	int ret = -1;
	void *addr = NULL;
	long long get_data_len;
	
	if(parse_cmdline(argc,argv))
	{
		return 0;
	}
	set_file_list = get_data_set_file_list();

	if(NULL == set_file_list)
	{
		printf("get file list error\r\n");
		return 0;
	}
	ret = construct_data_set(set_file_list);
	printf("construct_data_set ret is %d\r\n",ret);
	
	data_set_config_map_address = 0x7fa0746e000;
	
	addr  = map_data_set_file_anonymous(set_file_list,0x7fa0746e000);
	//printf("map data set file ret is %d\r\n",ret);
	if(-1 == data_set_config_map_read_start)
	{
		data_set_config_map_read_start = 0;
	}

	if(-1 == data_set_config_map_read_len )
	{
		get_data_len = -1;
	}
	else
	{
		get_data_len = data_set_config_map_read_len*data_set_config_instance_len;
	}
	get_data_from_file(set_file_list,data_set_config_map_read_start*data_set_config_instance_len,get_data_len);
	
	test_insert_proc(NULL);

	return 0;

}

