
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
static inline struct Node* create_new_branch(AABBtree* tree,const vec3 pos,const vec3 dims)
{
	tree->allocator[tree->allocatorIndex].type = Branch;
	tree->allocator[tree->allocatorIndex].childIndexes[0] = 0;
	tree->allocator[tree->allocatorIndex].childIndexes[1] = 0;
	tree->allocator[tree->allocatorIndex].p = pos;
	tree->allocator[tree->allocatorIndex].w = dims;
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

static inline void insert_to_tree(AABBtree* tree,const Object* obj)
{
	if(tree->allocatorIndex == 0)
	{
		struct Node* temp = create_new_leaf(tree, obj);
		tree->rootIndex = 0;
		temp->parentIndex = 0;
		return;
	}
	struct Node* Leaf = create_new_leaf(tree, obj);
	struct Node* bestNode = get_best_node(tree, obj->base.position, obj->dims);
	if(bestNode->type == Leaf)
	{
		vec3 pos = { (obj->base.position.x + bestNode->object->base.position.x) / 2.f ,(obj->base.position.y + bestNode->object->base.position.y) / 2.f ,(obj->base.position.z + bestNode->object->base.position.z) / 2.f };
		vec3 dim = { (abs(obj->base.position.x - bestNode->object->base.position.x) + obj->dims.x + bestNode->object->dims.x) / 2.f ,
			(abs(obj->base.position.y - bestNode->object->base.position.y) + obj->dims.y + bestNode->object->dims.y) / 2.f  ,
			(abs(obj->base.position.z - bestNode->object->base.position.z) + obj->dims.z + bestNode->object->dims.z) / 2.f };
		struct Node* branch = create_new_branch(tree,)
	}
}