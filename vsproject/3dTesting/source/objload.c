#include <string.h>

#define MAX_VER_AMOUNT 30000

typedef struct
{
	vec3*	vertexbuffer;
	vec3*	normalbuffer;
	uint	vertexsize;
} ModelHandle;

ModelHandle model_cache[maxmodelfiles] = { 0 };

static vec3* vertexes = NULL;
static vec3* normals = NULL;
static vec3* texturecoords = NULL;
static int* IndexBuffer = NULL;

ModelHandle load_model(const int model)
{
	if(model_cache[model].vertexbuffer != NULL)
	{
		return model_cache[model];
	}

	int size = 0;
	//char* mod = load_file_from_source(model_file_names[model],&size);
	FILE* file = fopen(/*model_file_names[model]*/ "models/teapot.obj", "r");
	assert(file);
	char buff[255];

	int end = fscanf(file, "%s", buff);

	if(vertexes == NULL)
	{
		vertexes = malloc(sizeof(vec3)*MAX_VER_AMOUNT + sizeof(int) * MAX_VER_AMOUNT + sizeof(vec3)*MAX_VER_AMOUNT + sizeof(vec3)* MAX_VER_AMOUNT);
		texturecoords = (vec3*)(vertexes + MAX_VER_AMOUNT);
		normals = (vec3*)(texturecoords + MAX_VER_AMOUNT);
		IndexBuffer = (int*)(normals + MAX_VER_AMOUNT);
	}

	int vertexesSize = 0;
	int textureCoordSize = 0;
	int noramlBufferSize = 0;
	int	indexBufferSize = 0;
	while (end != EOF)
	{
		if (!strcmp("v", buff))
		{
			assert(vertexesSize + 1 < MAX_VER_AMOUNT);
			int matches = fscanf(file, "%f %f %f\n", &vertexes[vertexesSize].x, &vertexes[vertexesSize].y, &vertexes[vertexesSize].z);
			vertexesSize += 1;
			if (matches != 3) {
				FATALERROR;
			}
		}
		else if(!strcmp("vt", buff))
		{
			assert(textureCoordSize + 1 < MAX_VER_AMOUNT);
			int matches = fscanf(file, "%f %f %f\n", &texturecoords[textureCoordSize].x, &texturecoords[textureCoordSize].y, &texturecoords[textureCoordSize].z);
			textureCoordSize += 1;
			if (matches != 3) {
				FATALERROR;
			}
		}
		else if (!strcmp("vn", buff))
		{
			assert(noramlBufferSize + 1 < MAX_VER_AMOUNT);
			int matches = fscanf(file, "%f %f %f\n", &normals[noramlBufferSize].x, &normals[noramlBufferSize].y, &normals[noramlBufferSize].z);
			noramlBufferSize += 1;
			if (matches != 3) {
				FATALERROR;
			}
		}
		else if (!strcmp("f", buff))
		{
			assert(indexBufferSize + 3 < MAX_VER_AMOUNT);
			int matches = fscanf(file, "%d %d %d\n", &IndexBuffer[indexBufferSize], &IndexBuffer[indexBufferSize + 1], &IndexBuffer[indexBufferSize + 2]);
			//f 2909 2921 2939
			indexBufferSize += 3;
			if (matches != 3) {
				FATALERROR;
			}
		}
		end = fscanf(file, "%s", buff);
	}
	fclose(file);
	// create buffers
	vec3* outVertBuffer = malloc(sizeof(vec3)*indexBufferSize);
	int vertsize = 0;

	for(int i = 0; i < indexBufferSize;i++)
	{
		int index = IndexBuffer[i];
		outVertBuffer[vertsize++] = vertexes[index - 1];
	}
	model_cache[model].vertexbuffer = outVertBuffer;
	model_cache[model].normalbuffer = NULL;
	model_cache[model].vertexsize = vertsize;	
	return	model_cache[model];
}

void dispose_model_memory()
{
	free(vertexes);
	for (int i = 0; i < maxmodelfiles; i++)
	{
		if (model_cache[i].normalbuffer != NULL)
		{
			free(model_cache[i].normalbuffer);
		}
		if (model_cache[i].vertexbuffer != NULL)
		{
			free(model_cache[i].vertexbuffer);
		}
	}
}