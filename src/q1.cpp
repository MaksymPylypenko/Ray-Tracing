// Assignment 2 Question 1

#include "common.h"
#include "raytracer.h"
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include <chrono>  // for high_resolution_clock

#define STB_IMAGE_WRITE_IMPLEMENTATION 
#include "utility/stb_image_write.h" // to save rendered images, https://github.com/nothings/stb 
#include <string>

#define M_PI 3.14159265358979323846264338327950288

const char *WINDOW_TITLE = "Ray Tracing";
const double FRAME_RATE_MS = 1;

typedef glm::vec3 point3;
typedef glm::vec3 colour3;

colour3 texture[1<<16]; // big enough for a row of pixels
point3 vertices[2]; // xy+u for start and end of line
GLuint Window;
int vp_width, vp_height;
float drawing_y = 0;

bool antialiasing;
float fov;
point3 eye;
float d = 1;


std::chrono::time_point<std::chrono::high_resolution_clock> start;
std::chrono::time_point<std::chrono::high_resolution_clock> finish;
char* sceneName;



//----------------------------------------------------------------------------
// Taken from here https://github.com/vallentin/GLCollection/blob/master/screenshot.cpp

void flipVertically(int width, int height, char* data)
{
	char rgb[3];

	for (int y = 0; y < height / 2; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			int top = (x + y * width) * 3;
			int bottom = (x + (height - y - 1) * width) * 3;

			memcpy(rgb, data + top, sizeof(rgb));
			memcpy(data + top, data + bottom, sizeof(rgb));
			memcpy(data + bottom, rgb, sizeof(rgb));
		}
	}
}

int saveScreenshot(const char* filename)
{
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	int x = viewport[0];
	int y = viewport[1];
	int width = viewport[2];
	int height = viewport[3];

	char* data = (char*)malloc((size_t)(width * height * 3)); // 3 components (R, G, B)

	if (!data)
		return 0;

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);

	flipVertically(width, height, data);

	int saved = stbi_write_png(filename, width, height, 3, data, 0);

	free(data);

	return saved;
}


int captureScreenshot(std::string savePath)
{
	int saved = saveScreenshot(savePath.c_str());

	if (saved)
		printf("Successfully Saved Image: %s\n", savePath.c_str());
	else
		fprintf(stderr, "Failed Saving Image: %s\n", savePath.c_str());

	return saved;
}

//----------------------------------------------------------------------------

point3 s(int x, int y, float offsetX = 0.0, float offsetY = 0.0 ) {
	float aspect_ratio = (float)vp_width / vp_height;
	float h = d * (float)tan((M_PI * fov) / 180.0 / 2.0);
	float w = h * aspect_ratio;
   
	float top = h;
	float bottom = -h;
	float left = -w;
	float right = w;
   
	float u = left + (right - left) * (x + 0.5f + offsetX) / vp_width;
	float v = bottom + (top - bottom) * (y + 0.5f + offsetY) / vp_height;
   
	return point3(u, v, -d);
}

//----------------------------------------------------------------------------

// OpenGL initialization
void init(char *fn) {
	sceneName = fn;
	loadScene(fn, fov, antialiasing); // Importing to my own data structure! 

	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers( 1, &buffer );
	glBindBuffer( GL_ARRAY_BUFFER, buffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), NULL, GL_STATIC_DRAW );

	// Load shaders and use the resulting shader program
	GLuint program = InitShader( "v.glsl", "f.glsl" );
	glUseProgram( program );

	// set up vertex arrays
	GLuint vPos = glGetAttribLocation( program, "vPos" );
	glEnableVertexAttribArray( vPos );
	glVertexAttribPointer( vPos, 3, GL_FLOAT, GL_FALSE, 0, 0 );

	Window = glGetUniformLocation( program, "Window" );

	// glClearColor( background_colour[0], background_colour[1], background_colour[2], 1 );
	glClearColor( 0.7, 0.7, 0.8, 1 );

	// set up a 1D texture for each scanline of output
	GLuint textureID;
	glGenTextures( 1, &textureID );
	glBindTexture( GL_TEXTURE_1D, textureID );
	glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	
}

//----------------------------------------------------------------------------

