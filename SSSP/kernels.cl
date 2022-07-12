__kernel void initialization(int source, __global int *C, int numberOfVertex)
{
	unsigned int id = get_global_id(0);
	if(id<numberOfVertex)
	{
		C[id] = 1e6;
	}
	if(id==source)
	{
		C[id] = 0;
	}
}
__kernel void ssspKernel(__global int *vertices, __global int *neighbours, __global int *sssp, __global int *changed, int numberOfVertex)
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
