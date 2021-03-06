// tag::C++11check[]
#define STRING2(x) #x
#define STRING(x) STRING2(x)

#if __cplusplus < 201103L
	#pragma message("WARNING: the compiler may not be C++11 compliant. __cplusplus version is : " STRING(__cplusplus))
#endif
// end::C++11check[]

// tag::includes[]
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <string>

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define GLM_FORCE_RADIANS // suppress a warning in GLM 0.9.5
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
// end::includes[]

// tag::using[]
// see https://isocpp.org/wiki/faq/Coding-standards#using-namespace-std
// don't use the whole namespace, either use the specific ones you want, or just type std::
using std::cout;
using std::cerr;
using std::endl;
using std::max;
using std::string;
// end::using[]


// tag::globalVariables[]
std::string exeName;
SDL_Window *win; //pointer to the SDL_Window
SDL_GLContext context; //the SDL_GLContext
int frameCount = 0;
std::string frameLine = "";
// end::globalVariables[]

// tag::loadShader[]
std::string loadShader(const string filePath) {
    std::ifstream fileStream(filePath, std::ios::in | std::ios::binary);
	if (fileStream)
	{
		string fileData( (std::istreambuf_iterator<char>(fileStream)),
		                 (std::istreambuf_iterator<char>()          ));

		cout << "Shader Loaded from " << filePath << endl;
		return fileData;
	}
	else
	{
        cerr << "Shader could not be loaded - cannot read file " << filePath << ". File does not exist." << endl;
        return "";
	}
}
// end::loadShader[]

//our variables
bool done = false;

// tag::vertexData[]
//the data about our geometry
const GLfloat vertexData[] = {
//	 X        Y          R     G     B      A       U     V
	 0.000f,  0.500f,    1.0f, 0.0f, 0.0f,  1.0f,   0.5f, 1.0f,
	-0.433f, -0.250f,    0.0f, 1.0f, 0.0f,  1.0f,   0.0f, 0.0f,
	 0.433f, -0.250f,    0.0f, 0.0f, 1.0f,  1.0f,   1.0f, 0.0f,
};
// end::vertexData[]

// tag::translationVectors[]
//the translation vector we'll pass to our GLSL program
glm::vec2 translationVector1 = { -0.5f, -0.5f };
glm::vec2 translationVelocityVector1 = { 0.1f, 0.1f};

glm::vec2 translationVector2 = { 0.8f, -0.5f };
glm::vec2 translationVelocityVector2 = { -0.2f, 0.15f};
// end::translationVectors[]

// tag::GLVariables[]
//our GL and GLSL variables
GLuint theProgram;
GLint positionLocation;
GLint vertexColorLocation;
GLint vertexUVLocation;

GLint textureSamplerLocation;
GLint translationVectorLocation;

GLuint textureID;

GLuint vertexDataBufferObject;
GLuint vertexArrayObject;

// end::GLVariables[]



// end Global Variables
/////////////////////////

// tag::initialise[]
void initialise()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
		cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
		exit(1);
	}

	cout << "SDL initialised OK!\n";
}
// end::initialise[]

// tag::createWindow[]
void createWindow()
{
	//get executable name, and use as window title
	int beginIdxWindows = exeName.rfind("\\"); //find last occurrence of a backslash
	int beginIdxLinux = exeName.rfind("/"); //find last occurrence of a forward slash
	int beginIdx = max(beginIdxWindows, beginIdxLinux);
	std::string exeNameEnd = exeName.substr(beginIdx + 1);
	const char *exeNameCStr = exeNameEnd.c_str();

	//create window
	win = SDL_CreateWindow(exeNameCStr, 100, 100, 600, 600, SDL_WINDOW_OPENGL); //same height and width makes the window square ...

	//error handling
	if (win == nullptr)
	{
		std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		exit(1);
	}
	cout << "SDL CreatedWindow OK!\n";
}
// end::createWindow[]

