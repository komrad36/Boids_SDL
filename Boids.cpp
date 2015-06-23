/*******************************************************************
*   Boids.cpp
*   Boids
*	Kareem Omar
*
*	6/18/2015
*   This program is entirely my own work.
*******************************************************************/

// This module contains the code for the Boid math and rendering.


#include "Boids.h"

// Random number generator for random initial positions
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> positionRandomDist(0.0, P_MAX);

// Current boid to process, for threading
unsigned int curIndex = 0;

// for blocking curIndex
std::mutex indexUpdateMutex;

float x_in_array[NUMBER_OF_BOIDS];
float y_in_array[NUMBER_OF_BOIDS];
float vx_in_array[NUMBER_OF_BOIDS];
float vy_in_array[NUMBER_OF_BOIDS];

float x_out_array[NUMBER_OF_BOIDS];
float y_out_array[NUMBER_OF_BOIDS];
float vx_out_array[NUMBER_OF_BOIDS];
float vy_out_array[NUMBER_OF_BOIDS];

float *x_in = x_in_array;
float *y_in = y_in_array;
float *vx_in = vx_in_array;
float *vy_in = vy_in_array;

float *x_out = x_out_array;
float *y_out = y_out_array;
float *vx_out = vx_out_array;
float *vy_out = vy_out_array;

std::thread *threads;

// Simplified HSV to RGB with S and V both 100%
RGB angleToRGB(float angle) {
	float       q, ff;
	long        i;
	RGB         out;

	//if (angle >= 360.0) angle = 0.0;
	angle /= 60.0f;
	i = (long)angle;
	ff = angle - i;
	q = 1.0f - ff;

	switch (i) {
	case 0:
		out.R = 255U;
		out.G = (uint8_t)(255.0f * ff);
		out.B = 0U;
		break;
	case 1:
		out.R = (uint8_t)(255.0f * q);
		out.G = 255U;
		out.B = 0U;
		break;
	case 2:
		out.R = 0U;
		out.G = 255U;
		out.B = (uint8_t)(255.0f * ff);
		break;
	case 3:
		out.R = 0U;
		out.G = (uint8_t)(255.0f * q);
		out.B = 255U;
		break;
	case 4:
		out.R = (uint8_t)(255.0f * ff);
		out.G = 0U;
		out.B = 255U;
		break;
	default:
		out.R = 255U;
		out.G = 0U;
		out.B = (uint8_t)(255.0f * q);
	}
	return out;
}

// Return shortest distance between coordinates c1 to c2,
// screenwrapping if necessary, and preserving direction
// of travel to get there.
inline float diff(const float c1, const float c2) {
	float direct_distance = c2 - c1;
	float wrap_distance = direct_distance + ((c2 > c1) ? -P_MAX : P_MAX);

	// figure out whether a direct path or one that wraps around the screen is shorter
	// but sign must be preserved to provide direction-to-destination info to boid

	return (fabs(direct_distance) < fabs(wrap_distance)) ? direct_distance : wrap_distance;

	//return c2 - c1;
}

// Return shortest distance between coordinates c1 to c2,
// screenwrapping if necessary, *independent* of direction
// fast as hell. For uses that only want magnitude
inline float fastdiff(const float c1, const float c2) {
	float direct_distance = fabs(c2 - c1);

	return (direct_distance < HALF_fP_MAX) ? direct_distance : fP_MAX - direct_distance;

	//return c2 - c1;
}

// Shortcut to add sign info to fastdiff to produce equivalent
// results as diff
inline float fastdiffToDiff(const float fastDiff, const float c1, const float c2) {
	return ((fabs(c2 - c1) < HALF_fP_MAX) ^ (c2 < c1)) ? fastDiff : -fastDiff;
}

float crazyStupidFastSqrtApprox(const float x) {
	int iX = (*(int*)&x >> 1) + 0x1FBD3EE7;
	return *(float*)&iX;
}

