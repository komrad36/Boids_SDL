/*******************************************************************
*   Boids.cpp
*   Boids
*	Kareem Omar
*
*	6/18/2015
*   This program is entirely my own work.
*******************************************************************/

// Boids simulates bird ("boid"), fish, crowd, etc. flocking behavior,
// resulting in emergent properties. The main focus of the project is
// showcasing high performance computing, so I refer you to the version
// titled Boids_CUDA_GL, which, as the name implies, uses CUDA for
// computation and then leverages CUDA-OpenGL interoperability to
// write output directly to an OpenGL texture for rendering, with
// no memory transfers or CPU involvement whatsoever. This causes
// epic performance.
//
// This is the multithreaded CPU version. As this is a computationally
// intensive problem with naive neighbor search, this will be slow
// at more than a few thousand birds.
//
// The simulation can be sped up using a k-d tree or neighborhood
// matrix, solutions I implement in the CUDA version. Again, this
// CPU version is a concept demo for machines without nVIDIA graphics.
//
// This simulation
// is primarily tuned for aesthetics, not physical accuracy, although
// careful selection of parameters can produce very flock-like emergent
// behavior. The color of each boid is determined by its direction of travel.
// Screen-wrapping and fullscreen are available as options in params.h as
// well as a variety of simulation parameters.
//
// Requires SDL and SDL_ttf.
//
// Commands:
//	Space			-	toggle screen blanking
//	P				-	pause
//	R				-	randomize boid positions
//	LCTRL			-	switch between mouse attraction and repulsion
//	LSHIFT			-	toggle STRONG attraction/repulsion
//	ESC				-	quit
//	Hold mouse btn	-	enable attraction/repulsion to mouse

#include "Boids.h"

// random number generator for boid initial positions
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> positionRandomDist(0.0, P_MAX);

#ifdef DYNAMIC_COLOR_MODE
RGB angleToRGB(const float angle) {
	// mult by 3/pi, equivalent to dividing by (pi/3) (60 degrees)
	float section = angle * THREE_OVER_PI;
	int i = static_cast<int>(section);
	float frac = section - i;

	uint8_t HSV_to_RGB[6] = { MAX_RGB, static_cast<uint8_t>(fMAX_RGB - fMAX_RGB*frac), 0, 0, static_cast<uint8_t>(fMAX_RGB*frac), MAX_RGB };

	// assuming max V and H, we can get RGB quickly by rotating them around, each separated by 60 degrees,
	// which we do quickly by using the 60 degree sector of the normalized velocity vector
	// to index into the HSV_to_RGB array accordingly.
	return RGB(HSV_to_RGB[i], HSV_to_RGB[(i + 4) % NUM_HSV_SECTORS], HSV_to_RGB[(i + 2) % NUM_HSV_SECTORS]);
}
#endif

#ifdef SCREEN_WRAP
float diff(const float c1, const float c2) {
	float direct_distance = c2 - c1;
	float wrap_distance = (c2 > c1) ? direct_distance - fP_MAX : direct_distance + fP_MAX;

	return fabsf(direct_distance) < fHALF_P_MAX ? direct_distance : wrap_distance;
}

float fastdiff(const float c1, const float c2) {
	float direct_distance = fabs(c2 - c1);

	return (direct_distance < fHALF_P_MAX) ? direct_distance : fP_MAX - direct_distance;
}

float fastdiffToDiff(const float fast_diff, const float c1, const float c2) {
	return ((fabs(c2 - c1) < fHALF_P_MAX) ^ (c2 < c1)) ? fast_diff : -fast_diff;
}
#endif

