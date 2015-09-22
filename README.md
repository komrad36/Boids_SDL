 Please see screenshot examples in master branch 'Screenshots' folder!
 
 Boids simulates bird ("boid"), fish, crowd, etc. flocking behavior,
 resulting in emergent properties. The main focus of the project is
 showcasing high performance computing, so I refer you to the version
 titled Boids_CUDA_GL, also on my GitHub, which, as the name implies,
 uses CUDA for computation and then leverages CUDA-OpenGL interoperability to
 write output directly to an OpenGL texture for rendering, with
 no memory transfers or CPU involvement whatsoever. This causes
 epic performance.

 This is the multithreaded CPU version. It showcases HPC,
 multithreading, OpenGL, and numerical simulation. As this is a
 computationally intensive problem with naive neighbor search,
 this will be slow at more than a few thousand birds.

 The simulation can be sped up using a k-d tree or neighborhood
 matrix, solutions I implement in the CUDA version. Again, this
 CPU version is a concept demo for machines without nVIDIA graphics.

 This simulation is primarily tuned for aesthetics, not physical accuracy,
 although careful selection of parameters can produce very flock-like emergent
 behavior. The color of each boid is determined by its direction of travel.
 Screen-wrapping and fullscreen are available as options in params.h as
 well as a variety of simulation parameters.

 Requires SDL and SDL_ttf.

 Commands:
 
	Space           -	toggle screen blanking
	
	P               -	pause
	
	R               -	randomize boid positions
	
	PRINT_SCREEN    -	save screenshot to <current time>.bmp so you don't have to quit or ALT-TAB to do so
	
	LCTRL           -	switch between mouse attraction and repulsion
	
	LSHIFT          -	toggle STRONG attraction/repulsion
	
	ESC             -	quit
	
	Hold mouse btn	-	enable attraction/repulsion to mouse
	

Please see screenshot examples in master branch 'Screenshots' folder!
