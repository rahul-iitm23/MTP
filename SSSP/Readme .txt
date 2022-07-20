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
