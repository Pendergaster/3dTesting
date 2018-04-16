#include <string.h>

#define MAX_VER_AMOUNT 30000

typedef struct
{
	uint	vertexsize;
	uint	vao;
	uint	vbo;
	uint	nbo;
	uint	uvbo;
	vec3	nativeScale;
} ModelHandle;




ModelHandle model_cache[maxmodelfiles] = { 0 };

static vec3* vertexes = NULL;
static vec3* normals = NULL;
static vec2* texturecoords = NULL;
static int* IndexBuffer = NULL;



ModelHandle load_model(const int model)
{

	if(model_cache[model].vbo != 0)
	{
		return model_cache[model];
	}
	/*ShaderHandle * s = &shader_cache[vert_sha]*/;
	/*const char* filename = shader_dile*/
	int size = 0;
	//char* mod = load_file_from_source(model_file_names[model],&size);
	FILE* file = fopen(model_file_names[model], "r");
	printf("loading model = %s \n", model_file_names[model]);
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

	float largestX = 0;
	float largestY = 0;
	float largestZ = 0;
	while (end != EOF)
	{
		if (!strcmp("v", buff))
		{
			assert(vertexesSize + 1 < MAX_VER_AMOUNT);
			int matches = fscanf(file, "%f %f %f\n", &vertexes[vertexesSize].x, &vertexes[vertexesSize].y, &vertexes[vertexesSize].z);
			largestX = largestX < abs(vertexes[vertexesSize].x) ? abs(vertexes[vertexesSize].x) : largestX;
			largestY = largestY < abs(vertexes[vertexesSize].y) ? abs(vertexes[vertexesSize].y) : largestY;
			largestZ = largestZ < abs(vertexes[vertexesSize].z) ? abs(vertexes[vertexesSize].z) : largestZ;
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
			int matches = fscanf(file, "%f %f\n", &texturecoords[textureCoordSize].x, &texturecoords[textureCoordSize].y);
			textureCoordSize += 1;
			if (matches != 2) {
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
	int StrideSize = (indexBufferSize / 3);
	vec3* outVertBuffer = malloc(sizeof(vec3)*StrideSize + sizeof(vec3)*StrideSize + (hasTexCoords ? sizeof(vec2) * StrideSize : 0));
	vec3* outNormalBuffer = outVertBuffer + StrideSize;
	vec2* outTextCoordBuffer = (vec2*)(hasTexCoords ? outNormalBuffer + StrideSize : NULL);
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

	}

	uint VertBo, NormBo, vao, UvOB;
	glGenBuffers(1, &VertBo);
	glGenBuffers(1, &NormBo);
	glGenBuffers(1, &UvOB);
	glGenVertexArrays(1, &vao);

	glCheckError();

	/* with texture*/
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, VertBo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, UvOB);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, NormBo);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glCheckError();


	glBindBuffer(GL_ARRAY_BUFFER, VertBo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * vertsize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * vertsize, outVertBuffer);
	glCheckError();


	glBindBuffer(GL_ARRAY_BUFFER, NormBo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) *vertsize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) *vertsize, outNormalBuffer);
	glCheckError();
	
	glBindBuffer(GL_ARRAY_BUFFER, UvOB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) *vertsize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) *vertsize, outTextCoordBuffer);
	glCheckError();

	model_cache[model].vbo = VertBo;
	model_cache[model].nbo = NormBo;
	model_cache[model].vao = vao;
	model_cache[model].uvbo = UvOB;
	model_cache[model].vertexsize = vertsize;
	model_cache[model].nativeScale.x = largestX;
	model_cache[model].nativeScale.y = largestY;
	model_cache[model].nativeScale.z = largestZ;

	free(outVertBuffer); // handle to all memory

	return	model_cache[model];
}

void dispose_model_memory()
{
	free(vertexes);
	//for (int i = 0; i < maxmodelfiles; i++)
	//{
	//	//vertex buffer on pää handle modelin muistiin, kaikki muu muisti on sen perässä
	//	if (model_cache[i].vertexbuffer != NULL)
	//	{
	//		free(model_cache[i].vertexbuffer);
	//	}
	//}
}
