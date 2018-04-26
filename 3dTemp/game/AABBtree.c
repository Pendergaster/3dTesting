

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


CREATEDYNAMICARRAY(struct Node, NodeArray)
CREATEDYNAMICARRAY(struct Node*, NodeStack)




typedef struct
{
	uint			rootIndex;
	NodeArray		NodeArrayallocator;
	index_buffer	freelist;	
	int				numObjs;
	NodeStack		nodeStack;
	ObjectBuffer	objectbuffer;
} AABBtree;

#define allocator NodeArrayallocator.buff

//#define DEFAULT_ALLOCATORSIZE 100
#define allocHandle 

static inline void init_tree(AABBtree* tree)
{
	//tree->allocator = calloc(DEFAULT_ALLOCATORSIZE, sizeof(struct Node));
	tree->rootIndex = 0;
	tree->numObjs = 0;
	INITARRAY(tree->freelist);
	INITARRAY(tree->NodeArrayallocator);
	INITARRAY(tree->nodeStack);
	INITARRAY(tree->objectbuffer);
}

#define ARRAY_INDEX(ARRAY,OBJ)(OBJ - ARRAY)
#define FATTEN 0.f
static inline uint create_new_leaf(AABBtree* tree,const Object* obj) // todo -> FATTEN IT UP BOIIII
{	
	if(tree->freelist.num == 0)
	{
		struct Node* n = NULL;
		GET_NEW_OBJ(tree->NodeArrayallocator,n);
		n->type = Leaf;
		n->object = obj;
		n->p = obj->base->position;
		n->w = obj->dims;
		n->w.x += FATTEN;
		n->w.y += FATTEN;
		n->w.z += FATTEN;
		return	ARRAY_INDEX(tree->allocator,n);
	}
	else
	{
		struct Node* ret = NULL;
		int* index = NULL;
		POP_ARRAY(tree->freelist,index);
		ret = &tree->allocator[*index];

		ret->type = Leaf;
		ret->object = obj;
		ret->p = obj->base->position;		
		ret->w = obj->dims;			
		ret->w.x += FATTEN;
		ret->w.y += FATTEN;
		ret->w.z += FATTEN;
		return	ARRAY_INDEX(tree->allocator,ret);
		
	}
}
static inline uint create_new_branch(AABBtree* tree)
{
	if(tree->freelist.num == 0)
	{
		struct Node* n = NULL;
		GET_NEW_OBJ(tree->NodeArrayallocator,n);
		
		n->type = Branch;
		n->childIndexes[0] = 0;
		n->childIndexes[1] = 0;
		return	ARRAY_INDEX(tree->allocator,n);

	}
	else
	{
		struct Node* ret = NULL;
		int* index = NULL;
		POP_ARRAY(tree->freelist,index);
		ret = &tree->allocator[*index];

		ret->type = Branch;
		ret->childIndexes[0] = 0;
		ret->childIndexes[1] = 0;
		return	ARRAY_INDEX(tree->allocator,ret);


	}
}
static inline ubyte AABB(const vec3 pos1,const vec3 dim1, const vec3 pos2, const vec3 dim2)
{
	ubyte ret = 0;
	float X = abs(pos1.x - pos2.x);
	float Y = abs(pos1.y - pos2.y);
	float Z = abs(pos1.z - pos2.z);

	return ((dim1.x + dim2.x) > X && (dim1.y + dim2.y) > Y && (dim1.z + dim2.z) > Z);
}

