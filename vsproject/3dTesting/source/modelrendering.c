
vec3 cubepositions[5] = {
{ 0.f, 0.f , 0.f },
{ 3.f, 0.f , 0.f },
{ 6.f, 0.f , 0.f },
{ 8.f, 0.f , 0.f },
{ 10.f, 0.f , 0.f }
};

typedef struct
{
	uint			noTex;
	uint			withTex;
	//uint			VertBO;
	//uint			UvBO;
	//uint			NormBO;
	//uint			VaoTex;
	//uint			VaoNoTex;
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
	{
		/* with texture*/
		LASTWRITES[frag_sha] = Win32GetLastWriteTime(txt_file_names[frag_sha]);
		LASTWRITES[vert_sha] = Win32GetLastWriteTime(txt_file_names[vert_sha]);
		ShaderHandle* shader = get_shader(SHA_PROG_UV);// &rend->withTex;
		rend->withTex = SHA_PROG_UV;
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
	}


	/* no texture*/
	{
		LASTWRITES[model_frag] = Win32GetLastWriteTime(txt_file_names[model_frag]);
		LASTWRITES[model_vert] = Win32GetLastWriteTime(txt_file_names[model_vert]);
		ShaderHandle* shader = get_shader(SHA_PROG_NO_UV);// &rend->withTex;
		rend->noTex = SHA_PROG_NO_UV;
		char* vert_s = load_file(model_vert, NULL);
		uint vertID = compile_shader(GL_VERTEX_SHADER, vert_s);
		free(vert_s);

		char* frag_s = load_file(model_frag, NULL);
		uint fragID = compile_shader(GL_FRAGMENT_SHADER, frag_s);
		free(frag_s);
		shader->progId = glCreateProgram();
		glAttachShader(shader->progId, vertID);
		glAttachShader(shader->progId, fragID);

		add_attribute(shader, "vertexPosition");
		add_attribute(shader, "normal");

		link_shader(shader, vertID, fragID);

		use_shader(shader);
		/*vec3 te = { 0 };
		set_vec3(shader, "material.diffuse", &te);*/
		unuse_shader(shader);
	}

	//uint VertBo,NormBo,UvOB, VAOtex,vaoNoTex;
	//glGenBuffers(1, &VertBo);
	//glGenBuffers(1, &NormBo);
	//glGenBuffers(1, &UvOB);
	//glGenVertexArrays(1, &VAOtex);
	//glGenVertexArrays(1, &vaoNoTex);

	//glCheckError();

	///* no texture*/
	//glBindVertexArray(vaoNoTex);

	//glBindBuffer(GL_ARRAY_BUFFER, VertBo);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	//glBindBuffer(GL_ARRAY_BUFFER, NormBo);
	//glEnableVertexAttribArray(1);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	////glBindBuffer(GL_ARRAY_BUFFER, 0);

	//glBindVertexArray(0);

	//glCheckError();
	///* with texture*/
	//glBindVertexArray(VAOtex);

	//glBindBuffer(GL_ARRAY_BUFFER, VertBo);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);

	//glBindBuffer(GL_ARRAY_BUFFER, UvOB);
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(1);

	//glBindBuffer(GL_ARRAY_BUFFER, NormBo);
	//glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(2);

	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	//glBindVertexArray(0);

	//glCheckError();

	ShaderHandle* withTex = get_shader(rend->withTex);
	ShaderHandle* noTex = get_shader(rend->noTex);
	rend->modelLOCtex = glGetUniformLocation(withTex->progId, "model");
	rend->viewLOCtex = glGetUniformLocation(withTex->progId, "view");
	rend->projectionLOCtex = glGetUniformLocation(withTex->progId, "projection");

	rend->modelLOCnoTex = glGetUniformLocation(noTex->progId, "model");
	rend->viewLOCnoTex = glGetUniformLocation(noTex->progId, "view");
	rend->projectionLOCnoTex = glGetUniformLocation(noTex->progId, "projection");

	//rend->VaoNoTex = vaoNoTex;
	//rend->VaoTex = VAOtex;

	//rend->UvBO = UvOB;
	//rend->VertBO = VertBo;
	//rend->NormBO = NormBo;

	assert(!(rend->modelLOCtex == GL_INVALID_INDEX || rend->viewLOCtex == GL_INVALID_INDEX || rend->projectionLOCtex == GL_INVALID_INDEX || rend->modelLOCnoTex == GL_INVALID_INDEX ||
		rend->viewLOCnoTex == GL_INVALID_INDEX || rend->viewLOCnoTex == GL_INVALID_INDEX || rend->projectionLOCnoTex == GL_INVALID_INDEX));
}



