
myree k;
typedef struct
{
	mat4	view;
	vec3	cameraPos;// = { 0.f,0.f,3.f };
	vec3	cameraDir;
	vec3	camUp;
	float	yaw;
	float	pitch;
	//float	fov;
} Camera;



static const vec3 up = { 0.f , 1.f , 0.f };
static const vec3 camDir = { 0.f , 0.f , -1.f };

void init_camera(Camera* c)
{
	identity(&c->view);
	vec3 temp = { 0 };
	vec3 cameraPos = { 0.f,0.f,3.f };
	vec3 cameraTarget = { 1.f,0.f,0.f };
	
	c->cameraPos = cameraPos;
	c->cameraDir = camDir;

	create_lookat_mat4(&c->view, &c->cameraPos, &c->cameraDir, &up);

	c->camUp.x = 0.f;
	c->camUp.y = 1.f;
	c->camUp.z = 0.f;
	c->yaw = -90.0f;
	c->pitch = 0;
	//c->fov = 45.0f;
}
const vec3 worldUP = { 0.f,1.f,0.f };
void update_camera(Camera* c,vec2 newMousePos,vec2 lastMousePos)
{
	float xOff = newMousePos.x - lastMousePos.x;
	float yOff = lastMousePos.y - newMousePos.y;
	
	const float sensitivity = 0.05f;
	xOff *= sensitivity;
	yOff *= sensitivity;
	

	c->yaw += xOff;
	c->pitch += yOff;

	if (c->pitch > 85.0f)
		c->pitch = 85.0f;
	if (c->pitch < -85.0f)
		c->pitch = -85.0f;

	c->cameraDir.x = cosf(deg_to_rad(c->pitch))*cosf(deg_to_rad(c->yaw));
	c->cameraDir.y = sinf(deg_to_rad(c->pitch));
	c->cameraDir.z = sinf(deg_to_rad(c->yaw))*cosf(deg_to_rad(c->pitch));

	normalize_vec3(&c->cameraDir);

	vec3 front = { 0 };
	add_vec3(&front, &c->cameraPos, &c->cameraDir);

	
	cross_product(&c->camUp, &c->cameraDir, &worldUP);
	normalize_vec3(&c->camUp);
	cross_product(&c->camUp, &c->camUp, &c->cameraDir);
	normalize_vec3(&c->camUp);
	//normalize_vec3(&front);
	// fron pitäs olla 0 0 2
	create_lookat_mat4(&c->view, &c->cameraPos, &front, &c->camUp);
}

void rotateCamera(Camera *c,float pitch,float yaw,float roll)
{
	
}

