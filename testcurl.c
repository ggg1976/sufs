#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sufs.h"
#include "curlapi.h"



int main()
{
	//
	Curl_Data * curldata = curl_new_data();
	if(curldata==NULL)
	{
		curl_delete_data(curldata);
		printf("memory error.\n");
		return -1;
	}
	int ret;
	curldata->bHeadValid = 1;
	ret = curlapi_authv1("X-storage-user: test:tester","X-storage-pass: testing","http://192.168.8.162:8080/auth/v1.0",curldata);
	printf("auth:%d\n",ret);
	curl_print_data(curldata);
	//ret = curlapi_get_account_data("X-Auth-Token: AUTH_tkc0e30afe963f4d97b041f258589df6eb","http://192.168.8.162:8080/v1/AUTH_test",);
	
	
	//char * token,char * url,int limit,char * marker,char * end_marker,Curl_Data * curldata
	
	curl_delete_data(curldata);
	return 0;
}