inline void render_models(const Renderer *rend,const renderData* data,const uint numData,const EngineCamera* camera,const LightValues light)
{
	if (numData == 0)return;
	ShaderHandle* withTex = get_shader(rend->withTex);
	use_shader(withTex);
	set_vec3(withTex, "ViewPos", &camera->cameraPos);
	//glBindVertexArray(m->vao);

	//set light values
	set_vec3(withTex, "light.position", &light.position);
	set_vec3(withTex, "light.ambient", &light.ambient);
	set_vec3(withTex, "light.diffuse", &light.diffuse);
	set_vec3(withTex, "light.specular", &light.specular);
	glCheckError();

	set_uniform_float(withTex, "light.constant", light.constant);
	set_uniform_float(withTex, "light.linear", light.linear);
	set_uniform_float(withTex, "light.quadratic", light.quadratic);

	vec3 dir = { -0.2f, -1.0f, -0.3f };
	vec3 ambient = { 0.5f, 0.5f, 0.5f };
	vec3 diff = { 0.4f, 0.4f, 0.4f };
	vec3 spec = { 0.5f, 0.5f, 0.5f };
	set_vec3(withTex, "glight.direction", &dir);
	set_vec3(withTex, "glight.ambient", &ambient);
	set_vec3(withTex, "glight.diffuse", &diff);
	set_vec3(withTex, "glight.specular", &spec);
	set_uniform_int(withTex, "material.diffuse", 0);

	glUniformMatrix4fv(rend->viewLOCtex, 1, GL_FALSE, (GLfloat*)camera->view.mat);

	mat4 projection = { 0 };

	perspective(&projection, deg_to_rad(fov), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 100.f);
	glUniformMatrix4fv(rend->projectionLOCtex, 1, GL_FALSE, (GLfloat*)projection.mat);

	glActiveTexture(GL_TEXTURE0);

	for (uint i = 0; i < numData; i++)
	{
		mat4 model = { 0 };
		identity(&model);
		translate_mat4(&model, &model, data[i].position);
		rotate_mat4_Z(&model, data[i].Rotation.z/*deg_to_rad(data[i].Rotation.z)*/);
		rotate_mat4_Y(&model, data[i].Rotation.y/*deg_to_rad(data[i].Rotation.y)*/);
		rotate_mat4_X(&model, data[i].Rotation.x/*deg_to_rad(data[i].Rotation.x)*/);
		scale_mat4(&model, data[i].scale);

		glCheckError();
		set_vec3(withTex, "material.specular", &data[i].material.specular);
		set_uniform_float(withTex, "material.shininess", data[i].material.shininess);
		glCheckError();

		glUniformMatrix4fv(rend->modelLOCtex, 1, GL_FALSE, (GLfloat*)model.mat);

		glBindTexture(GL_TEXTURE_2D, textureCache[ data[i].material.diffuse].ID);
		glActiveTexture(GL_TEXTURE0);
		ModelHandle* m = &model_cache[data[i].modelId];
		glBindVertexArray(m->vao);
		glDrawArrays(GL_TRIANGLES, 0, m->vertexsize);
	}
	glBindVertexArray(0);
	unuse_shader(withTex);
}