static inline float calculate_area(struct Node* obj1,vec3 p, vec3 w)
{
	float temp1 = obj1->p.x - obj1->w.x;
	float temp2 = p.x - w.x;
	float minX2 = temp1 < temp2 ? temp1 : temp2;

	temp1 = obj1->p.x + obj1->w.x;
	temp2 = p.x + w.x;
	float maxX2 = temp1 > temp2 ? temp1 : temp2;


	temp1 = obj1->p.y - obj1->w.y;
	temp2 = p.y - w.y;
	float minY2 = temp1 < temp2 ? temp1 : temp2;

	temp1 = obj1->p.y + obj1->w.y;
	temp2 = p.y + w.y;
	float maxY2 = temp1 > temp2 ? temp1 : temp2;


	temp1 = obj1->p.z - obj1->w.z;
	temp2 = p.z - w.z;
	float minZ2 = temp1 < temp2 ? temp1 : temp2;

	temp1 = obj1->p.z + obj1->w.z;
	temp2 = p.z + w.z;
	float maxZ2 = temp1 > temp2 ? temp1 : temp2;

	float maxX = maxX2;// > maxX2 ? maxX : maxX2;
	float maxY = maxY2;// > maxY2 ? maxY : maxY2;
	float maxZ = maxZ2;// > maxZ2 ? maxZ : maxZ2;

	float minX = minX2;//minX < minX2 ? minX : minX2;
	float minY = minY2;//minY < minY2 ? minY : minY2;
	float minZ = minZ2;// < minZ2 ? minZ : minZ2;

	vec3 ndim = {0};
	
	ndim.x = (minX - maxX) ;
	ndim.y = (minY - maxY);
	ndim.z = (minZ - maxZ);

	return ndim.x *ndim.y  + ndim.y* ndim.z + ndim.x * ndim.z;
}

static inline struct Node* get_best_node(AABBtree* tree,const vec3 pos, const vec3 dims)
{
	struct Node* current = &tree->allocator[tree->rootIndex];
	while(1)
	{
		if (current->type == Leaf) return current;
		float surface0 =  calculate_area(&tree->allocator[current->childIndexes[0]],pos,dims );//tree->allocator[current->childIndexes[0]].w.x *  tree->allocator[current->childIndexes[0]].w.y *  tree->allocator[current->childIndexes[0]].w.z;
		float surface1 =  calculate_area(&tree->allocator[current->childIndexes[1]],pos,dims );//tree->allocator[current->childIndexes[1]].w.x *  tree->allocator[current->childIndexes[1]].w.y *  tree->allocator[current->childIndexes[1]].w.z;

		if(surface0 < surface1)
		{
			current =  &tree->allocator[current->childIndexes[0]];
		}
		else
		{
			current =  &tree->allocator[current->childIndexes[1]];
		}
	}
	return current;
}


static inline void force_fit_parent(AABBtree* tree,struct Node* parent)
{

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

	float maxX = maxX2;// > maxX2 ? maxX : maxX2;
	float maxY = maxY2;// > maxY2 ? maxY : maxY2;
	float maxZ = maxZ2;// > maxZ2 ? maxZ : maxZ2;

	float minX = minX2;//minX < minX2 ? minX : minX2;
	float minY = minY2;//minY < minY2 ? minY : minY2;
	float minZ = minZ2;// < minZ2 ? minZ : minZ2;

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
	if(maxX < maxX2)
	{
		maxX = maxX2;
		inserted = 1;
	}
	if(maxY < maxY2)
	{
		maxY = maxY2;
		inserted = 1;
	}
	if(maxZ < maxZ2)
	{
		maxZ = maxZ2;
		inserted = 1;
	}

	if(minX > minX2)
	{
		minX = minX2;
		inserted = 1;
	}
	if(minY > minY2)
	{
		minY = minY2;
		inserted = 1;
	}
	if(minZ > minZ2)
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

	return 1;	
}



