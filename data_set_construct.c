/*************************************************************************
	> File Name: data_set_construct.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: Sun 26 Feb 2017 05:59:53 PM PST
 ************************************************************************/

#include<stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#define __USE_GNU 1
#include <fcntl.h>
#include "data_set.h"
#include "data_cache.h"
#include "vector.h"
#include "chunk.h"

#define DEFAULT_INS_LEN  1024
#define DEFAULT_INS_NUM  2048
#define DEFAULT_RANDOM_WAY 1
#define DEFAULT_FILE_LEN 1024*1024

long long  data_set_config_instance_len = DEFAULT_INS_LEN;
long long  data_set_config_instance_num = DEFAULT_INS_NUM;

long long  data_set_config_random = DEFAULT_RANDOM_WAY;
long long  data_set_config_file_len = DEFAULT_FILE_LEN;

long long  data_set_config_cache_unit_len = 1024*1024;

long data_set_config_map_address = 0;
long long data_set_config_map_read_start = -1;
long long data_set_config_map_read_len = -1;

struct data_set_file*  get_data_set_file_list()
{
	long long data_set_size = data_set_config_instance_len *data_set_config_instance_num;
	long long data_set_file_len = data_set_config_file_len;
	long long data_instance_num_per_file = 0;
	long long data_file_len = 0;
	int data_file_num = 0;
	

	int i = 0;
	struct data_set_file *head,*pre,*cur;
	char instance_num[16];
	char instance_len[16];

	if(data_set_size == 0 || data_set_file_len == 0)
	{
		return NULL;
	}
	/* calc data set file num*/
	data_instance_num_per_file = data_set_config_file_len / data_set_config_instance_len;
	data_file_len = data_instance_num_per_file * data_set_config_instance_len;
	data_file_num = data_set_config_instance_num / data_instance_num_per_file;
	
	if(data_set_config_instance_num % data_instance_num_per_file)
	{
		data_file_num++;
	}

	printf("data file num is %d\r\n",data_file_num);

	head = cur = pre = NULL;
	for(i = 0 ; i< data_file_num; i++)
	{
		cur = malloc(sizeof(struct data_set_file));
		if(cur != NULL)
		{
			memset(cur,0,sizeof(struct data_set_file));
			sprintf(instance_num,"%d",data_set_config_instance_num);
			sprintf(instance_len,"%d",data_set_config_instance_len);

			sprintf(cur->set_name,"test_case_%s_%s_id%d",instance_num,instance_len,i);
			printf("data set file namae is %s\r\n",cur->set_name);	
			if(i == (data_file_num -1))
			{
				cur->set_len = data_set_size - (data_file_len*i) ;
				cur->set_num = data_set_config_instance_num - (data_instance_num_per_file*i);
			}
			else
			{
				cur->set_len = data_file_len;
				cur->set_num = data_instance_num_per_file;
			}
			printf("data set file len is %d\r\n",cur->set_len);
			printf("data set file num is %d\r\n",cur->set_num);
			
			cur->set_index = i;
			cur->set_file_status = FILE_STATUS_NULL;
			cur->next = NULL;
			
			if(NULL == head)
			{
				head = pre = cur;
			}
			else
			{
				pre->next = cur;
				pre = cur;
			}
			printf("cur data set file pointer is %p\r\n",cur);
		}
		else
		{
			goto alloc_fail;
		}
	}
	return head;

alloc_fail:
	while(head)
	{
		cur = head;
		head = head->next;
		free(cur);
	}
	return NULL;

}

int  get_random_instance(int dev_id,void * instance_mem,int instance_size)
{
	int bytes_to_read = instance_size;
	int bytes_read = 0;
	int bytes_total_read = 0;
	if(!instance_mem || instance_size <= 0)
	{
		return -1;
	}

	do
	{
		bytes_read = read(dev_id,instance_mem + bytes_total_read, bytes_to_read);
		if(-1 == bytes_read)
		{
			return -1;
		}
		bytes_to_read = bytes_to_read - bytes_read;
		bytes_total_read += bytes_read;
		//printf("bytes to read is %d\r\n",bytes_to_read);	
	}while(bytes_to_read);
	
	return 0;
}

