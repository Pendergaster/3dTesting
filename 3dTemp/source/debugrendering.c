//
//CREATEDYNAMICARRAY(vec3, vert_buffer)
//CREATEDYNAMICARRAY(int, index_buffer)



void init_debugrend(DebugRend* rend)
{
	LASTWRITES[debug_frag] = Win32GetLastWriteTime(txt_file_names[debug_frag]);
	LASTWRITES[debug_vert] = Win32GetLastWriteTime(txt_file_names[debug_vert]);


	char* VERT_SRC = load_file(debug_vert, NULL);
	uint vert = compile_shader(GL_VERTEX_SHADER, VERT_SRC);
	free(VERT_SRC);
	assert(vert);

	char* FRAG_SRC = load_file(debug_frag, NULL);
	uint frag = compile_shader(GL_FRAGMENT_SHADER, FRAG_SRC);
	free(FRAG_SRC);
	assert(frag);

	ShaderHandle* shader = &shader_cache[DEBUG_PROG];
	shader->progId = glCreateProgram();

	add_attribute(shader, "vertexPosition");
	glAttachShader(shader->progId, vert);
	glAttachShader(shader->progId, frag);
	link_shader(shader, vert, frag);

	glGenVertexArrays(1, &rend->vao);
	glGenBuffers(1, &rend->vbo);
	glGenBuffers(1, &rend->ibo);

	assert(rend->vao > 0 || rend->vbo > 0 || rend->ibo > 0);

	glBindVertexArray(rend->vao);
	glBindBuffer(GL_ARRAY_BUFFER, rend->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rend->ibo);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	glBindVertexArray(0);

	/*INITARRAY(rend->verts);
	INITARRAY(rend->indexes);*/

	rend->numIndicies = 0;
}


void populate_debugRend_buffers(DebugRend* rend)
{
	glBindBuffer(GL_ARRAY_BUFFER, rend->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * rend->verts.num, NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * rend->verts.num, rend->verts.buff);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rend->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, rend->indexes.num * sizeof(int), NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, rend->indexes.num * sizeof(int), rend->indexes.buff);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	rend->numIndicies = rend->indexes.num;
	rend->indexes.num = 0;
	rend->verts.num = 0;
}
void dispose_debug_renderer(DebugRend* rend)
{
	if (rend->vao)
	{
		glDeleteVertexArrays(1, &(rend->vao));
	}
	if (rend->vbo)
	{
		glDeleteBuffers(1, &(rend->vao));
	}
	if (rend->ibo)
	{
		glDeleteBuffers(1, &(rend->ibo));
	}
	dispose_shader(&shader_cache[DEBUG_PROG]);
	
}
void render_debug_lines(DebugRend* rend, mat4* CamMat)
{
	if (rend->numIndicies == 0) return;

	mat4 projection = { 0 };
	perspective(&projection, deg_to_rad(fov), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 10000.f);

	use_shader(&shader_cache[DEBUG_PROG]);
	set_mat4(&shader_cache[DEBUG_PROG], "projection", projection.mat);
	set_mat4(&shader_cache[DEBUG_PROG], "view", CamMat->mat);
	glLineWidth(4);
	glBindVertexArray(rend->vao);
	glDrawElements(GL_LINES, rend->numIndicies, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	unuse_shader(&shader_cache[DEBUG_PROG]);
}