void Physics::launchThread(int start_idx, int end_idx) {
	float diffx, diffy, x, y, Vx, Vy, modVx, modVy, modVmag, magVsquared, fMouseX, fMouseY;
	float time_factor, factor, CMsumX, CMsumY, REPsumX, REPsumY, ALsumX, ALsumY;
	int neighbors, boid, test_boid;

	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

	for (;;) {
		for (boid = start_idx; boid <= end_idx; ++boid) {

			// bring boid's position and velocity local
			x = in[boid].x;
			y = in[boid].y;
			Vx = in[boid].vx;
			Vy = in[boid].vy;

			time_factor = TICK_FACTOR * time_since_last_frame;

			// apply mouse attraction/repulsion rules
			if (mouse_buttons_down) {
				fMouseX = static_cast<float>(mouse_x * P_MAX) / fWidth;
				fMouseY = static_cast<float>(mouse_y * P_MAX) / fHeight;
#ifdef SCREEN_WRAP
				diffx = diff(x, fMouseX);
				diffy = diff(y, fMouseY);
#else
				diffx = fMouseX - x;
				diffy = fMouseY - y;
#endif

				// We update the velocity components by a factor proportional to time elapsed
				// and ratio of component distance to the cursor to the total distance to the cursor
				// for a natural-looking attraction model
				factor = ((mouse_buttons_down) ? (repulsion_multiplier * ((repulsion_boost) ? STRONG_DOWN_STRENGTH_FACTOR : WEAK_MOUSE_DOWN_STRENGTH_FACTOR)) : 0.0f) / (sqrt(diffx * diffx + diffy * diffy) + PREVENT_ZERO_RETURN);
				Vx += time_factor * diffx * factor;
				Vy += time_factor * diffy * factor;
			}

			// apply neighbor-related rules for every other boid that's a neighbor
			CMsumX = 0.0f; CMsumY = 0.0f; REPsumX = 0.0f; REPsumY = 0.0f; ALsumX = 0.0f; ALsumY = 0.0f;

			neighbors = 0;
			for (test_boid = 0; test_boid < NUMBER_OF_BOIDS; ++test_boid) {

#ifdef SCREEN_WRAP
				diffx = fastdiff(x, in[test_boid].x);
				diffy = fastdiff(y, in[test_boid].y);
#else
				diffx = in[test_boid].x - x;
				diffy = in[test_boid].y - y;
#endif
				// to optimize we don't branch on whether neighbor is self,
				// which means we will always be counted as our own neighbor
				// The only rule this affects is alignment (the others go to
				// 0 due to distance being 0) and we deal with that later...
				if (diffx * diffx + diffy * diffy < NEIGHBOR_DISTANCE_SQUARED) {

#ifdef SCREEN_WRAP
					diffx = fastdiffToDiff(diffx, x, in[test_boid].x);
					diffy = fastdiffToDiff(diffy, y, in[test_boid].y);
#endif

					// update center of mass rule by distance and direction to neigbor
					CMsumX += diffx;
					CMsumY += diffy;

					factor = 1.0f / (diffx*diffx + diffy*diffy + PREVENT_ZERO_RETURN);
					// update repulsion rule by ratio of component distance to square of total distance to neighbor
					// for natural repulsion model
					REPsumX -= diffx * factor;
					REPsumY -= diffy * factor;

					// update alignment rule by component velocity of neighbor
					ALsumX += in[test_boid].vx;
					ALsumY += in[test_boid].vy;

					// keep track of total neighbor count for averaging these rule sums
					++neighbors;
				}
			}
#ifdef SCREEN_WRAP
			// okay, this is a fun one. We update the velocity component by the time factor multiplied by the center of mass average, which is the center of mass sum computed
			// in the loop above, divided by the number of neighbors.
			// We do the same with repulsion and alignment (there we must subtract our own velocity as it's the only rule affected by the fact that we chose to not
			// check whether the test boid is distinct (for speed), and thus count ourselves as a neighbor.
			Vx += time_factor * (CMsumX * CENTER_OF_MASS_STRENGTH_FACTOR / neighbors + REPULSION_STRENGTH_FACTOR * REPsumX + (ALsumX - in[boid].vx) * ALIGNMENT_STRENGTH_FACTOR / neighbors);
			Vy += time_factor * (CMsumY * CENTER_OF_MASS_STRENGTH_FACTOR / neighbors + REPULSION_STRENGTH_FACTOR * REPsumY + (ALsumY - in[boid].vx) * ALIGNMENT_STRENGTH_FACTOR / neighbors);
#else
			// the same occurs with screenwrap off as the above description, with one change: now repulsion also includes
			// a term for repelling off the edges of the screen, if within range, inversely proportional to distance from edge
			Vx += time_factor * (CMsumX * CENTER_OF_MASS_STRENGTH_FACTOR / neighbors + REPULSION_STRENGTH_FACTOR * (REPsumX + EDGE_REPULSION_STRENGTH_FACTOR*(x < NEIGHBOR_DISTANCE)*(NEIGHBOR_DISTANCE - x) - EDGE_REPULSION_STRENGTH_FACTOR*(x > fP_MAX - NEIGHBOR_DISTANCE)*(x - (fP_MAX - NEIGHBOR_DISTANCE))) + (ALsumX - in[boid].vx) * ALIGNMENT_STRENGTH_FACTOR / neighbors);
			Vy += time_factor * (CMsumY * CENTER_OF_MASS_STRENGTH_FACTOR / neighbors + REPULSION_STRENGTH_FACTOR * (REPsumY + EDGE_REPULSION_STRENGTH_FACTOR*(y < NEIGHBOR_DISTANCE)*(NEIGHBOR_DISTANCE - y) - EDGE_REPULSION_STRENGTH_FACTOR*(y > fP_MAX - NEIGHBOR_DISTANCE)*(y - (fP_MAX - NEIGHBOR_DISTANCE))) + (ALsumY - in[boid].vx) * ALIGNMENT_STRENGTH_FACTOR / neighbors);
#endif

			// limit velocity if over V_LIM
			magVsquared = Vx*Vx + Vy*Vy;
			factor = (magVsquared > V_LIM_2) ? V_LIM / sqrt(magVsquared) : 1.0f;
			Vx *= factor;
			Vy *= factor;

#ifdef SCREEN_WRAP
			// update position...
			x += Vx * time_factor;
			y += Vy * time_factor;
			// ...then screenwrap it and store the result both to
			// the local component and to the global out array
			out[boid].x = (x += fP_MAX*((x < 0.0f) - (x >= fP_MAX)));
			out[boid].y = (y += fP_MAX*((y < 0.0f) - (y >= fP_MAX)));
#else
			// if not screenwrapping,
			// adjust the sign of the velocity of any boid outside the box
			// so it's heading inside again in case that wasn't handled
			// by the repulsion force.
			//
			// do NOT just bring its position to some value like 0.0f
			// because then multiple boids would collide (share the exact
			// same position) and thus might move together in future
			// if their velocities also match (as they might well - reduced
			// to a zero or V_LIM equilibrium in a corner, say)...
			if (x < 0.0f) Vx = fabs(Vx);
			if (x >= fP_MAX) Vx = -fabs(Vx);
			if (y < 0.0f) Vy = fabs(Vy);
			if (y >= fP_MAX) Vy = -fabs(Vy);

			// ...and THEN update position so we move back inside
			// without looking too unnaturally bounded
			x += Vx * time_factor;
			y += Vy * time_factor;

			out[boid].x = x;
			out[boid].y = y;
#endif

			// store velocities back to global
			out[boid].vx = Vx;
			out[boid].vy = Vy;

			// return to screen reference frame
			x = fWidth * x / fP_MAX;
			y = fHeight * y / fP_MAX;

			// convert vel to screen frame, off by a constant fP_MAX
			// (that's okay because we're about to normalize)
			modVx = fWidth*Vx;
			modVy = fHeight*Vy;
			modVmag = 1.0f / sqrt(modVx*modVx + modVy*modVy + PREVENT_ZERO_RETURN);
			Vx = modVx * modVmag;
			Vy = modVy * modVmag;

#ifdef DYNAMIC_COLOR_MODE
			out[boid].color = angleToRGB(atan2f(Vx, Vy) + fPI);
#endif

			out[boid].draw_x1 = static_cast<int>(x - LINE_LENGTH * Vx + 0.5f);
			out[boid].draw_y1 = static_cast<int>(y - LINE_LENGTH * Vy + 0.5f);
			out[boid].draw_x2 = static_cast<int>(x + LINE_LENGTH * Vx + 0.5f);
			out[boid].draw_y2 = static_cast<int>(y + LINE_LENGTH * Vy + 0.5f);

		} // dynamic thread reassign
		mtx.lock();
		if (cur_idx >= NUMBER_OF_BOIDS) {
			mtx.unlock();
			return;
		}
		start_idx = cur_idx;
		cur_idx += (NUMBER_OF_BOIDS - cur_idx) / num_CPU / 2 + 1;
		end_idx = cur_idx - 1;
		mtx.unlock();
	}
}