static inline uint insert_to_tree(AABBtree* tree,const Object* obj)
{
	tree->numObjs++;
//	printf("NUM OBJECTS%d\n",tree->numObjs);
	if(tree->NodeArrayallocator.num == 0)
	{
		uint tempInd = create_new_leaf(tree, obj);
		struct Node* temp = &tree->allocator[tempInd];
		tree->rootIndex = 0;
		temp->parentIndex = 0;
		return 0;
	}


	
	if(tree->NodeArrayallocator.num == 1)
	{
		uint leafIndex = create_new_leaf(tree, obj);
		uint branchIndex = create_new_branch(tree);

		struct Node* leaf = &tree->allocator[leafIndex];
		struct Node* branch = &tree->allocator[branchIndex];
		//struct Node t = *tree->allocator;
		branch->childIndexes[0] = tree->rootIndex;
		branch->childIndexes[1] = ARRAY_INDEX(tree->allocator,leaf);

		tree->allocator[tree->rootIndex].parentIndex = ARRAY_INDEX(tree->allocator, branch);
		leaf->parentIndex = ARRAY_INDEX(tree->allocator, branch);
		tree->rootIndex = ARRAY_INDEX(tree->allocator,branch);
		branch->type = Root;


		//*branch = t;


		// tree->allocator->type = Root;
		// tree->rootIndex = 0;
		//re_fit(tree->allocator,leaf,branch);
		force_fit_parent(tree,branch);
		// leaf->parentIndex = 0;
		// branch->parentIndex = 0;
		return (uint)ARRAY_INDEX(tree->allocator,leaf);
	}
	uint leafIndex = create_new_leaf(tree, obj);
	uint branchIndex = create_new_branch(tree);

	struct Node* leaf = &tree->allocator[leafIndex];
	struct Node* branch = &tree->allocator[branchIndex];

	struct Node* bestNode = get_best_node(tree, obj->base->position, obj->dims);




	//(*branch) = (*bestNode);
	uint k1 = ARRAY_INDEX(tree->allocator,branch);
	uint k2 = tree->allocator[bestNode->parentIndex].childIndexes[0];
	uint k3 = tree->allocator[bestNode->parentIndex].childIndexes[1];
	if(bestNode->type == Root)
	{
		//branch->type = Branch;
		tree->rootIndex = ARRAY_INDEX(tree->allocator,branch);
		bestNode->type = Branch;
		branch->type = Root;
	}
	else
	{
		tree->allocator[bestNode->parentIndex].childIndexes[ 
			//&tree->allocator[tree->allocator[branch->parentIndex].childIndexes[0]] == bestNode ? 0 : 1
			tree->allocator[bestNode->parentIndex].childIndexes[0] == ARRAY_INDEX(tree->allocator,bestNode) ? 0 : 1
		 ] = ARRAY_INDEX(tree->allocator,branch);
	}


	
 	branch->childIndexes[0] = ARRAY_INDEX(tree->allocator,leaf);
	branch->childIndexes[1] = ARRAY_INDEX(tree->allocator,bestNode);
	branch->parentIndex = bestNode->parentIndex;


	leaf->parentIndex = ARRAY_INDEX(tree->allocator,branch);
	bestNode->parentIndex = ARRAY_INDEX(tree->allocator,branch);

	force_fit_parent(tree,branch);

	assert(branch->type != Root && branch->type != Leaf);
	struct Node* current = &tree->allocator[branch->parentIndex];

	while(1)
	{
		if(current->type == Root)
		{
			force_fit_parent(tree,current);
			break;
		}
		//if(!refit_parent(tree,current)) break;
		force_fit_parent(tree,current);
		current = &tree->allocator[current->parentIndex];
	}
	//push_upper_dims(tree,bestNode);
	return (uint)ARRAY_INDEX(tree->allocator,leaf);
}

