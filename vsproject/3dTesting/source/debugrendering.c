
CREATEDYNAMICARRAY(vec3, vert_buffer)
CREATEDYNAMICARRAY(int, index_buffer)



static vert_buffer verts;

typedef struct
{
	uint				vao;
	uint				vbo;
	uint				ibo;
	int					numIndicies;
	vert_buffer			verts;
	index_buffer		indexes;
} DebugRend;


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

	INITARRAY(rend->verts);
	INITARRAY(rend->indexes);

	rend->numIndicies = 0;
}

void draw_line(DebugRend* rend, const vec3 pos1, const vec3 pos2)
{
	vec3* vertArray = NULL;
	int ind = rend->verts.num;
	GET_NEW_BLOCK(rend->verts, vertArray, 2);
	vertArray[0] = pos1;
	vertArray[1] = pos2;
	
	PUSH_NEW_OBJ(rend->indexes, ind);
	PUSH_NEW_OBJ(rend->indexes, ind + 1);
}

inline vec2 rotatePoint(const vec2 point, float angle)
{
	vec2 newV = { 0 };
	newV.x = point.x * cosf(angle) - point.y *sinf(angle);
	newV.y = point.x * sinf(angle) + point.y *cosf(angle);
	return newV;
}

void draw_box(DebugRend* rend, const vec3 pos, const vec3 dim)
{
	int ind = rend->verts.num;
	vec3* vertArray = NULL;
	GET_NEW_BLOCK(rend->verts, vertArray, 8);
	vec3 p1 = { .x = pos.x - dim.x ,.y = pos.y - dim.y,.z = pos.z - dim.z };
	vec3 p2 = { .x = pos.x - dim.x ,.y = pos.y + dim.y,.z = pos.z - dim.z };
	vec3 p3 = { .x = pos.x + dim.x ,.y = pos.y + dim.y,.z = pos.z - dim.z };
	vec3 p4 = { .x = pos.x + dim.x ,.y = pos.y - dim.y,.z = pos.z - dim.z };

	vec3 p5 = { .x = pos.x - dim.x ,.y = pos.y - dim.y,.z = pos.z + dim.z };
	vec3 p6 = { .x = pos.x - dim.x ,.y = pos.y + dim.y,.z = pos.z + dim.z };
	vec3 p7 = { .x = pos.x + dim.x ,.y = pos.y + dim.y,.z = pos.z + dim.z };
	vec3 p8 = { .x = pos.x + dim.x ,.y = pos.y - dim.y,.z = pos.z + dim.z };

	vertArray[0] = p1;
	vertArray[1] = p2;
	vertArray[2] = p3;
	vertArray[3] = p4;

	vertArray[4] = p5;
	vertArray[5] = p6;
	vertArray[6] = p7;
	vertArray[7] = p8;

	int *indbuff = NULL;
	GET_NEW_BLOCK(rend->indexes, indbuff,24);


	indbuff[0] = ind++;
	indbuff[1]  = ind;
	indbuff[2]  = ind++;
	indbuff[3]  = ind;
	indbuff[4]  = ind++;
	indbuff[5]  = ind;
	indbuff[6]  = ind;
	indbuff[7]  = (ind++) -3;
		
	indbuff[8]  = ind++;
	indbuff[9]  = ind;
	indbuff[10]  = ind++;
	indbuff[11]  = ind;
	indbuff[12]  = ind++;
	indbuff[13]  = ind;
	indbuff[14]  = ind;
	indbuff[15]  = (ind++ )- 3;
	

	indbuff[16] = indbuff[0];
	indbuff[17] = indbuff[8];

	indbuff[18] = indbuff[2];
	indbuff[19] = indbuff[10];

	indbuff[20] = indbuff[4];
	indbuff[21] = indbuff[12];

	indbuff[22] = indbuff[6];
	indbuff[23] = indbuff[14];
	
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
	DISPOSE_ARRAY(rend->indexes);
	DISPOSE_ARRAY(rend->verts);
}
void render_debug_lines(DebugRend* rend, mat4* CamMat)
{
	if (rend->numIndicies == 0) return;

	mat4 projection = { 0 };
	perspective(&projection, deg_to_rad(fov), (float)SCREENWIDHT / (float)SCREENHEIGHT, 0.1f, 100.f);


	use_shader(&shader_cache[DEBUG_PROG]);
	set_mat4(&shader_cache[DEBUG_PROG], "projection", projection.mat);
	set_mat4(&shader_cache[DEBUG_PROG], "view", CamMat->mat);
	glLineWidth(4);
	glBindVertexArray(rend->vao);
	glDrawElements(GL_LINES, rend->numIndicies, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	unuse_shader(&shader_cache[DEBUG_PROG]);
}