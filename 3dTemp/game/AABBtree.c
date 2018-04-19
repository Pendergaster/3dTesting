
typedef struct
{
	renderData*	base;
	vec3		dims;
} Object;

enum
{
	Leaf,
	Branch,
	Root
};

struct Node
{
	int		parentIndex;
	uint	type;
	union 
	{
		int			childIndexes[2];
		Object*		object;
	};
	vec3	p, w;
};

typedef struct
{
	uint			rootIndex;
	struct Node*	allocator;
	uint			allocatorIndex;
} AABBtree;

#define DEFAULT_ALLOCATORSIZE 100
static inline void init_tree(AABBtree* tree)
{
	tree->allocator = calloc(DEFAULT_ALLOCATORSIZE, sizeof(struct Node));
	tree->rootIndex = 0;
	tree->allocatorIndex = 0;
}
static inline struct Node* create_new_leaf(AABBtree* tree,const Object* obj) // todo -> FATTEN IT UP BOIIII
{
	tree->allocator[tree->allocatorIndex].type = Leaf;
	tree->allocator[tree->allocatorIndex].object = obj;
	tree->allocator[tree->allocatorIndex].p = obj->base->position;
	tree->allocator[tree->allocatorIndex].w = obj->dims;

	return	&tree->allocator[tree->allocatorIndex++];
}
static inline struct Node* create_new_branch(AABBtree* tree)
{
	tree->allocator[tree->allocatorIndex].type = Branch;
	tree->allocator[tree->allocatorIndex].childIndexes[0] = 0;
	tree->allocator[tree->allocatorIndex].childIndexes[1] = 0;
	// tree->allocator[tree->allocatorIndex].p = pos;
	// tree->allocator[tree->allocatorIndex].w = dims;
	return	&tree->allocator[tree->allocatorIndex++];
}
static inline ubyte AABB(const vec3 pos1,const vec3 dim1, const vec3 pos2, const vec3 dim2)
{
	float X = abs(pos1.x - pos2.x);
	float Y = abs(pos1.y - pos2.y);
	float Z = abs(pos1.z - pos2.z);

	return ((dim1.x + dim2.x) > X && (dim1.y + dim2.y) > Y && (dim1.z + dim2.z) > Z);
}
static inline struct Node* get_best_node(AABBtree* tree,const vec3 pos, const vec3 dims)
{
	struct Node* current = &tree->allocator[tree->rootIndex];
	while(1)
	{
		if (current->type == Leaf) return current;

		if(AABB(pos,dims, tree->allocator[current->childIndexes[0]].p, tree->allocator[current->childIndexes[0]].w))
		{
			current = &tree->allocator[current->childIndexes[0]];
			continue;
		}
		if (AABB(pos, dims, tree->allocator[current->childIndexes[1]].p, tree->allocator[current->childIndexes[1]].w))
		{
			current = &tree->allocator[current->childIndexes[1]];
			continue;
		}
		break;
	}
	return current;
}

// static inline void re_fit(struct Node* br,struct Node* branch,struct Node* leaf)
// {
// 	vec3 npos = {(branch->p.x + leaf->p.x)/2.f,(branch->p.y + leaf->p.y)/2.f, (branch->p.z + leaf->p.z)/2.f};
//     vec3 ndim =  {((branch->p.x + leaf->p.x)+(branch->w.x + leaf->w.x))/2.f,
// 	((branch->p.y + leaf->p.y)+(branch->w.y + leaf->w.y))/2.f, 
// 	((branch->w.z + leaf->w.z)+(branch->p.z + leaf->p.z))/2.f};

// 	br->p = npos;
// 	br->w = ndim;

// 	// float maxX = branch->p.x + leaf->p.x;
// 	// float maxY = branch->p.y + leaf->p.y;
// 	// float maxZ = branch->p.z + leaf->p.z;
	
// 	// float minX = branch->p.x - leaf->p.x;
// 	// float minY = branch->p.y - leaf->p.y;
// 	// float minZ = branch->p.z - leaf->p.z;

// 	// vec3 npos = {0};
//     // vec3 ndim =  {0};

// 	// npos.x = (maxX + minX) / 2.f;
// 	// npos.y = (maxY + minY) / 2.f;
// 	// npos.z = (maxZ + minZ) / 2.f;

// 	// ndim.x = (minX - maxX) * -1) ;
// 	// ndim.y = (minY - maxY) * -1) ;
// 	// ndim.z = (minZ - maxZ) * -1 );

// 	// br->p = npos;
// 	// br->w = ndim;
// }

