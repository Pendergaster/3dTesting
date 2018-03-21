
vec3 cubepositions[5] = {
{ 0.f, 0.f , 0.f },
{ 3.f, 0.f , 0.f },
{ 6.f, 0.f , 0.f },
{ 8.f, 0.f , 0.f },
{ 10.f, 0.f , 0.f }
};

typedef struct
{
	ShaderHandle	noTex;
	ShaderHandle	withTex;
	uint			VertBO;
	uint			UvBO;
	uint			NormBO;
	uint			VaoTex;
	uint			VaoNoTex;
	uint			modelLOCtex;
	uint			viewLOCtex;
	uint			projectionLOCtex;
	uint			modelLOCnoTex;
	uint			viewLOCnoTex;
	uint			projectionLOCnoTex;
	//light stuff
} Renderer;

inline void init_renderer(Renderer *rend)
{
	/* with texture*/
	ShaderHandle* shader = &rend->withTex;
	char* vert_s = load_file(vert_sha, NULL);
	uint vertID = compile_shader(GL_VERTEX_SHADER, vert_s);
	free(vert_s);

	char* frag_s = load_file(frag_sha, NULL);
	uint fragID = compile_shader(GL_FRAGMENT_SHADER, frag_s);
	free(frag_s);
	shader->progId = glCreateProgram();
	glAttachShader(shader->progId, vertID);
	glAttachShader(shader->progId, fragID);

	add_attribute(shader, "vertexPosition");
	add_attribute(shader, "uv");
	add_attribute(shader, "normal");


	link_shader(shader, vertID, fragID);

	use_shader(shader);
	unuse_shader(shader);


	/* no texture*/

	shader = &rend->noTex;
	vert_s = load_file(vert_sha, NULL);
	vertID = compile_shader(GL_VERTEX_SHADER, vert_s);
	free(vert_s);

	frag_s = load_file(frag_sha, NULL);
	fragID = compile_shader(GL_FRAGMENT_SHADER, frag_s);
	free(frag_s);
	shader->progId = glCreateProgram();
	glAttachShader(shader->progId, vertID);
	glAttachShader(shader->progId, fragID);

	add_attribute(shader, "vertexPosition");
	add_attribute(shader, "normal");


	link_shader(shader, vertID, fragID);

	use_shader(shader);
	unuse_shader(shader);

	uint VertBo,NormBo,UvOB, VAOtex,vaoNoTex;
	glGenBuffers(1, (GLuint)VertBo);
	glGenBuffers(1, (GLuint)NormBo);
	glGenBuffers(1, (GLuint)UvOB);
	glGenVertexArrays(1, &VAOtex);
	glGenVertexArrays(1, &vaoNoTex);

	glCheckError();

	/* with texture*/
	glBindVertexArray(VAOtex);

	glBindBuffer(GL_ARRAY_BUFFER, VertBo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, NormBo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glCheckError();
	/* no texture*/
	glBindVertexArray(vaoNoTex);

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

	rend->modelLOCtex = glGetUniformLocation(rend->withTex.progId, "model");
	rend->viewLOCtex = glGetUniformLocation(rend->withTex.progId, "view");
	rend->projectionLOCtex = glGetUniformLocation(rend->withTex.progId, "projection");

	rend->modelLOCnoTex = glGetUniformLocation(rend->noTex.progId, "model");
	rend->viewLOCnoTex = glGetUniformLocation(rend->noTex.progId, "view");
	rend->projectionLOCnoTex = glGetUniformLocation(rend->noTex.progId, "projection");


	assert(!(rend->modelLOCtex == GL_INVALID_INDEX || rend->viewLOCtex == GL_INVALID_INDEX || rend->projectionLOCtex == GL_INVALID_INDEX || rend->modelLOCnoTex == GL_INVALID_INDEX ||
		rend->viewLOCnoTex == GL_INVALID_INDEX || rend->viewLOCnoTex == GL_INVALID_INDEX || rend->projectionLOCnoTex == GL_INVALID_INDEX));
}



inline void render(const int model,const vec3 pos, const vec3 rotations, const float scale)
{
	ModelHandle* m = &model_cache[model];

	//käytä texturoitua
	if(m->texcoordbuffer != NULL)
	{
		
	}
	// käytä normaalia
	else
	{

	}
}


inline void render_model(const ModelHandle* handle, const vec3 pos, const vec3 rotations, const float scale, const int modelLOC)
{


	mat4 model = { 0 };
	identity(&model);
	translate_mat4(&model, &model, pos);
	rotate_mat4_Z(&model, deg_to_rad(rotations.z));
	rotate_mat4_Y(&model, deg_to_rad(rotations.y));
	rotate_mat4_X(&model, deg_to_rad(rotations.x));

	scale_mat4(&model, scale);

	//rotate_mat4(&model, &model, axis,/*(float)glfwGetTime()*/1 * deg_to_rad(i * 10));
	glUniformMatrix4fv(modelLOC, 1, GL_FALSE, (GLfloat*)model.mat);
	glDrawArrays(GL_TRIANGLES, 0, handle->vertexsize);
}

inline void render_boxes(ShaderHandle* handle, uint VBO, uint VAO, uint projectionLOC, uint modelLoc, uint viewLOC, vec3 lightpos, Camera* camera, mat4* projection)
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	use_shader(handle);



	vec3 objcolor = { 1.f,1.0f,1.0f };
	vec3 lightcolor = { 1.f,1.f,1.f };


	set_vec3(handle, "ViewPos", &camera->cameraPos);
	typedef struct
	{
		vec3 ambient;
		vec3 diffuse;
		vec3 specular;
		float shininess;
	} Material;

	Material cube = { 0 };
	vec3 amb = { 0.9f, 0.9f, 0.9f };
	vec3 diff = { 0.9f, 0.9f, 0.9f };
	vec3 spec = { 0.9f, 0.9f, 0.9f };
	float shine = 12.0f;
	cube.ambient = amb;
	cube.diffuse = diff;
	cube.specular = spec;
	cube.shininess = shine;
	//set_vec3(&shader, "material.ambient", &cube.ambient);
	//set_vec3(&shader, "material.diffuse", &cube.diffuse);
	set_vec3(handle, "material.specular", &cube.diffuse);
	set_uniform_float(handle, "material.shininess", cube.shininess);

	typedef struct
	{
		vec3 position;
		vec3 ambient;
		vec3 diffuse;
		vec3 specular;
	} LightP;

	LightP pro = { 0 };
	pro.position = lightpos;
	vec3 ambL = { 0.7f, 0.7f, 0.7f };
	vec3 diffL = { 0.5f, 0.5f, 0.5f };
	vec3 specL = { 1.0f, 1.0f, 1.0f };
	pro.ambient = ambL;
	pro.diffuse = diffL;
	pro.specular = specL;

	vec3 ldir = { -0.2f, -1.0f, -0.3f };
	//set_vec3(&shader, "light.direction", &ldir);
	set_vec3(handle, "light.position", &lightpos);
	set_vec3(handle, "light.ambient", &pro.ambient);
	set_vec3(handle, "light.diffuse", &pro.diffuse);
	set_vec3(handle, "light.specular", &pro.diffuse);

	set_uniform_float(handle, "light.constant", 1.0f);
	set_uniform_float(handle, "light.linear", 0.1f);
	set_uniform_float(handle, "light.quadratic", 0.032f);

	glUniformMatrix4fv(viewLOC, 1, GL_FALSE, (GLfloat*)camera->view.mat);

	perspective(projection, deg_to_rad(fov), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 100.f);
	glUniformMatrix4fv(projectionLOC, 1, GL_FALSE, (GLfloat*)projection->mat);
	glBindVertexArray(VAO);

	mat4 model = { 0 };
	identity(&model);
	vec3 axis = { 1.f,0.f,0.f };
	for (int i = 0; i < 5; i++)
	{
		/*vec3 Rotation = { i * 10, i * 5 , i * 20 };
		render_model(&TeaPot, cubepositions[i], Rotation, 0.5f, modelLOC)*/;
	translate_mat4(&model, &model, cubepositions[i]);
	rotate_mat4(&model, &model, axis,/*(float)glfwGetTime()*/1 * deg_to_rad(0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (GLfloat*)model.mat);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	glBindVertexArray(0);

	unuse_shader(handle);
}
inline void render_light(Light light, Camera* camera, mat4* projection, vec3 lightpos)
{
	glBindBuffer(GL_ARRAY_BUFFER, light.vbo);
	glBufferData(GL_ARRAY_BUFFER,/* sizeof(verticesBOX)*/sizeof(vec3) * TeaPot.vertexsize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0,/* sizeof(verticesBOX)*/TeaPot.vertexsize * sizeof(vec3), TeaPot.vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	use_shader(&light.shader);

	set_mat4(&light.shader, "view", camera->view.mat);
	set_mat4(&light.shader, "projection", projection->mat);
	vec3 lightColor = { 1.f,1.f,1.f };
	set_vec3(&light.shader, "lightColor", &lightColor);

	glBindVertexArray(light.vao);
	glCheckError();
	static float x = 0;
	x += 0.1f;
	static float y = 0;
	y += 0.1f;
	vec3 rotations = { 0,0,x };
	vec3 npooos = { 0 };
	render_model(&TeaPot, lightpos, rotations, 0.2, glGetUniformLocation(light.shader.progId, "model"));
	glBindVertexArray(0);

	unuse_shader(&light.shader);
}
vec3 modelNormLocsTEMP[4] =
{
	{ 0.f, 0.f, 1.f },
{ 0.f, 0.f, 3.f },
{ 0.f, 0.f, 6.f },
{ 0.f, 0.f, 7.f },
};
inline void render_model_normals(uint VBO, uint normalbuffer, ShaderHandle* handle, ModelHandle* model, Camera* camera, vec3 lightpos, uint viewLOC, uint projectionLOC, uint modelLoc, uint VAO, mat4* projection)
{
	//bind vertexes
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*model->vertexsize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3)*model->vertexsize, model->vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//bind normals
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*model->vertexsize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3)*model->vertexsize, model->normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	use_shader(handle);

	vec3 objcolor = { 1.f,1.0f,1.0f };
	vec3 lightcolor = { 1.f,1.f,1.f };


	set_vec3(handle, "ViewPos", &camera->cameraPos);
	typedef struct
	{
		vec3 ambient;
		vec3 diffuse;
		vec3 specular;
		float shininess;
	} Material;

	Material cube = { 0 };
	vec3 amb = { 0.9f, 0.9f, 0.9f };
	vec3 diff = { 0.9f, 0.9f, 0.9f };
	vec3 spec = { 0.9f, 0.9f, 0.9f };
	float shine = 12.0f;
	cube.ambient = amb;
	cube.diffuse = diff;
	cube.specular = spec;
	cube.shininess = shine;
	//set_vec3(&shader, "material.ambient", &cube.ambient);
	//set_vec3(&shader, "material.diffuse", &cube.diffuse);
	set_vec3(handle, "material.specular", &cube.diffuse);
	set_uniform_float(handle, "material.shininess", cube.shininess);

	typedef struct
	{
		vec3 position;
		vec3 ambient;
		vec3 diffuse;
		vec3 specular;
	} LightP;

	LightP pro = { 0 };
	pro.position = lightpos;
	vec3 ambL = { 0.7f, 0.7f, 0.7f };
	vec3 diffL = { 0.5f, 0.5f, 0.5f };
	vec3 specL = { 1.0f, 1.0f, 1.0f };
	pro.ambient = ambL;
	pro.diffuse = diffL;
	pro.specular = specL;

	vec3 ldir = { -0.2f, -1.0f, -0.3f };
	//set_vec3(&shader, "light.direction", &ldir);
	set_vec3(handle, "light.position", &lightpos);
	set_vec3(handle, "light.ambient", &pro.ambient);
	set_vec3(handle, "light.diffuse", &pro.diffuse);
	set_vec3(handle, "light.specular", &pro.diffuse);

	set_uniform_float(handle, "light.constant", 1.0f);
	set_uniform_float(handle, "light.linear", 0.1f);
	set_uniform_float(handle, "light.quadratic", 0.032f);

	glUniformMatrix4fv(viewLOC, 1, GL_FALSE, (GLfloat*)camera->view.mat);

	perspective(projection, deg_to_rad(fov), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 100.f);
	glUniformMatrix4fv(projectionLOC, 1, GL_FALSE, (GLfloat*)projection->mat);
	glBindVertexArray(VAO);



	for (int i = 0; i < 4; i++)
	{
		vec3 Rotation = { 0, 0 ,0 };
		render_model(model, modelNormLocsTEMP[i], Rotation, 0.5f, modelLoc);
	}

	glBindVertexArray(0);

	unuse_shader(handle);
}

//todo push to renderer sort ja älä bindaa vertexejä uudestaan