static inline void remove_node(AABBtree* tree, uint node)
{
	if(tree->numObjs < 3)
	{
 		struct Node* leaf = &tree->allocator[node];
 		assert(leaf->type == Leaf);
 		PUSH_NEW_OBJ(tree->freelist,(ARRAY_INDEX(tree->allocator,leaf)));
 	}
 	struct Node* parent = &tree->allocator[tree->allocator[node].parentIndex];
 	if(parent->type == Branch)
 	{
 		struct Node* leaf = &tree->allocator[node];
 		struct Node* grandpa =  &tree->allocator[parent->parentIndex];
		struct Node* leaf_to_stay = &tree->allocator[ parent->childIndexes[ parent->childIndexes[0] == ARRAY_INDEX(tree->allocator,leaf) ? 1 : 0 ] ];

 		grandpa->childIndexes[ 
 			grandpa->childIndexes[0] == ARRAY_INDEX(tree->allocator,parent) ? 0 : 1
 			] = parent->childIndexes[ parent->childIndexes[0] == ARRAY_INDEX(tree->allocator, leaf_to_stay) ? 0 : 1 ];

 		PUSH_NEW_OBJ(tree->freelist,(ARRAY_INDEX(tree->allocator,leaf)));
 		PUSH_NEW_OBJ(tree->freelist,(ARRAY_INDEX(tree->allocator,parent)));

		leaf_to_stay->parentIndex = ARRAY_INDEX(tree->allocator, grandpa);



//		force_fit_parent(tree,grandpa);
		struct Node* current = grandpa;

		while(1)
		{
			if(current->type == Root)
			{
				force_fit_parent(tree,current);
				break;
			}
		//if(!refit_parent(tree,current)) break;
			force_fit_parent(tree,current);
			current = &tree->allocator[current->parentIndex];
		}
 	}
 	else
 	{
 		assert(parent->type == Root);
 		struct Node* leaf = &tree->allocator[node];
 		struct Node* branch_to_del = &tree->allocator[parent->childIndexes[ node == parent->childIndexes[0] ? 1 : 0]];
 		parent->childIndexes[0] = branch_to_del->childIndexes[0];
 		parent->childIndexes[1] = branch_to_del->childIndexes[1];

 		PUSH_NEW_OBJ(tree->freelist,(ARRAY_INDEX(tree->allocator,leaf)));
 		PUSH_NEW_OBJ(tree->freelist,(ARRAY_INDEX(tree->allocator,branch_to_del)));
		force_fit_parent(tree,parent);
 	}
 	tree->numObjs--;
}

static uint inline update_object_in_tree(AABBtree* tree,uint node)
{
	
	//printf("type = %d\n", tree->allocator[node].type );
	assert(tree->allocator[node].type == Leaf);
	Object* ob = tree->allocator[node].object;

	//contains?
	float maxX = tree->allocator[node].p.x + tree->allocator[node].w.x;
	float maxY = tree->allocator[node].p.y + tree->allocator[node].w.y;
	float maxZ = tree->allocator[node].p.z + tree->allocator[node].w.z;

	float minX = tree->allocator[node].p.x - tree->allocator[node].w.x;
	float minY = tree->allocator[node].p.y - tree->allocator[node].w.y;
	float minZ = tree->allocator[node].p.z - tree->allocator[node].w.z;
	ubyte reinsert = 1;
	do
	{
		if(!(BETWEEN(minX , (ob->base->position.x + ob->dims.x) ,maxX))) break;		
		if(!(BETWEEN(minX , (ob->base->position.x - ob->dims.x) ,maxX))) break;		
		if(!(BETWEEN(minY , (ob->base->position.y + ob->dims.y) ,maxY))) break;		
		if(!(BETWEEN(minY , (ob->base->position.y - ob->dims.y) ,maxY))) break;		
		if(!(BETWEEN(minZ , (ob->base->position.z + ob->dims.z) ,maxZ))) break;		
		if(!(BETWEEN(minZ , (ob->base->position.z - ob->dims.z) ,maxZ))) break;		



		reinsert = 0;
	} while(0);

	if(reinsert)
	{
		//printf("REINSERTING\n");
		remove_node(tree,node);
		node = insert_to_tree(tree,ob);
	}
	return node;
}

#define DEBUG_TREE
#ifdef DEBUG_TREE
int numchecks[NUM_OBJS] = {0};
int index = 0;
float currentTime = 0;
const float dt = 1.f / 60.f;

