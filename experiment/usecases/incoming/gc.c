/*
 * gc.c
 *
 *  Created on: 2017. 07. 15.
 *      Author: szupervigyor
 */

/*

Goal:
	- create a cyclic refrenced data structuire from LxcValues
		(programically or with LXC)

	- release the root reference
	- run the GC manually (as first solution)
	- enjoy the grabage free heap. (sounds good, right?)
		(ensure all value freed.)

TODO: wite (especially) concurrency tests to cdll_* functions
TODO: involve LxcValueContainer and indirect manager function like
		lxc_vc_import, etc.
 */