// tag::setGLAttributes[]
void setGLAttributes()
{
	int major = 3;
	int minor = 3;
	cout << "Built for OpenGL Version " << major << "." << minor << endl; //ahttps://en.wikipedia.org/wiki/OpenGL_Shading_Language#Versions
	// set the opengl context version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); //core profile
	cout << "Set OpenGL context to versicreate remote branchon " << major << "." << minor << " OK!\n";
}
// tag::setGLAttributes[]

// tag::createContext[]
void createContext()
{
	setGLAttributes();

	context = SDL_GL_CreateContext(win);
	if (context == nullptr){
		SDL_DestroyWindow(win);
		std::cerr << "SDL_GL_CreateContext Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		exit(1);
	}
	cout << "Created OpenGL context OK!\n";
}
// end::createContext[]

// tag::initGlew[]
void initGlew()
{
	GLenum rev;
	glewExperimental = GL_TRUE; //GLEW isn't perfect - see https://www.opengl.org/wiki/OpenGL_Loading_Library#GLEW
	rev = glewInit();
	if (GLEW_OK != rev){
		std::cerr << "GLEW Error: " << glewGetErrorString(rev) << std::endl;
		SDL_Quit();
		exit(1);
	}
	else {
		cout << "GLEW Init OK!\n";
	}
}
// end::initGlew[]

// tag::createShader[]
GLuint createShader(GLenum eShaderType, const std::string &strShaderFile)
{
	GLuint shader = glCreateShader(eShaderType);
	//error check
	const char *strFileData = strShaderFile.c_str();
	glShaderSource(shader, 1, &strFileData, NULL);

	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

		const char *strShaderType = NULL;
		switch (eShaderType)
		{
		case GL_VERTEX_SHADER: strShaderType = "vertex"; break;
		case GL_GEOMETRY_SHADER: strShaderType = "geometry"; break;
		case GL_FRAGMENT_SHADER: strShaderType = "fragment"; break;
		}

		fprintf(stderr, "Compile failure in %s shader:\n%s\n", strShaderType, strInfoLog);
		delete[] strInfoLog;
	}

	return shader;
}
// end::createShader[]

// tag::createProgram[]
GLuint createProgram(const std::vector<GLuint> &shaderList)
{
	GLuint program = glCreateProgram();

	for (size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glAttachShader(program, shaderList[iLoop]);

	glLinkProgram(program);

	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
	}

	for (size_t iLoop = 0; iLoop < shaderList.size(); iLoop++)
		glDetachShader(program, shaderList[iLoop]);

	return program;
}
// end::createProgram[]

// tag::initializeProgram[]
void initializeProgram()
{
	std::vector<GLuint> shaderList;

	shaderList.push_back(createShader(GL_VERTEX_SHADER, loadShader("vertexShader.glsl")));
	shaderList.push_back(createShader(GL_FRAGMENT_SHADER, loadShader("fragmentShader.glsl")));

	theProgram = createProgram(shaderList);
	if (theProgram == 0)
	{
		cerr << "GLSL program creation error." << std::endl;
		SDL_Quit();
		exit(1);
	}
	else {
		cout << "GLSL program creation OK! GLUint is: " << theProgram << std::endl;
	}

	// tag::getLocations[]
	positionLocation = glGetAttribLocation(theProgram, "position");
	vertexColorLocation = glGetAttribLocation(theProgram, "vertexColor");
	vertexUVLocation = glGetAttribLocation(theProgram, "vertexUV");

	translationVectorLocation = glGetUniformLocation(theProgram, "translationVector");
	textureSamplerLocation = glGetUniformLocation(theProgram, "textureSampler");
	// end::getLocations[]

	//clean up shaders (we don't need them anymore as they are no in theProgram
	for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
}
// end::initializeProgram[]

