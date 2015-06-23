# Boids_SDL
Numerical simulation of flocking behavior using pure CPU and SDL.

Showcases HPC, hand-rolled threading engine, SDL, C++. Demonstrates emergent behavior in flocks. Pretty colors.

Definitely a work in progress. Current key commands are not visible to end user. Will be improved in future
as well as integrated with the CUDA version.

Until then, keys are:
Space - toggle screen blanking
R     - randomize boid locations
C     - pause/unpause
LShift- toggle increased attraction/repulsion strength
LCtrl - toggle attraction/repulsion
PrtScn- save screenshot to CWD
Esc   - quit

Requires SDL.

Verisons of this are also available (not currently uploaded to github) as pure python (very slow!) and as a SWIG Python-C
hybrid, where Python handles front-end but calls a multithreaded C engine for the math, showcasing interoperability. Contact
me if interested.

Please see screenshot demos in master branch!
