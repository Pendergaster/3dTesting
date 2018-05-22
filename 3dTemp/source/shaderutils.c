//#include <ycm_helper.h>
typedef struct
{

	uint		progId;
	int		numAttribs;
} ShaderHandle;

enum
{
	SHA_PROG_UV = 0,
	SHA_PROG_NO_UV,
	LIGHT,
	DEBUG_PROG,
	skyboxShader,
	frameShader,
	BlurShader,
	ParticleProg,
	PokemonProg,
	maxshaderprogs

};

static ShaderHandle shader_cache[maxshaderprogs] = { 0 }; // model uv, model no uv

ShaderHandle* get_shader(int shader)
{
	return &shader_cache[shader];
}

uint compile_shader(uint glenum, const char* source)
{
	int compilecheck = 0;
	int shader = glCreateShader(glenum);
	if (shader == 0) FATALERROR;

	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &(compilecheck));

	if (!compilecheck)
	{
		int infolen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &(infolen));
		if (infolen > 1)
		{
			char* infoLog = malloc(sizeof(char) * infolen);
			glGetShaderInfoLog(shader, infolen, NULL, infoLog);
			printf("Error compiling shader :\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteShader((GLuint)shader);
		FATALERROR;
	}
	return shader;
}
#define INVALIDSHADER 666
uint soft_compile_shader(uint glenum, const char* source)
{
	int compilecheck = 0;
	uint shader = glCreateShader(glenum);
	if (shader == 0) return INVALIDSHADER;

	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &compilecheck);

	if (!compilecheck)
	{
		int infolen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infolen);
		if (infolen > 1)
		{
			char* infoLog = malloc(sizeof(char) * infolen);
			glGetShaderInfoLog(shader, infolen, NULL, infoLog);
			printf("Error compiling shader :\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteShader(shader);
		return INVALIDSHADER;
	}
	return shader;
}

void dispose_shader(ShaderHandle* sha)
{
	if (sha->progId)glDeleteProgram(sha->progId);

	sha->numAttribs = 0;
	sha->progId = 0;
}
void unuse_shader(const ShaderHandle* sha)
{
	glUseProgram(0);
//	for (int i = 0; i < sha->numAttribs; i++) {
//		glDisableVertexAttribArray(i);
//	}
}
void add_attribute(ShaderHandle* shader, const char* name)
{
	printf("Adding attribute %s to program %d \n", name, shader->progId);
	glBindAttribLocation(shader->progId, shader->numAttribs++, name);
}
void set_uniform_int(const ShaderHandle* shader, const char* name, int value)
{
	glUniform1i(glGetUniformLocation(shader->progId, name), value);
}
void set_uniform_float(const ShaderHandle* shader, const char* name, float value)
{
	glUniform1f(glGetUniformLocation(shader->progId, name), value);
}
void set_mat4(const ShaderHandle* sha, const char* name,const float mat[4][4])
{
	int vertexLocation = glGetUniformLocation(sha->progId, name);
	if (vertexLocation == GL_INVALID_INDEX)
	{
		FATALERROR;
	}
	glUniformMatrix4fv(vertexLocation, 1, GL_FALSE, (GLfloat*)mat);
}
void set_vec3(const ShaderHandle* sha, const char* name, const vec3* vec)
{
	int vertexLocation = glGetUniformLocation(sha->progId, name);
	if (vertexLocation == GL_INVALID_INDEX)
	{
		FATALERROR;
	}
	glUniform3f(vertexLocation, vec->x, vec->y, vec->z);
}
void set_vec4(const ShaderHandle* sha, const char* name, const vec4* vec)
{
	int vertexLocation = glGetUniformLocation(sha->progId, name);
	if (vertexLocation == GL_INVALID_INDEX)
	{
		FATALERROR;
	}
	glUniform4f(vertexLocation, vec->x, vec->y, vec->z,vec->w);
}



uint get_uniform_location(ShaderHandle* shader, const char* name)
{
	GLint location = glGetUniformLocation(shader->progId, name);
	if (location == GL_INVALID_INDEX)
	{
		printf("failed to get uniform");
		FATALERROR;
	}
	return location;
}



void use_shader(const ShaderHandle* shader)
{
	glUseProgram(shader->progId);
	//for (int i = 0; i < shader->numAttribs; i++) {
//		glEnableVertexAttribArray(i);
//	}
}

void link_shader(ShaderHandle* shader, uint vert, uint frag)
{
	printf("Linking program %d\n", shader->progId);
	//shader->progId = glCreateProgram();
	glLinkProgram(shader->progId);
	int linked = 0;
	glGetProgramiv(shader->progId, GL_LINK_STATUS, &linked);

	if (!linked)
	{
		int infolen = 0;
		glGetProgramiv(shader->progId, GL_INFO_LOG_LENGTH, &infolen);
		if (infolen > 1)
		{
			char* infolog = malloc(sizeof(char)*infolen);
			glGetProgramInfoLog(shader->progId, infolen, NULL, infolog);
			printf("Error linking program \n %s", infolog);
			free(infolog);
		}
		glDeleteProgram(shader->progId);
		FATALERROR;
	}

	glDeleteShader(vert);
	glDeleteShader(frag);
}
uint soft_link_shader(ShaderHandle* shader, uint vert, uint frag)
{
	printf("Linking program %d\n", shader->progId);
	//shader->progId = glCreateProgram();
	glLinkProgram(shader->progId);
	int linked = 0;
	glGetProgramiv(shader->progId, GL_LINK_STATUS, &linked);

	if (!linked)
	{
		int infolen = 0;
		glGetProgramiv(shader->progId, GL_INFO_LOG_LENGTH, &infolen);
		if (infolen > 1)
		{
			char* infolog = malloc(sizeof(char)*infolen);
			glGetProgramInfoLog(shader->progId, infolen, NULL, infolog);
			printf("Error linking program \n %s", infolog);
			free(infolog);
		}
		glDeleteProgram(shader->progId);
		return 0;
	}

	glDeleteShader(vert);
	glDeleteShader(frag);
	return 1;
}



//ShaderHandle create_shader_stof(const int numAttribs,const int vertfile,const int fragfile ,...)
//{
//	ShaderHandle shader = { 0 };
//	char* vert_s = load_file(vertfile, NULL);
//	uint vertID = compile_shader(GL_VERTEX_SHADER, vert_s);
//	free(vert_s);
//
//	char* frag_s = load_file(fragfile, NULL);
//	uint fragID = compile_shader(GL_FRAGMENT_SHADER, frag_s);
//	free(frag_s);
//	shader.progId = glCreateProgram();
//	glAttachShader(shader.progId, vertID);
//	glAttachShader(shader.progId, fragID);
//
//
//
//	va_list args;
//
//
//	va_start(args,numAttribs);
//	
//	char * s = va_arg(args, char *);
//	/*puts(s);*/
//	// vsnprintf(buffer, sizeof buffer, "%s", args);
//	va_end(args);
//	//FlushFunnyStream(buffer);
//
//
//	for(uint i = 0; i <numAttribs;i++)
//	{
//		
//		add_attribute(&shader, "ree");
//	}
//
//	link_shader(&shader, vertID, fragID);
//
//	use_shader(&shader);
//	unuse_shader(&shader);
//	return shader;
//}