static inline void force_fit_parent(AABBtree* tree,struct Node* parent)
{
	float maxX = parent->p.x + parent->w.x;
	float maxY = parent->p.y + parent->w.y;
	float maxZ = parent->p.z + parent->w.z;
	
	float minX = parent->p.x - parent->w.x;
	float minY = parent->p.y - parent->w.y;
	float minZ = parent->p.z - parent->w.z;

	struct Node* obj1 = &tree->allocator[parent->childIndexes[0]];
	struct Node* obj2 = &tree->allocator[parent->childIndexes[1]];

	float temp1 = obj1->p.x - obj1->w.x;
	float temp2 = obj2->p.x - obj2->w.x;
	float minX2 = temp1 < temp2 ? temp1 : temp2;

	temp1 = obj1->p.x + obj1->w.x;
	temp2 = obj2->p.x + obj2->w.x;
	float maxX2 = temp1 > temp2 ? temp1 : temp2;


	temp1 = obj1->p.y - obj1->w.y;
	temp2 = obj2->p.y - obj2->w.y;
	float minY2 = temp1 < temp2 ? temp1 : temp2;

	temp1 = obj1->p.y + obj1->w.y;
	temp2 = obj2->p.y + obj2->w.y;
	float maxY2 = temp1 > temp2 ? temp1 : temp2;


	temp1 = obj1->p.z - obj1->w.z;
	temp2 = obj2->p.z - obj2->w.z;
	float minZ2 = temp1 < temp2 ? temp1 : temp2;

	temp1 = obj1->p.z + obj1->w.z;
	temp2 = obj2->p.z + obj2->w.z;
	float maxZ2 = temp1 > temp2 ? temp1 : temp2;

	maxX = maxX > maxX2 ? maxX : maxX2;
	maxY = maxY > maxY2 ? maxY : maxY2;
	maxZ = maxZ > maxZ2 ? maxZ : maxZ2;

	minX = minX < minX2 ? minX : minX2;
	minY = minY < minY2 ? minY : minY2;
	minZ = minZ < minZ2 ? minZ : minZ2;

	vec3 npos = {0};
	vec3 ndim = {0};

	npos.x = (maxX + minX) / 2.f;
	npos.y = (maxY + minY) / 2.f;
	npos.z = (maxZ + minZ) / 2.f;
	
	ndim.x = (minX - maxX) * -0.5f ;
	ndim.y = (minY - maxY) * -0.5f;
	ndim.z = (minZ - maxZ) * -0.5f;

	parent->p = npos;
	parent->w = ndim;
}

static inline ubyte refit_parent(AABBtree* tree,struct Node* parent)
{

	// if(node->type == Root) 
	// {
	// 	return;
	// }
	//struct Node* parent = &tree->allocator[node->parentIndex];
	float maxX = parent->p.x + parent->w.x;
	float maxY = parent->p.y + parent->w.y;
	float maxZ = parent->p.z + parent->w.z;
	
	float minX = parent->p.x - parent->w.x;
	float minY = parent->p.y - parent->w.y;
	float minZ = parent->p.z - parent->w.z;

	struct Node* obj1 = &tree->allocator[parent->childIndexes[0]];
	struct Node* obj2 = &tree->allocator[parent->childIndexes[1]];

	float temp1 = obj1->p.x - obj1->w.x;
	float temp2 = obj2->p.x - obj2->w.x;
	float minX2 = temp1 < temp2 ? temp1 : temp2;

	temp1 = obj1->p.x + obj1->w.x;
	temp2 = obj2->p.x + obj2->w.x;
	float maxX2 = temp1 > temp2 ? temp1 : temp2;


	temp1 = obj1->p.y - obj1->w.y;
	temp2 = obj2->p.y - obj2->w.y;
	float minY2 = temp1 < temp2 ? temp1 : temp2;

	temp1 = obj1->p.y + obj1->w.y;
	temp2 = obj2->p.y + obj2->w.y;
	float maxY2 = temp1 > temp2 ? temp1 : temp2;


	temp1 = obj1->p.z - obj1->w.z;
	temp2 = obj2->p.z - obj2->w.z;
	float minZ2 = temp1 < temp2 ? temp1 : temp2;

	temp1 = obj1->p.z + obj1->w.z;
	temp2 = obj2->p.z + obj2->w.z;
	float maxZ2 = temp1 > temp2 ? temp1 : temp2;

	ubyte inserted = 0;
	if(maxX > maxX2)
	{
		maxX = maxX2;
		inserted = 1;
	}
	if(maxY > maxY2)
	{
		maxY = maxY2;
		inserted = 1;
	}
	if(maxZ > maxZ2)
	{
		maxZ = maxZ2;
		inserted = 1;
	}

	if(minX < minX2)
	{
		minX = minX2;
		inserted = 1;
	}
	if(minY < minY2)
	{
		minY = minY2;
		inserted = 1;
	}
	if(minZ < minZ2)
	{
		minZ = minZ2;
		inserted = 1;
	}
	if (!inserted) return 0;

	vec3 npos = {0};
	vec3 ndim = {0};

	npos.x = (maxX + minX) / 2.f;
	npos.y = (maxY + minY) / 2.f;
	npos.z = (maxZ + minZ) / 2.f;
	
	ndim.x = (minX - maxX) * -0.5f;
	ndim.y = (minY - maxY) * -0.5f;
	ndim.z = (minZ - maxZ) * -0.5f;

	parent->p = npos;
	parent->w = ndim;
	// maxX = maxX > maxX2 : maxX ? maxX2;
	// maxY = maxY > maxY2 : maxY ? maxY2;
	// maxZ = maxZ > maxZ2 : maxZ ? maxZ2;
	
	// minX = minX < minX2 : minX ? minX2;
	// minY = minY < minY2 : minY ? minY2;
	// minZ = minZ < minZ2 : minZ ? minZ2;

	// ubyte refit = 0;
	// do
	// {
	// 	float temp = (node->p.x + node->w.x);
	// 	if(temp > maxX)
	// 	{
	// 		maxX = temp; 
	// 		refit = 1;
	// 	}
	// 	temp = (node->p.x - node->w.x);
	// 	if(temp < minX)
	// 	{
	// 		minX = temp;
	// 		refit = 1;	
	// 	}
	// 	temp = (node->p.y + node->w.y);
	// 	if(temp > maxY)
	// 	{
	// 		maxY = temp;
	// 		refit = 1;
	// 	}
	// 	if((node->p.y - node->w.y) < minY)
	// 	{
	// 		refit = 1;
	// 	}
	// 	if((node->p.z + node->w.z) > maxZ)
	// 	{
	// 		refit = 1;
	// 	}
	// 	if((node->p.z - node->w.z) < minZ)
	// 	{
	// 		refit = 1;
	// 	}
	// }while(0);

	// if(refit)
	// {
	// 	re_fit(parent,node,node == &tree->allocator[parent->childIndexes[0]] ? &tree->allocator[parent->childIndexes[1]] : &tree->allocator[parent->childIndexes[0]]);
	// 	push_upper_dims(tree,parent);
	// }
	return 1;	
}


