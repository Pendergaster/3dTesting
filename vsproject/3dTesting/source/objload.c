#include <string.h>

#define MAX_VER_AMOUNT 50000
void load_model(int model,vec3** outVertBuffer, int* vertsize)
{
	int size = 0;
	//char* mod = load_file_from_source(model_file_names[model],&size);
	FILE* file = fopen(/*model_file_names[model]*/"models/teapot.obj", "r");
	assert(file);
	char buff[255];

	int end = fscanf(file, "%s", buff);

	vec3* vertexes = malloc(sizeof(vec3)*MAX_VER_AMOUNT);
	int vertexesSize = 0;

	int* IndexBuffer = malloc(sizeof(int) * MAX_VER_AMOUNT);
	int	indexBufferSize = 0;
	int line = 1;
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
		line++;
	}
	fclose(file);
	// create buffers
	*outVertBuffer = malloc(sizeof(vec3)*MAX_VER_AMOUNT);
	*vertsize = 0;

	for(int i = 0; i < indexBufferSize;i++)
	{
		int index = IndexBuffer[i];
		(*outVertBuffer)[(*vertsize)++] = vertexes[index - 1];
	}
}