void launchThread(unsigned int startIndex, unsigned int endIndex) {
	float diffx, diffy, x, y, Vx, Vy, modVx, modVy, modVmag, magVsquared, fMouseX, fMouseY;
	float timeFactor, factor, CMsumX, CMsumY, REPsumX, REPsumY, ALsumX, ALsumY;
	unsigned int drawPos, neighbors, boid, test_boid;

	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

	for (;;) {
		for (boid = startIndex; boid <= endIndex; ++boid) {

			x = x_in[boid];
			y = y_in[boid];
			Vx = vx_in[boid];
			Vy = vy_in[boid];

			// apply mouse-related rule
			fMouseX = (float)(mousexpos * P_MAX) / fWidth;
			fMouseY = (float)(mouseypos * P_MAX) / fHeight;
			timeFactor = time_since_last_frame / TICK_DIVISOR;
			diffx = diff(x, fMouseX);
			diffy = diff(y, fMouseY);
			factor = ((mouse_buttons_down) ? (repulsion_multiplier * ((repulsion_boost) ? STRONG_DOWN_STRENGTH_FACTOR : WEAK_MOUSE_DOWN_STRENGTH_FACTOR)) : 0.0f) / (sqrt(diffx * diffx + diffy * diffy) + 0.1f);
			Vx += timeFactor * diffx * factor;
			Vy += timeFactor * diffy * factor;

			// apply neighbor-related rules for every other boid that's a neighbor
			CMsumX = 0.0f; CMsumY = 0.0f; REPsumX = 0.0f; REPsumY = 0.0f; ALsumX = 0.0f; ALsumY = 0.0f;

			neighbors = 0;
			for (test_boid = 0; test_boid < NUMBER_OF_BOIDS; ++test_boid) {

				diffx = fastdiff(x, x_in[test_boid]);
				diffy = fastdiff(y, y_in[test_boid]);
				if (diffx * diffx + diffy * diffy < NEIGHBOR_DISTANCE_SQUARED) {
					++neighbors;

					diffx = fastdiffToDiff(diffx, x, x_in[test_boid]);
					diffy = fastdiffToDiff(diffy, y, y_in[test_boid]);

					//diffx = diff(x, x_in[test_boid]);
					//diffy = diff(y, y_in[test_boid]);
					
					CMsumX += diffx;
					CMsumY += diffy;

					factor = diffx*diffx + diffy*diffy + PREVENT_ZERO_RETURN;
					REPsumX -= diffx / factor;
					REPsumY -= diffy / factor;

					ALsumX += vx_in[test_boid];
					ALsumY += vy_in[test_boid];
				}
			}
			Vx += timeFactor * CMsumX / ((float)neighbors * CENTER_OF_MASS_STRENGTH_DIVISOR) + REPULSION_STRENGTH_FACTOR * REPsumX + (ALsumX - vx_in[boid]) / (float)(neighbors * ALIGNMENT_STRENGTH_DIVISOR);
			Vy += timeFactor * CMsumY / ((float)neighbors * CENTER_OF_MASS_STRENGTH_DIVISOR) + REPULSION_STRENGTH_FACTOR * REPsumY + (ALsumY - vy_in[boid]) / (float)(neighbors * ALIGNMENT_STRENGTH_DIVISOR);
			// corrected for the fact that own boid was counted as neighbor
			
			// limit velocity if over V_LIM
			magVsquared = Vx*Vx + Vy*Vy;
			factor = (magVsquared > V_LIM_2) ? V_LIM / sqrt(magVsquared) : 1.0f;
			Vx *= factor;
			Vy *= factor;

			// tick
			x += Vx * timeFactor;
			y += Vy * timeFactor;

			// screen wrap
			if (x > fP_MAX)
				x -= fP_MAX;
			if (x < 0.0f)
				x += fP_MAX;
			if (y > fP_MAX)
				y -= fP_MAX;
			if (y < 0.0f)
				y += fP_MAX;

			// copyback
			x_out[boid] = x;
			y_out[boid] = y;
			vx_out[boid] = Vx;
			vy_out[boid] = Vy;

			// return to screen reference frame
			x = fWidth * x / fP_MAX;
			y = fHeight * y / fP_MAX;

			// convert vel to unit vectors
			modVx = fWidth*Vx / fP_MAX;
			modVy = fHeight*Vy / fP_MAX;
			modVmag = sqrt(modVx*modVx + modVy*modVy + PREVENT_ZERO_RETURN);
			Vx = modVx / modVmag;
			Vy = modVy / modVmag;

#ifdef DYNAMIC_COLOR_MODE
			colors[boid] = angleToRGB(180.0f / (float)M_PI * atan2f(Vx, Vy) + 180.0f);
#endif

			drawPos = 4 * boid;
			drawingPoints[drawPos] = (int)(x - LINE_LENGTH * Vx + 0.5f);
			drawingPoints[drawPos + 1] = (int)(y - LINE_LENGTH * Vy + 0.5f);
			drawingPoints[drawPos + 2] = (int)(x + LINE_LENGTH * Vx + 0.5f);
			drawingPoints[drawPos + 3] = (int)(y + LINE_LENGTH * Vy + 0.5f);

		} // dynamic thread reassign
		indexUpdateMutex.lock();
		if (curIndex >= NUMBER_OF_BOIDS) {
			indexUpdateMutex.unlock();
			return;
		}
		startIndex = curIndex;
		curIndex += (NUMBER_OF_BOIDS - curIndex) / numCPU / 2 + 1;
		endIndex = curIndex - 1;
		indexUpdateMutex.unlock();
	}
}

void initThreads() {
	// get number of logical cores
#ifdef OVERRIDE_CPU_COUNT_AUTODETECT
	numCPU = OVERRIDE_CPU_COUNT_AUTODETECT;
#else
	numCPU = std::min((unsigned int)NUMBER_OF_BOIDS, std::thread::hardware_concurrency());
#endif

	threads = new std::thread[numCPU];
}

void spawnBoids() {
	// generate host-side random initial positions
	for (unsigned int i = 0; i < NUMBER_OF_BOIDS; ++i) {
		vx_in[i] = 0.0f;
		vy_in[i] = 0.0f;

		x_in[i] = positionRandomDist(gen);
		y_in[i] = positionRandomDist(gen);
	}
}

void process_rules() {
	unsigned int startIndex;

	// spawn threads
	indexUpdateMutex.lock();
	for (unsigned int i = 0; i < numCPU; ++i) {
		startIndex = curIndex;
		curIndex += NUMBER_OF_BOIDS / numCPU / 2 + 1;
		threads[i] = std::thread(launchThread, startIndex, curIndex - 1);
	}
	indexUpdateMutex.unlock();

	for (unsigned int i = 0; i < numCPU; ++i)
		threads[i].join();

	// reset for next frame
	curIndex = 0;

	//// the pretty way to do it...
	//std::swap(d_vx_in, d_vx_out);
	//std::swap(d_vy_in, d_vy_out);
	//std::swap(d_x_in, d_x_out);
	//std::swap(d_y_in, d_y_out);

	// ...but this is ever-so-slightly faster
	auto temp = x_in;
	x_in = x_out;
	x_out = temp;

	temp = y_in;
	y_in = y_out;
	y_out = temp;

	temp = vx_in;
	vx_in = vx_out;
	vx_out = temp;

	temp = vy_in;
	vy_in = vy_out;
	vy_out = temp;
}

void deInitBoids() {
	delete[] threads;
}