#define ARRAY_INDEX(ARRAY,OBJ)(OBJ - ARRAY)
static inline uint insert_to_tree(AABBtree* tree,const Object* obj)
{
	printf("%f\n",obj->base->position.x);
	if(tree->allocatorIndex == 0)
	{
		struct Node* temp = create_new_leaf(tree, obj);
		tree->rootIndex = 0;
		temp->parentIndex = 0;
		return 0;
	}


	
	if(tree->allocatorIndex == 1)
	{
		struct Node* leaf = create_new_leaf(tree, obj);
		struct Node* branch = create_new_branch(tree);
		struct Node t = *tree->allocator;
		tree->allocator->childIndexes[0] = 1;
		tree->allocator->childIndexes[1] = 2;
		*branch = t;
		tree->allocator->type = Root;
		tree->rootIndex = 0;
		//re_fit(tree->allocator,leaf,branch);
		force_fit_parent(tree,tree->allocator);
		leaf->parentIndex = 0;
		branch->parentIndex = 0;
		return (uint)ARRAY_INDEX(tree->allocator,leaf);
	}
	struct Node* leaf = create_new_leaf(tree, obj);
	struct Node* branch = create_new_branch(tree);

	struct Node* bestNode = get_best_node(tree, obj->base->position, obj->dims);




	(*branch) = (*bestNode);
	if(bestNode->type == Root)
	{
		//branch->type = Branch;
		tree->rootIndex = ARRAY_INDEX(tree->allocator,branch);
		bestNode->type = Branch;
	}
	else
	{
		tree->allocator[branch->parentIndex].childIndexes[ 
			&tree->allocator[tree->allocator[branch->parentIndex].childIndexes[0]] == bestNode ? 0 : 1
		 ] = ARRAY_INDEX(tree->allocator,branch);
	}


	
 	branch->childIndexes[0] = ARRAY_INDEX(tree->allocator,leaf);
	branch->childIndexes[1] = ARRAY_INDEX(tree->allocator,bestNode);



	leaf->parentIndex = ARRAY_INDEX(tree->allocator,branch);
	bestNode->parentIndex = ARRAY_INDEX(tree->allocator,branch);
	//create size for parent
	//re_fit(bestNode,branch,leaf);
	force_fit_parent(tree,branch);
	// vec3 npos = {(branch->p.x + leaf->p.x)/2.f,(branch->p.y + leaf->p.y)/2.f, (branch->p.z + leaf->p.z)/2.f};
    // vec3 ndim =  {((branch->p.x + leaf->p.x)+(branch->d.x + leaf->d.x))/2.f,
	// ((branch->p.y + leaf->p.y)+(branch->d.y + leaf->d.y)))/2.f, 
	// ((branch->d.z + leaf->d.z)+(branch->p.z + leaf->p.z))/2.f};

	//push upper dimensions
	struct Node* current = branch;
	while(1)
	{
		if(current->type == Root)
		{
			force_fit_parent(tree,current);
			break;
		}
		if(!refit_parent(tree,current)) break;
		current = &tree->allocator[current->parentIndex];
	}
	//push_upper_dims(tree,bestNode);
	return (uint)ARRAY_INDEX(tree->allocator,leaf);
}
// static inline struct Node* get_left_most(AABBtree* tree,struct Node* n)
// {
// 	while(n->type != Leaf)
// 	{
// 		n =  &tree->allocator[n->childIndexes[2]];
// 	}	
// 	return n;
// }
 static inline void draw_abbREEE(struct Node* n,DebugRend* rend)
 {
 	draw_box(rend,n->p,n->w);
 }