int construct_data_set(struct data_set_file *list)
{
	FILE *stream = NULL;
	int i = 0;
	void *instance_mem = NULL;
	int instance_size = 0;
	struct data_set_file *cur = NULL;
	int dev_random_id = -1;
	if(NULL == list){
		return -1;
	}

	dev_random_id = open("/dev/urandom",'r');
	if(-1 == dev_random_id){
		return -1;
	}

	cur = list;
	while(cur)
	{
		printf("cur data set file pointer is %p\r\n",cur);
		if(access(cur->set_name,F_OK)== 0){
			printf("data set file exist %s\r\n",cur->set_name);
			if(data_set_config_random != DEFAULT_RANDOM_WAY)
			{
				cur->set_file_status = FILE_STATUS_DATA;
				printf("use exist data set\r\n");
				cur = cur->next;
				continue;
			}
			else
			{
				remove(cur->set_name);
			}
		}
		else
		{
			if(data_set_config_random != DEFAULT_RANDOM_WAY)
			{
				close(dev_random_id);
				return -1;
			}
		}
		
		stream = fopen(cur->set_name,"wb");
		if(stream)
		{
			instance_size = cur->set_len/cur->set_num;	
			instance_mem = malloc(instance_size);
			printf("instance size is %d\r\n",instance_size);
			if(!instance_mem){
				close(dev_random_id);
				fclose(stream);
				return -1;
			}
			for(i = 0 ; i < cur->set_num ; i++)
			{
#if 1
				if(-1 == get_random_instance(dev_random_id,instance_mem,instance_size))
				{
					close(dev_random_id);
					fclose(stream);
					free(instance_mem);
					return -1;
				}	
#endif
				fwrite(instance_mem,instance_size,1,stream);
			}

			cur->set_file_status = FILE_STATUS_DATA;
			
			fclose(stream);
			free(instance_mem);
		}
		else
		{
			close(dev_random_id);
			return -1;
		}
		cur = cur->next;	
	
	}
	
	close(dev_random_id);
	return 0;
}


void *map_data_set_file_shared(struct data_set_file *flist,long start_addr)
{
	struct data_set_file *cur = NULL;
	int start = 0;
	int fd = -1;
	long long map_len;
	void *map_addr = NULL;
	void *begin = NULL;
	
	void *hope_addr = (void*)start_addr;
	if(NULL == flist)
	{
		return NULL;
	}
	cur = flist ;

	while(cur)
	{
		if(FILE_STATUS_DATA != cur->set_file_status)
		{
			goto release_src;
		}

		fd = open(cur->set_name,O_RDWR|O_DIRECT);
		if(-1 == fd)
		{
			goto release_src;
		}
			
		map_addr  = mmap(hope_addr ,cur->set_len,PROT_READ,MAP_SHARED,fd,0);
		printf("map addr is %p\r\n",map_addr);
		printf("hope addr is %p\r\n",hope_addr);
		if((NULL == map_addr) || (-1 == (long)map_addr))
		{
			close(fd);
			goto release_src;
		}
		
		if((NULL != hope_addr) && (hope_addr != map_addr))
		{
			close(fd);
			goto release_src;
		}
		if(NULL == begin)
		{
			begin = map_addr;
		}

		hope_addr = map_addr + cur->set_len;
		map_len  += cur->set_len;		
		cur->file_fd = fd;
		cur->set_file_status = FILE_STATUS_MAP;
		cur = cur->next;
	}

	return begin;
release_src:
	if(begin)
	{
		munmap(begin,map_len);
	}
	cur = flist;
	while(cur)
	{
		if(cur->set_file_status == FILE_STATUS_MAP)
		{
			close(cur->file_fd);
		}
		cur = cur->next;
	}

	return NULL;
}

void*  map_data_set_file_anonymous(struct data_set_file *flist,long start_addr)
{
	long long data_set_len = 0;
	struct data_set_file *cur = flist;
	void *map_addr;

	if(NULL == flist)
	{
		return NULL;
	}
	while(cur)
	{
		data_set_len += cur->set_len;
		cur = cur->next;
	}

	if(!data_set_len)
	{
		return NULL;
	}

	printf("data set len is %d\r\n",data_set_len);
	map_addr = mmap(start_addr ,data_set_len ,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);
	
	printf("map_addr is %p\r\n",map_addr);
	
	if(map_addr == start_addr)
	{
		cur = flist;
		while(cur)
		{
			cur->file_fd = -1;	
			cur = cur->next;
		}
		return map_addr;
	}

	return NULL;
}

void get_data_file(struct data_set_file *f, long long off,void *start,long long read_len)
{
	long long len = read_len;
	long long i = 0;

	printf("fd is %d ,off is %lld,start is 0x%p,read len is %lld\r\n",f->file_fd,off,start,read_len);
	if(-1 != f->file_fd)/*map shared*/
	{
		for(i = 0 ; i < read_len ; i++)
		{
			*(start+i);
		}

	}
	else /*map anon*/
	{	
		int fd = open(f->set_name,O_RDWR|O_DIRECT);
		int r_cnt = 0;
		if(-1 == fd)
		{
			printf("err in get data file \r\n");	
		}
			
		lseek(fd,off,SEEK_SET);
		r_cnt =read(fd,start,read_len);
		printf("r_cnt is %d\r\n",r_cnt);
		if(r_cnt == -1)
		{
			perror("read:");
		}
		close(fd);
	}
	
	return;

}

