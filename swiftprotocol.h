#ifndef __SWIFT_PROTOCOL_H__
#define __SWIFT_PROTOCOL_H__

#define SWIFT_OBJECT_TYPE_IFERROR	0
#define SWIFT_OBJECT_TYPE_IFDIR 	1
#define SWIFT_OBJECT_TYPE_IFFILE	2

/* authenticate data*/
// Auth data:
typedef struct SWIFT_AUTHENTICATE
{
	int bAuth;		//1 has auth token,0 has no auth to use fs.	
	char strUser[128];
	char strPassword[128];
	char strStorageToken[128];
	char strAuthToken[128];
	char strAuthUrl[256];
	char strUrl[1024];
}Swift_Auth_Data;

/* account data*/
// Container detail info:
typedef struct SWIFT_ACCOUNT_LIST_DETAIL
{
	int nCount;			//object count
	long lBytes;		//total bytes in the container
	char strName[256];	//
}Swift_Account_List_Detail;
// Account metadata:
typedef struct SWIFT_ACCOUNT_METADATA
{
	int nAccountObjectCount;	//
	int nAccountContainerCount;	//
	int nContentLength;			//
	long lAccountBytesUsed;		//
	int nContainterLoc;						//container array size
	Swift_Account_List_Detail * pDetail;	//container array
}Swift_Account_Metadata;


/* container data*/
typedef struct SWIFT_CONTAINER_METADATA
{
	int nContainerObjectCount;
	int bContainerRead;
	long lContainerBytesUsed;
	int nContentLength;
	char strContainerMetaWebIndex[128];
	long tMakeTime;
}Swift_Container_Metadata;

/* object data*/
// Object metadata:
typedef struct SWIFT_OBJECT_METADATA
{
	int nContentLength;
	char strContentType[256];
	long tLastModified;
	long tMakeTime;
	char strEtag[128];
}Swift_Object_Metadata;
// User directory:
typedef struct USER_DIRECTORY
{
	char strCurrentDir[1024];			//such as :"curl/test" exclude of the container.	
	char strFiles[50][1024];	//such as :"1.ppt","jisuanjijieshao.ppt"....
	int nValidCount;			//the count of the files which are validate.
	int nLimit;					//request condition
	char strMarker[256];		//request condition
	char strEndMarker[256];		//request condition
	char strPrefix[256];		//request condition
	char strDelimiter[256];		//request condition
}User_Directory;
//
typedef struct SWIFT_INTERFACE_DATA
{
	Swift_Auth_Data * pAuthData;
	Swift_Account_Metadata * pAccountMetadata;
	Swift_Account_List_Detail * pAccountListDetail;
	Swift_Container_Metadata * pContainerMetadata;
	Swift_Object_Metadata * pObjectMetadata;
	User_Directory * pUserDirectory;
	//FIFO * liCurl;

}Swift_Interface_Data;


//APIs:
/* Auth functions:*/
Swift_Auth_Data * swift_new_auth();
void swift_delete_auth(Swift_Auth_Data * auth);
void swift_auth_init(char * user,char *passwd,char * host,Swift_Auth_Data * auth);
int swiftapi_get_auth(FIFO * li,Swift_Auth_Data * auth);
void swift_print_auth(Swift_Auth_Data * auth);

/*Container functions*/
int swift_get_container_metadata(FIFO * li,Swift_Container_Metadata * metadata);
Swift_Container_Metadata * swift_new_container_metadata();
void swift_delete_container_metadata(Swift_Container_Metadata * data);

Swift_Object_Metadata * swift_new_object_metadata();
void swift_delete_object_metadata(Swift_Object_Metadata * metadata);
int swift_get_object_metadata(FIFO * li,Swift_Object_Metadata * metadata);





#endif