inline void render(Renderer* rend,const int modelID,const vec3 pos, const vec3 rotations, const float scale,Material material,LightValues light,EngineCamera* camera,uint texid)
{
	ModelHandle* m = &model_cache[modelID];

	//käytä texturoitua

	//if (m->texcoordbuffer != NULL && texid != 0)
	//{

		/*glBindBuffer(GL_ARRAY_BUFFER, rend->VertBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * m->vertexsize, NULL, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m->vertexsize, m->vertexbuffer);

		glBindBuffer(GL_ARRAY_BUFFER, rend->NormBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * m->vertexsize, NULL, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m->vertexsize, m->normalbuffer);

		glBindBuffer(GL_ARRAY_BUFFER, rend->UvBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * m->vertexsize, NULL, GL_DYNAMIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m->vertexsize, m->texcoordbuffer);
		glCheckError();*/

		mat4 model = { 0 };
		identity(&model);
		translate_mat4(&model, &model, pos);
		rotate_mat4_Z(&model, deg_to_rad(rotations.z));
		rotate_mat4_Y(&model, deg_to_rad(rotations.y));
		rotate_mat4_X(&model, deg_to_rad(rotations.x));

		scale_mat4(&model, scale);
		ShaderHandle* withTex = get_shader(rend->withTex);
		glBindVertexArray(m->vao);
		use_shader(withTex);

		set_vec3(withTex, "ViewPos", &camera->cameraPos);

		//aseta materiaalit
		//set_vec3(withTex, "material.diffuse", &material.diffuse);
		set_uniform_int(withTex, "material.diffuse", 0);
		glCheckError();
		set_vec3(withTex, "material.specular", &material.specular);
		set_uniform_float(withTex, "material.shininess", material.shininess);
		glCheckError();

		//set light values
		set_vec3(withTex, "light.position", &light.position);
		set_vec3(withTex, "light.ambient", &light.ambient);
		set_vec3(withTex, "light.diffuse", &light.diffuse);
		set_vec3(withTex, "light.specular", &light.specular);
		glCheckError();

		set_uniform_float(withTex, "light.constant", light.constant);
		set_uniform_float(withTex, "light.linear", light.linear);
		set_uniform_float(withTex, "light.quadratic", light.quadratic);

		vec3 dir = { -0.2f, -1.0f, -0.3f };
		vec3 ambient = { 0.05f, 0.05f, 0.05f };
		vec3 diff = { 0.4f, 0.4f, 0.4f };
		vec3 spec = { 0.5f, 0.5f, 0.5f };
		set_vec3(withTex, "glight.direction", &dir);
		set_vec3(withTex, "glight.ambient", &ambient);
		set_vec3(withTex, "glight.diffuse", &diff);
		set_vec3(withTex, "glight.specular", &spec);
		glCheckError();


		glUniformMatrix4fv(rend->viewLOCtex, 1, GL_FALSE, (GLfloat*)camera->view.mat);

		mat4 projection = { 0 };

		perspective(&projection, deg_to_rad(fov), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 100.f);
		glUniformMatrix4fv(rend->projectionLOCtex, 1, GL_FALSE, (GLfloat*)projection.mat);
		glUniformMatrix4fv(rend->modelLOCtex, 1, GL_FALSE, (GLfloat*)model.mat);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureCache[material.diffuse].ID);

		glDrawArrays(GL_TRIANGLES, 0, m->vertexsize);

		unuse_shader(withTex);


#if 0
	}
	// käytä normaalia
	else

	{
	/*	for (int i = 0; i < m->vertexsize; i++)
		{
			printf("%f , %f , %f\n", m->normalbuffer[i].x, m->normalbuffer[i].y, m->normalbuffer[i].z);
		}*/
		//static int test = 0;
		////printf("test %d \n", test++);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);
		//glBindBuffer(GL_ARRAY_BUFFER, rend->VertBO);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * m->vertexsize, NULL, GL_DYNAMIC_DRAW);
		//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m->vertexsize, m->vertexbuffer);

		//glBindBuffer(GL_ARRAY_BUFFER, rend->NormBO);
		//glCheckError();
		//glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * m->vertexsize, NULL, GL_DYNAMIC_DRAW);
		//glCheckError();
		//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m->vertexsize, m->normalbuffer);
		//glCheckError();
		mat4 model = { 0 };
		identity(&model);
		translate_mat4(&model, &model, pos);
		rotate_mat4_Z(&model, deg_to_rad(rotations.z));
		rotate_mat4_Y(&model, deg_to_rad(rotations.y));
		rotate_mat4_X(&model, deg_to_rad(rotations.x));

		scale_mat4(&model, scale);
		ShaderHandle* noTex = get_shader(rend->noTex);
		glBindVertexArray(m->vao);
		use_shader(noTex);

		set_vec3(noTex, "ViewPos", &camera->cameraPos);
		glCheckError();

		//printf("CAMPOS %f , %f , %f\n", camera->cameraPos.x, camera->cameraPos.y, camera->cameraPos.z);
		//aseta materiaalit
		set_vec3(noTex, "material.diffuse", &material.diffuse);		//väri vec3
		glCheckError();

		set_vec3(noTex, "material.specular", &material.specular);	
		set_uniform_float(noTex, "material.shininess", material.shininess);
		glCheckError();

		//set light values
		set_vec3(noTex, "light.position", &light.position);
		set_vec3(noTex, "light.ambient", &light.ambient);
		set_vec3(noTex, "light.diffuse", &light.diffuse);
		set_vec3(noTex, "light.specular", &light.specular);
		glCheckError();

		set_uniform_float(noTex, "light.constant", light.constant);
		set_uniform_float(noTex, "light.linear", light.linear);
		set_uniform_float(noTex, "light.quadratic", light.quadratic);
		glCheckError();

		vec3 dir = { -0.2f, -1.0f, -0.3f };
		vec3 ambient = { 0.05f, 0.05f, 0.05f };
		vec3 diff = { 0.4f, 0.4f, 0.4f };
		vec3 spec = { 0.5f, 0.5f, 0.5f };
		set_vec3(noTex, "glight.direction", &dir);
		set_vec3(noTex, "glight.ambient", &ambient);
		set_vec3(noTex, "glight.diffuse", &diff);
		set_vec3(noTex, "glight.specular", &spec);
		glCheckError();

		

		glUniformMatrix4fv(rend->viewLOCnoTex, 1, GL_FALSE, (GLfloat*)camera->view.mat);

		mat4 projection = { 0 };

		perspective(&projection, deg_to_rad(fov), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 100.f);
		glUniformMatrix4fv(rend->projectionLOCnoTex, 1, GL_FALSE, (GLfloat*)projection.mat);
		glUniformMatrix4fv(rend->modelLOCnoTex, 1, GL_FALSE, (GLfloat*)model.mat);
	/*	glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);*/

		glDrawArrays(GL_TRIANGLES, 0, m->vertexsize);

		unuse_shader(noTex);

	}
