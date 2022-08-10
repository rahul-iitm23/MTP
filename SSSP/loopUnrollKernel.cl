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
	unsigned int localId = get_local_id(0);
	int maxItemPerThread = 16;
	int N = get_local_size(0);
	__local int wlSize;
	__local int Worklist[128]; 
	int arr[16];
	int i=0; 
	wlSize = 0;
	if(id<numberOfVertex)
	{
		int x = sssp[id];
		for(int childIndex = vertices[id]; childIndex<vertices[id+1]; childIndex++)
		{
			if(x+1<sssp[neighbours[childIndex]])
			{
				atomic_min(&sssp[neighbours[childIndex]] , x+1);
				*changed = 1;
				if(i<maxItemPerThread)
				{
					arr[i] = neighbours[childIndex];
					i++;
				}
				
			}
		}
	}

	int workListStartIndex = atomic_add(&wlSize, i);
	for(int j = workListStartIndex; (j<(workListStartIndex+i))&&(j<N) ; j++)
	{
		Worklist[j] = arr[j-workListStartIndex];
	}
	barrier(CLK_LOCAL_MEM_FENCE);

	if(localId<wlSize)
	{
		id = Worklist[localId];
		int x = sssp[id];
		for(int childIndex = vertices[id]; childIndex<vertices[id+1]; childIndex++)
		{
			if(x+1<sssp[neighbours[childIndex ]])
			{
				atomic_min(&sssp[neighbours[childIndex]] , x+1);
			}
		}
	}
	
}


__kernel void ssspKernelPull(__global int *revIndexCSR, __global int *CSC, __global int *indexCSR, 
	__global int *CSR,  __global int *sssp, __global int *changed, int numberOfVertex)
{
	unsigned int id = get_global_id(0);
	int flag = 0;
	if(id<numberOfVertex)
	{
		
		for(int parent = revIndexCSR[id]; parent<revIndexCSR[id+1]; parent++)
		{
			if(sssp[id]> sssp[CSC[parent]]+1)
			{
				sssp[id] = sssp[CSC[parent]]+1;
				*changed = 1;
				flag = 1;
			}
		}
	}
	if(flag==1)
	{
		int x = sssp[id];
		for(int childIndex = indexCSR[id]; childIndex<indexCSR[id+1]; childIndex++)
		{
			if(x+1<sssp[CSR[childIndex ]])
			{
				atomic_min(&sssp[CSR[childIndex]] , x+1);
			}
				
		}
	}
	
	
}

