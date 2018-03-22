#include <string.h>

#define MAX_VER_AMOUNT 30000

typedef struct
{
	vec3*	vertexbuffer;
	vec3*	normalbuffer;
	vec2*	texcoordbuffer;
	uint	vertexsize;
} ModelHandle;

ModelHandle model_cache[maxmodelfiles] = { 0 };

static vec3* vertexes = NULL;
static vec3* normals = NULL;
static vec2* texturecoords = NULL;
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
		vertexes = malloc(sizeof(vec3)*MAX_VER_AMOUNT + sizeof(vec2)*MAX_VER_AMOUNT + sizeof(vec3)*MAX_VER_AMOUNT +  sizeof(int) * MAX_VER_AMOUNT);
		texturecoords = (vec2*)(vertexes + MAX_VER_AMOUNT);
		normals = (vec3*)(texturecoords + MAX_VER_AMOUNT);
		IndexBuffer = (int*)(normals + MAX_VER_AMOUNT);
	}

	int vertexesSize = 0;
	int textureCoordSize = 0;
	int noramlBufferSize = 0;
	int	indexBufferSize = 0;
	uint hasTexCoords = 0;
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
			hasTexCoords = 1;
			assert(textureCoordSize + 1 < MAX_VER_AMOUNT);
			float temp = 0;
			int matches = fscanf(file, "%f %f %f\n", &texturecoords[textureCoordSize].x, &texturecoords[textureCoordSize].y,&temp);
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
			if(hasTexCoords)
			{
				//dont capture 3rd tex coord
				assert(indexBufferSize + 9 < MAX_VER_AMOUNT);
				int temp = 0;
				// vert, coor, norm
				int matches = fscanf(file, "%d/%d/%d",&IndexBuffer[indexBufferSize], &IndexBuffer[indexBufferSize + 1], &IndexBuffer[indexBufferSize + 2]);
				matches += fscanf(file, "%d/%d/%d", &IndexBuffer[indexBufferSize + 3], &IndexBuffer[indexBufferSize + 4], &IndexBuffer[indexBufferSize + 5]);
				matches += fscanf(file, "%d/%d/%d\n", &IndexBuffer[indexBufferSize + 6], &IndexBuffer[indexBufferSize + 7], &IndexBuffer[indexBufferSize + 8]);

				indexBufferSize += 9;
				if (matches != 9) {
					FATALERROR;
				}
			}
			else
			{
				// no texture coords
				assert(indexBufferSize + 6 < MAX_VER_AMOUNT);
				int matches = fscanf(file, "%d/%d/%d", &IndexBuffer[indexBufferSize], &IndexBuffer[indexBufferSize + 1], &IndexBuffer[indexBufferSize + 2]);
				matches += fscanf(file, "%d/%d/%d", &IndexBuffer[indexBufferSize + 3], &IndexBuffer[indexBufferSize + 4], &IndexBuffer[indexBufferSize + 5]);

				indexBufferSize += 6;
				if (matches != 6) {
					FATALERROR;
				}
			}
		}
		end = fscanf(file, "%s", buff);
	}
	fclose(file);
	// create buffers
	vec3* outVertBuffer = malloc(sizeof(vec3)*indexBufferSize + sizeof(vec3)*indexBufferSize + (hasTexCoords ? sizeof(vec2) * indexBufferSize : 0));
	vec3* outNormalBuffer = outVertBuffer + indexBufferSize;
	vec2* outTextCoordBuffer = (vec2*)(hasTexCoords ? outNormalBuffer + indexBufferSize : NULL);
	int vertsize = 0;
	int uvsize = 0;
	int normalsize = 0;
	int i = 0;
	while (i < indexBufferSize)
	{
		int index = IndexBuffer[i++];
		outVertBuffer[vertsize++] = vertexes[index - 1];

		if (hasTexCoords)
		{
			index = IndexBuffer[i++];
			outTextCoordBuffer[uvsize++] = texturecoords[index - 1];
		}

		index = IndexBuffer[i++];
		outNormalBuffer[normalsize++] = normals[index - 1];

		index = IndexBuffer[i++];
		outVertBuffer[vertsize++] = vertexes[index - 1];

		if (hasTexCoords)
		{
			index = IndexBuffer[i++];
			outTextCoordBuffer[uvsize++] = texturecoords[index - 1];
		}

		index = IndexBuffer[i++];
		outNormalBuffer[normalsize++] = normals[index - 1];

		index = IndexBuffer[i++];
		outVertBuffer[vertsize++] = vertexes[index - 1];

		if (hasTexCoords)
		{
			index = IndexBuffer[i++];
			outTextCoordBuffer[uvsize++] = texturecoords[index - 1];
		}

		index = IndexBuffer[i++];
		outNormalBuffer[normalsize++] = normals[index - 1];









	/*	index = IndexBuffer[i++];
		outVertBuffer[vertsize++] = vertexes[index - 1];

		index = IndexBuffer[i++];
		outVertBuffer[vertsize++] = vertexes[index - 1];


			index = IndexBuffer[i++];
			outTextCoordBuffer[uvsize++] = texturecoords[index - 1];

			index = IndexBuffer[i++];
			outTextCoordBuffer[uvsize++] = texturecoords[index - 1];

		index = IndexBuffer[i++];
		outNormalBuffer[normalsize++] = normals[index - 1];		

		index = IndexBuffer[i++];
		outNormalBuffer[normalsize++] = normals[index - 1];*/
	}



	model_cache[model].vertexbuffer = outVertBuffer;
	model_cache[model].normalbuffer = outNormalBuffer;
	model_cache[model].texcoordbuffer = outTextCoordBuffer;
	model_cache[model].vertexsize = vertsize;
	return	model_cache[model];
}

void dispose_model_memory()
{
	free(vertexes);
	for (int i = 0; i < maxmodelfiles; i++)
	{
		//vertex buffer on pää handle modelin muistiin, kaikki muu muisti on sen perässä
		if (model_cache[i].vertexbuffer != NULL)
		{
			free(model_cache[i].vertexbuffer);
		}
	}
}