#endif
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

inline void render_boxes(ShaderHandle* handle, uint VBO, uint VAO, uint projectionLOC, uint modelLoc, uint viewLOC, vec3 lightpos, EngineCamera* camera, mat4* projection)
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


	set_vec3(handle, "material.specular", &cube.diffuse);
	set_uniform_float(handle, "material.shininess", cube.shininess);



	LightValues pro = { 0 };
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
inline void render_light(Light light,EngineCamera* camera, mat4* projection, vec3 lightpos)
{
#if 0
	ShaderHandle* s = get_shader(light.shader);
	//glBindBuffer(GL_ARRAY_BUFFER, light.vbo);
	//glBufferData(GL_ARRAY_BUFFER,/* sizeof(verticesBOX)*/sizeof(vec3) * TeaPot.vertexsize, NULL, GL_STATIC_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, 0,/* sizeof(verticesBOX)*/sizeof(vec3) * TeaPot.vertexsize, TeaPot.vertexbuffer/*verticesBOX*/);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	use_shader(s);

	set_mat4(s, "view", camera->view.mat);
	set_mat4(s, "projection", projection->mat);
	vec3 lightColor = { 1.f,0.5f,1.f };
	set_vec3(s, "lightColor", &lightColor);

	glBindVertexArray(light.model.vao);
	glCheckError();
	static float x = 0;
	//x += 0.01f;
	static float y = 0;
	y += 0.1f;
	vec3 rotations = { 0,0,x };
	



	mat4 model = { 0 };
	identity(&model);
	translate_mat4(&model, &model, lightpos);
	rotate_mat4_Z(&model, deg_to_rad(rotations.z));
	rotate_mat4_Y(&model, deg_to_rad(rotations.y));
	rotate_mat4_X(&model, deg_to_rad(rotations.x));

	scale_mat4(&model, 0.5f);

	//rotate_mat4(&model, &model, axis,/*(float)glfwGetTime()*/1 * deg_to_rad(i * 10));
	glUniformMatrix4fv(glGetUniformLocation(s->progId, "model"), 1, GL_FALSE, (GLfloat*)model.mat);
	glDrawArrays(GL_TRIANGLES, 0,TeaPot.vertexsize/* 36*/);

	//render_model(&TeaPot, lightpos, rotations, 0.2, glGetUniformLocation(light.shader.progId, "model"));
	glBindVertexArray(0);

	unuse_shader(s);
#endif
}
vec3 modelNormLocsTEMP[4] =
{
	{ 0.f, 0.f, 1.f },
{ 0.f, 0.f, 3.f },
{ 0.f, 0.f, 6.f },
{ 0.f, 0.f, 7.f },
};
inline void render_model_normals(uint VBO, uint normalbuffer, ShaderHandle* handle, ModelHandle* model, EngineCamera* camera, vec3 lightpos, uint viewLOC, uint projectionLOC, uint modelLoc, uint VAO, mat4* projection)
{
	//bind vertexes
#if 0
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


	Material cube = { 0 };
	vec3 amb = { 0.9f, 0.9f, 0.9f };
	vec3 diff = { 0.9f, 0.9f, 0.9f };
	vec3 spec = { 0.9f, 0.9f, 0.9f };
	float shine = 12.0f;
//	cube.ambient = amb;
	cube.diffuse = diff;
	cube.specular = spec;
	cube.shininess = shine;
	//set_vec3(&shader, "material.ambient", &cube.ambient);
	//set_vec3(&shader, "material.diffuse", &cube.diffuse);
	set_vec3(handle, "material.specular", &cube.diffuse);
	set_uniform_float(handle, "material.shininess", cube.shininess);



	LightValues pro = { 0 };
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
#endif
}

//todo push to renderer sort ja älä bindaa vertexejä uudestaan
