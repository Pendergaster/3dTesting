
typedef struct
{
	renderData	base;
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
static inline struct Node* create_new_leaf(AABBtree* tree,const Object* obj)
{
	tree->allocator[tree->allocatorIndex].type = Leaf;
	tree->allocator[tree->allocatorIndex].object = obj;
	tree->allocator[tree->allocatorIndex].p = obj->base.position;
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

	return (dim1.x + dim2.x) < X && (dim1.y + dim2.y) < Y && (dim1.z + dim2.z) < Z;
}
static inline struct Node* get_best_node(AABBtree* tree,const vec3 pos, const vec3 dims)
{
	struct Node* current = &tree->allocator[tree->allocatorIndex];
	while(1)
	{
		if (current->type == Leaf) return current;

		if(AABB(pos,dims, tree->allocator[current->childIndexes[0]].object->base.position, tree->allocator[current->childIndexes[0]].object->dims))
		{
			current = &tree->allocator[current->childIndexes[0]];
			continue;
		}
		if (AABB(pos, dims, tree->allocator[current->childIndexes[1]].object->base.position, tree->allocator[current->childIndexes[0]].object->dims))
		{
			current = &tree->allocator[current->childIndexes[1]];
			continue;
		}
		break;
	}
	return current;
}

static inline void re_fit(struct Node* br,struct Node* branch,struct Node* leaf)
{
	vec3 npos = {(branch->p.x + leaf->p.x)/2.f,(branch->p.y + leaf->p.y)/2.f, (branch->p.z + leaf->p.z)/2.f};
    vec3 ndim =  {((branch->p.x + leaf->p.x)+(branch->w.x + leaf->w.x))/2.f,
	((branch->p.y + leaf->p.y)+(branch->w.y + leaf->w.y))/2.f, 
	((branch->w.z + leaf->w.z)+(branch->p.z + leaf->p.z))/2.f};

	br->p = npos;
	br->w = ndim;
}



static inline void push_upper_dims(AABBtree* tree,struct Node* node)
{
	if(node->type == Root) return;

	struct Node* parent = &tree->allocator[node->parentIndex];
	float maxX = parent->p.x + parent->w.x;
	float maxY = parent->p.y + parent->w.y;
	
	float minX = parent->p.x - parent->w.x;
	float minY = parent->p.y - parent->w.y;

	ubyte refit = 0;
	do
	{
		if((node->p.x + node->w.x) > maxX)
		{
			refit = 1;
			break;
		}
		if((node->p.x - node->w.x) < minX)
		{
			refit = 1;
			break;		
		}
		if((node->p.y + node->w.y) > maxY)
		{
			refit = 1;
			break;
		}
		if((node->p.y - node->w.y) < minY)
		{
			refit = 1;
			break;		
		}

	}while(0);

	if(refit)
	{
		re_fit(parent,node,node == &tree->allocator[parent->childIndexes[0]] ? &tree->allocator[parent->childIndexes[1]] : &tree->allocator[parent->childIndexes[0]]);
	}
	return;	
}


#define ARRAY_INDEX(ARRAY,OBJ)(OBJ - ARRAY)
static inline void insert_to_tree(AABBtree* tree,const Object* obj)
{
	if(tree->allocatorIndex == 0)
	{
		struct Node* temp = create_new_leaf(tree, obj);
		tree->rootIndex = 0;
		temp->parentIndex = 0;
		return;
	}
	struct Node* leaf = create_new_leaf(tree, obj);
	struct Node* branch = create_new_branch(tree);
	struct Node* bestNode = get_best_node(tree, obj->base.position, obj->dims);

	struct Node temp = *bestNode;
	bestNode->type = Branch;
	bestNode->childIndexes[0] = ARRAY_INDEX(tree->allocator,leaf);
	bestNode->childIndexes[1] = ARRAY_INDEX(tree->allocator,branch);
	(*branch) = temp; // push old finding down
	leaf->parentIndex = ARRAY_INDEX(tree->allocator,bestNode);
	branch->parentIndex = ARRAY_INDEX(tree->allocator,bestNode);
	//create size for parent
	re_fit(bestNode,branch,leaf);
	// vec3 npos = {(branch->p.x + leaf->p.x)/2.f,(branch->p.y + leaf->p.y)/2.f, (branch->p.z + leaf->p.z)/2.f};
    // vec3 ndim =  {((branch->p.x + leaf->p.x)+(branch->d.x + leaf->d.x))/2.f,
	// ((branch->p.y + leaf->p.y)+(branch->d.y + leaf->d.y)))/2.f, 
	// ((branch->d.z + leaf->d.z)+(branch->p.z + leaf->p.z))/2.f};
	//push upper dimensions
	push_upper_dims(tree,bestNode);
	
}