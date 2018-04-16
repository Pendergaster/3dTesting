#define GENERICSIZE 10
#define CREATEDYNAMICARRAY(TYPE,NAME) typedef struct{int size;int num;	TYPE* buff; } NAME;

#define INITARRAY(OBJ)\
OBJ.size = GENERICSIZE;\
OBJ.num = 0;\
OBJ.buff = malloc((sizeof *OBJ.buff) * GENERICSIZE);

#define GET_NEW_OBJ(OBJ,BUFF)\
do{																		\
if(OBJ.num + 1 >= OBJ.size){											\
	OBJ.size *= 2;														\
	void* temp = OBJ.buff;												\
	OBJ.buff = realloc(OBJ.buff, (sizeof *OBJ.buff) * OBJ.size);		\
	if (!OBJ.buff){														\
		OBJ.buff = temp;												\
		OBJ.size /= 2;													\
		BUFF = NULL;													\
		break;															\
	}																	\
}																		\
BUFF = &OBJ.buff[OBJ.num++];											\
}while(0)

#define PUSH_NEW_OBJ(OBJ,PUSH)											\
do{																		\
if(OBJ.num + 1 >= OBJ.size){											\
	OBJ.size *= 2;														\
	void* temp = OBJ.buff;												\
	OBJ.buff = realloc(OBJ.buff, (sizeof *OBJ.buff) * OBJ.size);		\
	if (!OBJ.buff){														\
		OBJ.buff = temp;												\
		OBJ.size /= 2;													\
		break;															\
	}																	\
}																		\
OBJ.buff[OBJ.num++] = PUSH;												\
}while(0)

#define GET_NEW_BLOCK(OBJ,BUFF,SIZE)									\
do{																		\
if(OBJ.num + SIZE >= OBJ.size){											\
	OBJ.size = (OBJ.size * 2) + SIZE;									\
	void* temp = OBJ.buff;												\
	OBJ.buff = realloc(OBJ.buff, (sizeof *OBJ.buff) * OBJ.size);		\
	if (!OBJ.buff){														\
		OBJ.buff = temp;												\
		OBJ.size /= 2;													\
		BUFF = NULL;													\
		break;															\
	}																	\
}																		\
BUFF = &OBJ.buff[OBJ.num];												\
OBJ.num += SIZE;														\
}while(0)

#define FAST_DELETE_INDEX(OBJ,TYPE,INDEX) do{ \
TYPE temp = OBJ.buff[--OBJ.num];		  \
OBJ.buff[INDEX] = temp;\
}while(0)

#define POP_ARRAY(OBJ,BUFF) BUFF = &OBJ.buff[--OBJ.num];

#define DISPOSE_ARRAY(OBJ) do{ free(OBJ.buff); OBJ.size =  0; OBJ.num = 0;}while(0);