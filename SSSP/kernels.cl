__kernel void initialization(int source, __global int *C, int numberOfVertex)
{
	unsigned int id = get_global_id(0);
	if(id<numberOfVertex)
	{
		C[id] = 1e8;
	}
	if(id==source)
	{
		C[id] = 0;
	}
}
__kernel void ssspKernelPush(__global int *vertices, __global int *neighbours, __global int *sssp, __global int *changed, int numberOfVertex)
{
	unsigned int id = get_global_id(0);
	if(id<numberOfVertex)
	{
		int x = sssp[id];
		for(int child = vertices[id]; child<vertices[id+1]; child++)
		{
			if(x+1<sssp[neighbours[child]])
			{
				atomic_min(&sssp[neighbours[child]] , x+1);
				*changed = 1;
			}
		}

	}
}
__kernel void ssspKernelPull(__global int *vertices, __global int *neighbours, __global int *sssp, __global int *changed, int numberOfVertex)
{
	unsigned int id = get_global_id(0);
	if(id<numberOfVertex)
	{
		
		for(int parent = vertices[id]; parent<vertices[id+1]; parent++)
		{
			if(sssp[id]> sssp[neighbours[parent]]+1)
			{
				sssp[id] = sssp[neighbours[parent]]+1;
				*changed = 1;
			}
		}
	}
}