void Physics::initThreads() {
#ifdef OVERRIDE_CPU_COUNT_AUTODETECT
	num_CPU = OVERRIDE_CPU_COUNT_AUTODETECT;
#else
	num_CPU = std::min(static_cast<unsigned int>(NUMBER_OF_BOIDS), std::thread::hardware_concurrency());
#endif

	threads = new std::thread[num_CPU];
}

void Physics::spawnBoids() {
	// generate host-side random initial positions
	for (int i = 0; i < NUMBER_OF_BOIDS; ++i) {
		in[i].vx = in[i].vy = 0.0f;

		in[i].x = positionRandomDist(gen);
		in[i].y = positionRandomDist(gen);
	}
}

void Physics::processRules() {
	int start_idx;

	// send out threads, each with half the naive workload,
	// i.e. for 8 cores, give each 1/16 of the work to start
	// with. This way, when some return before others, they
	// can be dynamically assigned more work (occurs in the
	// 'launchThread' method, mutexed by mtx) in smaller and
	// smaller increments such that at the end, all threads
	// finish up the last few boids at the same time
	mtx.lock();
	for (int i = 0; i < num_CPU; ++i) {
		start_idx = cur_idx;
		cur_idx += NUMBER_OF_BOIDS / num_CPU / 2 + 1;
		threads[i] = std::thread(&Physics::launchThread, this, start_idx, cur_idx - 1);
	}
	mtx.unlock();

	// wait for all threads to return
	for (int i = 0; i < num_CPU; ++i) {
		threads[i].join();
	}

	// reset for next frame
	cur_idx = 0;

}