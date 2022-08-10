kernels.cl : This file contains the kernels used by ssspv1.cpp and pullsssp.cpp. There are 3 kernels.
              1. initialization kernel
              2.ssspKernelPush()
              3. ssspKernelPull()
ssspv1.cpp : First version of sssp. It is based on push method and therefore uses atomics for handling the data race. Obstcle faced while
              implementing this version is implementation of pinned memory. Command line arguments for this program are
              i. kernels.cl
              ii. input.txt file: first line of input.txt must contain 3 things number of vertices(n), number of edges(e) and source vertex
              following e lines will contain e edges of the graph.
              iii. output.txt file for the result
pullsssp.cpp : This file containes the implementation of single source shortest path using pull method. So this implementation is atomics                   free.  Command line arguments goes same as ssspv1.cpp .
ssspv2.cpp : In this implementation, I have tried loop unrolling with push method. If a thread founds that there was change in shortest                   path from source  then it pushes all his child to worklist. Worklist is shared across all the work-item of a work group.
              So a thread after completing his work picks a item from worklist and process that. So here one we have one kind of work                     donation. As one node do not need to process all his children. it's children are getting processed by other nodes also.
              Command line argument goes same as ssspv1.cpp except instead of kernels.cl we need to pass loopUnrollKernel.cl .
pullssspv2.cpp : This one implements loop unrolling but in this case we are doing first pull and then push. It is line one node is fetching                 information from his parent and passing it to its children. In this implementation we have not used worklist. But it is                     giving best peformance overall on all graph except the Wikipedia-link graph in which case it is taking littlle longer than                other implementations. Command line arguments will be same as ssspv1.cpp except instead of kernels.cl we need to pass                     loopUnrollKernel.cl .
Graph2.hpp : It contains all the imporatant data structures to store and preprocess the graph.