void display( void ) {
	// draw one scanline at a time, to each buffer; only clear when we draw the first scanline
	// (when fract(drawing_y) == 0.0, draw one buffer, when it is 0.5 draw the other)
	
	if (drawing_y <= 0.5) {
		if (drawing_y == 0.0) {
			// Record start time
			std::cout << "Starting a timer\n";
			start = std::chrono::high_resolution_clock::now();
		}

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glFlush();
		glFinish();
		glutSwapBuffers();

		drawing_y += 0.5;
	} else if (drawing_y >= 1.0 && drawing_y <= vp_height + 0.5) {
		int y = int(drawing_y) - 1;

		// only recalculate if this is a new scanline
		if (drawing_y == int(drawing_y)) {

			for (int x = 0; x < vp_width; x++) {				

				if (antialiasing)
				{
					float offset = 0.25;
					std::vector<glm::vec3> samples;
					std::vector<glm::vec3> rays;

					glm::vec3 rayDir = s(x, y);
					rays.push_back(normalize(s(x, y, +offset, +offset) - eye));
					rays.push_back(normalize(s(x, y, +offset, -offset) - eye));
					rays.push_back(normalize(s(x, y, -offset, +offset) - eye));
					rays.push_back(normalize(s(x, y, -offset, -offset) - eye));

					for (glm::vec3 rayDir : rays) {
						Ray ray = Ray();
						ray.origin = eye;
						ray.direction = rayDir;
						samples.push_back(
							trace(ray));
					}

					glm::vec3 out;
					for (glm::vec3 colour : samples) {
						out += colour;
					}
					texture[x] = out / (float)samples.size();
				}
				else {
					glm::vec3 rayDir = normalize(s(x, y) - eye);
					Ray ray = Ray();
					ray.origin = eye;
					ray.direction = rayDir;
					texture[x] = trace(ray);
				}
	
				

								
			}

			// to ensure a power-of-two texture, get the next highest power of two
			// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
			unsigned int v; // compute the next highest power of 2 of 32-bit v
			v = vp_width;
			v--;
			v |= v >> 1;
			v |= v >> 2;
			v |= v >> 4;
			v |= v >> 8;
			v |= v >> 16;
			v++;
			
			glTexImage1D( GL_TEXTURE_1D, 0, GL_RGB, v, 0, GL_RGB, GL_FLOAT, texture );
			vertices[0] = point3(0, y, 0);
			vertices[1] = point3(v, y, 1);
			glBufferSubData( GL_ARRAY_BUFFER, 0, 2 * sizeof(point3), vertices);
		}

		glDrawArrays( GL_LINES, 0, 2 );
		
		glFlush();
		glFinish();
		glutSwapBuffers();
		
		drawing_y += 0.5;
	}

	if (drawing_y == vp_height) {
		// Record end time
		finish = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = finish - start;
		std::cout << "Elapsed time: " << elapsed.count() << " s\n";
		std::string savePath = "../rendered/";
		
		savePath.append("[");
		savePath.append(std::to_string(vp_width));
		savePath.append("x");
		savePath.append(std::to_string(vp_height));
		savePath.append("] ");
		savePath.append(sceneName);
		if (antialiasing) {
			savePath.append(" (x4) ");
		}
		else {
			savePath.append(" ");
		}		
		savePath.append(std::to_string((int)round(elapsed.count())));
		
		savePath.append(" sec.png");
		captureScreenshot(savePath);
	}	
}

//----------------------------------------------------------------------------

void keyboard( unsigned char key, int x, int y ) {
	switch( key ) {
	case 033: // Escape Key
	case 'q': case 'Q':
		exit( EXIT_SUCCESS );
		break;
	case ' ':
		drawing_y = 1;
		break;
	}
}

//----------------------------------------------------------------------------

void mouse( int button, int state, int x, int y ) {
	y = vp_height - y - 1;
	if ( state == GLUT_DOWN ) {
		switch( button ) {
		case GLUT_LEFT_BUTTON:
			colour3 c;
			point3 uvw = s(x, y);
			std::cout << "\n\nCASTING A RAY\n\n";
			glm::vec3 rayDir = normalize(s(x, y) - eye);
			Ray ray = Ray();
			ray.origin = eye;
			ray.direction = rayDir;
			ray.debugOn = true;
			trace(ray);			
			break;
		}
	}
}

//----------------------------------------------------------------------------

void update( void ) {
}

//----------------------------------------------------------------------------

void reshape( int width, int height ) {
	glViewport( 0, 0, width, height );

	// GLfloat aspect = GLfloat(width)/height;
	// glm::mat4  projection = glm::ortho( -aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f );
	// glUniformMatrix4fv( Projection, 1, GL_FALSE, glm::value_ptr(projection) );
	vp_width = width;
	vp_height = height;
	glUniform2f( Window, width, height );
	drawing_y = 0;
}