// tag::initializeVertexArrayObject[]
void initializeVertexArrayObject()
{
	glGenVertexArrays(1, &vertexArrayObject);
	cout << "Vertex Array Object created OK! GLUint is: " << vertexArrayObject << std::endl;

	glBindVertexArray(vertexArrayObject);

		glBindBuffer(GL_ARRAY_BUFFER, vertexDataBufferObject);

		glEnableVertexAttribArray(positionLocation);
		glEnableVertexAttribArray(vertexColorLocation);
		glEnableVertexAttribArray(vertexUVLocation);

		glVertexAttribPointer(positionLocation,    2, GL_FLOAT, GL_FALSE, (8 * sizeof(GL_FLOAT)), (GLvoid *) (0 * sizeof(GLfloat)));
		glVertexAttribPointer(vertexColorLocation, 4, GL_FLOAT, GL_FALSE, (8 * sizeof(GL_FLOAT)), (GLvoid *) (2 * sizeof(GLfloat)));
		glVertexAttribPointer(vertexUVLocation, 2, GL_FLOAT, GL_FALSE, (8 * sizeof(GL_FLOAT)), (GLvoid *) (6 * sizeof(GLfloat)));

	glBindVertexArray(0);

	//cleanup
	glDisableVertexAttribArray(positionLocation);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
// end::initializeVertexArrayObject[]

// tag::initializeVertexBuffer[]
void initializeVertexBuffer()
{
	glGenBuffers(1, &vertexDataBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, vertexDataBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	cout << "vertexDataBufferObject created OK! GLUint is: " << vertexDataBufferObject << std::endl;

	initializeVertexArrayObject();
}
// end::initializeVertexBuffer[]

// tag::invert_image[]
// from http://www.gribblegames.com/articles/game_programming/sdlgl/invert_sdl_surfaces.html - licensed as GPL (unchanged here)
int invert_image(int pitch, int height, void* image_pixels)
{
	int index;
	void* temp_row;
	int height_div_2;

	temp_row = (void *)malloc(pitch);
	if(NULL == temp_row)
	{
		SDL_SetError("Not enough memory for image inversion");
		return -1;
	}
	//if height is odd, don't need to swap middle row
	height_div_2 = (int) (height * .5);
	for(index = 0; index < height_div_2; index++) 	{
		//uses string.h
		memcpy((Uint8 *)temp_row,
			(Uint8 *)(image_pixels) +
			pitch * index,
			pitch);

		memcpy(
			(Uint8 *)(image_pixels) +
			pitch * index,
			(Uint8 *)(image_pixels) +
			pitch * (height - index-1),
			pitch);
		memcpy(
			(Uint8 *)(image_pixels) +
			pitch * (height - index-1),
			temp_row,
			pitch);
	}
	free(temp_row);
	return 0;
}
// end::invert_image[]

// tag::initializeTexturesAndSamplers[]
void initializeTexturesAndSamplers()
{
	std::string imagePath = "Opengl-logo.svg.png";
	SDL_Surface* image = IMG_Load(imagePath.c_str());
	if (image == NULL)
	{
		cout << "image loading (for texture) failed." << std::endl;
		SDL_Quit();
		exit(1);
	}
	else
	{
		cout << "loaded image: " << imagePath << " (" << image->w << ", " << image->h << ")" << std::endl;
		invert_image(image->pitch, image->h, image->pixels);
	}

	glGenTextures(1, &textureID); //generate a texture ID and store it
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->w, image->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_FreeSurface(image);

	cout << "texture created OK! GLUint is: " << textureID << std::endl;

}
// tag::initializeTexturesAndSamplers[]

// tag::loadAssets[]
void loadAssets()
{
	initializeProgram(); //create GLSL Shaders, link into a GLSL program, and get IDs of attributes and variables

	initializeVertexBuffer(); //load data into a vertex buffer

	initializeTexturesAndSamplers();

	cout << "Loaded Assets OK!\n";
}
// end::loadAssets[]

// tag::handleInput[]
void handleInput()
{
	//Event-based input handling
	//The underlying OS is event-based, so **each** key-up or key-down (for example)
	//generates an event.
	//  - https://wiki.libsdl.org/SDL_PollEvent
	//In some scenarios we want to catch **ALL** the events, not just to present state
	//  - for instance, if taking keyboard input the user might key-down two keys during a frame
	//    - we want to catch based, and know the order
	//  - or the user might key-down and key-up the same within a frame, and we still want something to happen (e.g. jump)
	//  - the alternative is to Poll the current state with SDL_GetKeyboardState

	SDL_Event event; //somewhere to store an event

	//NOTE: there may be multiple events per frame
	while (SDL_PollEvent(&event)) //loop until SDL_PollEvent returns 0 (meaning no more events)
	{
		switch (event.type)
		{
		case SDL_QUIT:
			done = true; //set donecreate remote branch flag if SDL wants to quit (i.e. if the OS has triggered a close event,
							//  - such as window close, or SIGINT
			break;

			//keydown handling - we should to the opposite on key-up for direction controls (generally)
		case SDL_KEYDOWN:
			//Keydown can fire repeatable if key-repeat is on.
			//  - the repeat flag is set on the keyboard event, if this is a repeat event
			//  - in our case, we're going to ignore repeat events
			//  - https://wiki.libsdl.org/SDL_KeyboardEvent
			if (!event.key.repeat)
				switch (event.key.keysym.sym)
				{
					//hit escape to exit
					case SDLK_ESCAPE: done = true;
				}
			break;
		}
	}
}
// end::handleInput[]

// tag::updateSimulation[]
void updateSimulation(double simLength = 0.02) //update simulation with an amount of time to simulate for (in seconds)
{
	//WARNING - we should calculate an appropriate amount of time to simulate - not always use a constant amount of time
			// see, for example, http://headerphile.blogspot.co.uk/2014/07/part-9-no-more-delays.html

	translationVector1 += (float)simLength * translationVelocityVector1;

	translationVector2 += (float)simLength * translationVelocityVector2;
}
// end::updateSimulation[]

// tag::preRender[]
void preRender()
{
	glViewport(0, 0, 600, 600); //set viewpoint
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f); //set clear colour
	glClear(GL_COLOR_BUFFER_BIT); //clear the window (technical the scissor box bounds)
}
// end::preRender[]

// tag::render[]
void render()
{
	glUseProgram(theProgram); //installs the program object specified by program as part of current rendering state

	glBindVertexArray(vertexArrayObject);


	glUniform1i(textureSamplerLocation, 0); //make texture unit 0 feed our textureSampler
	glActiveTexture(GL_TEXTURE0); //make texture unit 0 the active texture unit (which texture unit subsequent texture state calls will	affect)

	glBindTexture(GL_TEXTURE_2D, textureID); //make our texture object feed the active texture unit

	glUniform2fv(translationVectorLocation, 1, glm::value_ptr(translationVector1));
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glUniform2fv(translationVectorLocation, 1, glm::value_ptr(translationVector2));
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);

	glUseProgram(0); //clean up
}
// end::render[]

// tag::postRender[]
void postRender()
{
	SDL_GL_SwapWindow(win);; //present the frame buffer to the display (swapBuffers)
	frameLine += "Frame: " + std::to_string(frameCount++);
	cout << "\r" << frameLine << std::flush;
	frameLine = "";
}
// end::postRender[]

// tag::cleanUp[]
void cleanUp()
{
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	cout << "Cleaning up OK!\n";
}
// end::cleanUp[]

// tag::main[]
int main( int argc, char* args[] )
{
	exeName = args[0];
	//setup
	//- do just once
	initialise();
	createWindow();

	createContext();

	initGlew();

	glViewport(0,0,600,600); //should check what the actual window res is?

	//SDL_GL_SwapWindow(win); //force a swap, to make the trace clearer

	//do stuff that only needs to happen once
	//- create shaders
	//- load vertex data
	loadAssets();

	while (!done) //loop until done flag is set)
	{
		handleInput(); // this should ONLY SET VARIABLES

		updateSimulation(); // this should ONLY SET VARIABLES according to simulation

		preRender();

		render(); // this should render the world state according to VARIABLES -

		postRender();

	}

	//cleanup and exit
	cleanUp();
	SDL_Quit();

	return 0;
}
// end::main[]
