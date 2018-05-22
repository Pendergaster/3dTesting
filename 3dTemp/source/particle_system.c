




void init_particle_system(ParticleSystem* ps, double startTime)
{
		ps->particles = calloc(MAX_PARTICLES,sizeof(Particle));
		ps->currentParticleIndex = 0;
		
		ShaderHandle* sha = get_shader(ParticleProg);
		LASTWRITES[particle_vert] = Win32GetLastWriteTime(txt_file_names[particle_vert]);
		LASTWRITES[particle_frag] = Win32GetLastWriteTime(txt_file_names[particle_frag]);

		char* vertfile = load_file(particle_vert,NULL);
		uint vertid = compile_shader(GL_VERTEX_SHADER, vertfile);
		free(vertfile);
		vertfile = NULL;

		char* fragfile = load_file(particle_frag,NULL);
		uint fragid = compile_shader(GL_FRAGMENT_SHADER, fragfile);
		free(fragfile);
		vertfile = NULL;

		sha->progId = glCreateProgram();
		glAttachShader(sha->progId,vertid);
		glAttachShader(sha->progId,fragid);

		add_attribute(sha,"vertPos");	
		add_attribute(sha,"vel");	
		add_attribute(sha,"time");	
		add_attribute(sha,"wpos");	
		add_attribute(sha,"scale");	
		add_attribute(sha,"lifetime");	
		glCheckError();


		link_shader(sha,vertid,fragid);	
		use_shader(sha);
		unuse_shader(sha);
		glCheckError();

		glGenVertexArrays(1,&ps->vao);	
		uint buffers[2];
		glGenBuffers(2,buffers);

		glBindVertexArray(ps->vao);
		glBindBuffer(GL_ARRAY_BUFFER,buffers[0]); 	//vertexes
		

		static const float g_vertex_buffer_data[] = {
			/*	-0.5f,  0.5f,
				0.5f, -0.5f,
				-0.5f, -0.5f,

				-0.5f,  0.5f,
				0.5f, -0.5f, 
				0.5f,  0.5f, */
				 -0.5f, -0.5f,
 0.5f, -0.5f, 
 -0.5f, 0.5f,
 0.5f, 0.5f
				
			};

		glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);//  bind once

		glBindBuffer(GL_ARRAY_BUFFER,buffers[1]);
		
		glBufferData(GL_ARRAY_BUFFER,sizeof(Particle) * MAX_PARTICLES,NULL, GL_DYNAMIC_DRAW); // bound later

		
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);	
		glEnableVertexAttribArray(4);	
		glCheckError();


		glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
		glVertexAttribDivisor(0, 0);//Vertex position, update always

		glBindBuffer(GL_ARRAY_BUFFER,buffers[1]);

		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Particle),(void*)offsetof(Particle, velocity));
		glVertexAttribDivisor(1, 1);//Vertex position, update always
		glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,sizeof(Particle),(void*)offsetof(Particle, times));
		glVertexAttribDivisor(2, 1);//Vertex position, update always
		glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(Particle),(void*)offsetof(Particle, startPosition));
		glVertexAttribDivisor(3, 1);//Vertex position, update always
		glVertexAttribPointer(4,1,GL_FLOAT,GL_FALSE,sizeof(Particle),(void*)offsetof(Particle, scale));
		glVertexAttribDivisor(4, 1);//Vertex position, update always
		glCheckError();


		ps->startTime = (float)startTime;
		ps->buffer = buffers[1];
		//TODO anna buffer structiin
		
		

	    glBindVertexArray(0);	
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
}

static inline void create_buffers_particle(ParticleSystem* ps)
{
	glBindBuffer(GL_ARRAY_BUFFER,ps->buffer);	
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(Particle), NULL, GL_DYNAMIC_DRAW); 
    glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_PARTICLES * sizeof(Particle), ps->particles);
		
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static inline void render_particles(const ParticleSystem* ps,const mat4* view,const mat4* projection,float currentTime, uint textureId)
{
	ShaderHandle* sha = get_shader(ParticleProg);	
	glBindVertexArray(ps->vao);
	use_shader(sha);
		
	set_mat4(sha,"view",view->mat);
	set_mat4(sha,"projection",projection->mat);
	set_uniform_float(sha,"currentTime",currentTime);
    set_uniform_int(sha,"tex",0);

	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId); 

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, MAX_PARTICLES);
    
}

static inline void dispose_particles(ParticleSystem* ps)
{
	free(ps->particles);	
	ps->particles = NULL;
}