void get_data_from_file(struct data_set_file *flist,long long start_off,long long len)
{
	long map_start = data_set_config_map_address ;
	long long start = start_off;
	long long read_len = 0;
	struct data_set_file *cur = flist;
	printf("start off is %lld,len is %lld\r\n",start_off,len);	
	while(cur)
	{
		if(cur->set_len <= start)
		{
			start = start - cur->set_len;
		}
		else
		{
			read_len = cur->set_len - start;
			if(-1 == len)
			{
				get_data_file(cur,start,map_start+start_off,read_len);
				start_off +=  read_len;
			}
			else
			{
				if(read_len <= len)
				{
					len = len - read_len;
					get_data_file(cur,start,map_start+start_off,read_len);
					start_off +=  read_len;
				}
				else
				{
					if(len != 0){
						get_data_file(cur,start,map_start+start_off,len);
					}
					break;
				}
			}
			start = 0;

		}
		cur = cur->next;
	}
			
	return ;
}
extern void test_insert_data(char *pdata);
extern int test_delete_data(char *pdata);
void test_insert_proc(void *args)
{
	struct data_set_cache *cur = NULL;
	struct data_set_cache *next = NULL;
	void *data = NULL;
	int insert_cnt = 0;
	unsigned long long per_cache_time_begin = 0;	
	unsigned long long per_cache_time_end = 0;	
	unsigned long long total_time_begin = 0;	
	unsigned long long total_time_end = 0;	
	
	do {
		next = get_next_data_set_cache(cur);		
		if(NULL == next)
		{
			break;
		}
		per_cache_time_begin = rdtsc();
		while(data = get_next_data(next))
		{
			//printf("data addr is 0x%p\r\n",data);
			insert_cnt++;	
			test_insert_data(data);
		}
		per_cache_time_end = rdtsc();
		printf("per cache insert cost: %lld\r\n", per_cache_time_end - per_cache_time_begin);
		printf("insert data num is %d\r\n",insert_cnt);
		if(cur)
		{
			free(cur);
		}
		cur = next;
		sleep(0);
	}while(cur);
	printf("insert over\r\n");

}

void test_delete_proc(void *args)
{
	struct data_set_cache *cur = NULL;
	struct data_set_cache *next = NULL;
	void *data = NULL;
    int ret;
	int delete_cnt = 0;
	unsigned long long per_cache_time_begin = 0;	
	unsigned long long per_cache_time_end = 0;	
	unsigned long long total_time_begin = 0;	
	unsigned long long total_time_end = 0;	
	do {
		next = get_next_data_set_cache(cur);		
		if(NULL == next)
		{
			break;
		}

		per_cache_time_begin = rdtsc();
        spt_thread_start(g_thrd_id);
		while(data = get_next_data(next))
		{
			delete_cnt++;
            if(delete_cnt%100 == 0)
            {
                spt_thread_exit(g_thrd_id);
                spt_thread_start(g_thrd_id);
            }
try_again:
			if(test_delete_data(data) < 0)
            {
                ret = spt_get_errno();
                if(ret == SPT_MASKED)
                {
                    spt_thread_exit(g_thrd_id);
                    spt_thread_start(g_thrd_id);
                    goto try_again;
                }
                else if(ret == SPT_WAIT_AMT)
                {
                    spt_thread_exit(g_thrd_id);
                    spt_thread_start(g_thrd_id);
                    goto try_again;
                }
                else if(ret == SPT_NOMEM)
                {
                    printf("OOM,%d\t%s\r\n", __LINE__, __FUNCTION__);
                    spt_thread_exit(g_thrd_id);
                    break;
                }
                else
                {
                    printf("DELETE ERROR[%d],%d\t%s\r\n", ret,__LINE__, __FUNCTION__);
                    spt_thread_exit(g_thrd_id);
                    break;
                }
            }         
		}
		
		per_cache_time_end = rdtsc();
		printf("per cache delete cost: %lld\r\n", per_cache_time_end - per_cache_time_begin);
		printf("delete data num is %d\r\n",delete_cnt);
		
		if(cur)
		{
			free(cur);
		}
		cur = next;
		sleep(0);
	}while(cur);
	
	printf("delete over\r\n");
}
