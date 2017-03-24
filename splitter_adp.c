/*************************************************************************
	> File Name: splitter_adp.c
	> Author: ma6174
	> Mail: ma6174@163.com 
	> Created Time: Fri 24 Mar 2017 12:18:58 AM PDT
 ************************************************************************/

#include<stdio.h>
#include "vector.h"
#include "chunk.h"

char *tree_get_key_from_data(char *pdata)
{
	return pdata;
}
void tree_free_key(char *key)
{
	return;
}
void tree_free_data(char *p)
{
	return;
}

char *tree_construct_data_from_key(char *pkey)
{
    char *pdata;

    pdata = (char *)malloc(DATA_SIZE);
    if(pdata == NULL)
        return NULL;
    memcpy(pdata, pkey, DATA_SIZE);
    return (char *)pdata;
}