int averageChecks[10] = {0};
int averageIndex = 0;
#endif
static void inline query_area(AABBtree* tree, vec3 pos, float r,ObjectBuffer* buffer)
{
#ifdef DEBUG_TREE
	if(index >= NUM_OBJS)
	{
		
		float add = 0;
	 	for(int i = 0; i < NUM_OBJS;i++)
	 		add += (float)numchecks[i];

		add /= (float)NUM_OBJS;
		if(averageIndex >= 10)
			averageIndex = 0;
		averageChecks[averageIndex++] = (int)add;
		index = 0;

		currentTime += dt;
		if(currentTime > 1.f)
		{
			float add = 0;
			for(int i = 0; i < 10;i++)
				add += averageChecks[i];

			add /= 10.f;
			printf("AVERAGE CHECK IN TREE IS %.2f\n",add);
			currentTime = 0;
		}
	}
#endif
	int checks = 0;
	PUSH_NEW_OBJ(tree->nodeStack, &tree->allocator[tree->rootIndex]);
	struct Node* current = NULL;
	vec3 dims = {r,r,r};
	while(tree->nodeStack.num)
	{
		POP_ARRAY_COPY(tree->nodeStack,current);
		assert(current->type == Leaf ||current->type == Branch || current->type == Root);

		if(AABB(current->p,current->w, pos,dims))
		{
			checks++;
			if(current->type == Leaf)
			{
				//printf("VISITED\n");
				assert(current->object->base->modelId == Planet1);
				PUSH_NEW_OBJ(tree->objectbuffer, current->object);
			}
			else
			{	
				PUSH_NEW_OBJ(tree->nodeStack, &tree->allocator[current->childIndexes[0]]);
				PUSH_NEW_OBJ(tree->nodeStack, &tree->allocator[current->childIndexes[1]]);
			}
		}

	}	
#ifdef DEBUG_TREE//printf("%d\n", checks);
	numchecks[index++] = checks;//tree->objectbuffer.num;
#endif


	tree->nodeStack.num = 0;

	r *= r;
	assert(tree->objectbuffer.num <= tree->numObjs);
	for(int i = 0; i < tree->objectbuffer.num; i++)
	{
		Object* ob = tree->objectbuffer.buff[i];
		float dist = (pos.x - ob->base->position.x) * (pos.x - ob->base->position.x)  +
			(pos.y - ob->base->position.y) *  (pos.y - ob->base->position.y) + 
			(pos.z - ob->base->position.z) * (pos.z - ob->base->position.z);
		if(r > dist)
		{
			int xt = (int)ob->base->position.x;
			int yt = (int)ob->base->position.y;
			int zt = (int)ob->base->position.z;
			PUSH_NEW_OBJ((*buffer),ob);
		}
		
	}
	tree->objectbuffer.num = 0;

}

static void draw_tree(AABBtree* tree,DebugRend* rend)
{
	

	PUSH_NEW_OBJ(tree->nodeStack, &tree->allocator[tree->rootIndex]);
	struct Node* current = NULL;
	while(tree->nodeStack.num)
	{
		POP_ARRAY_COPY(tree->nodeStack,current);

		draw_box(rend,current->p,current->w);

		if(current->type == Leaf)
		{
			continue;
			//PUSH_NEW_OBJ(tree->objectbuffer, current->object);
		}
		else
		{	
			PUSH_NEW_OBJ(tree->nodeStack, &tree->allocator[current->childIndexes[0]]);
			PUSH_NEW_OBJ(tree->nodeStack, &tree->allocator[current->childIndexes[1]]);
		}
	}	
	tree->nodeStack.num = 0;
}

static inline void dispose_tree(AABBtree* tree)
{
	DISPOSE_ARRAY(tree->freelist);
	DISPOSE_ARRAY(tree->NodeArrayallocator);
	DISPOSE_ARRAY(tree->nodeStack);
	DISPOSE_ARRAY(tree->objectbuffer);
}