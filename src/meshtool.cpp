/**
 * meshtool mesh converter and viewer utility
 *
 * Author: Todd Saharchuk, AScT.
 * Date:   October 23, 2018
 *
 *
 */
#include <meshtool.hpp>
#include <cmath>

namespace twg {  

  meshtool::meshtool()
  {}
  
  meshtool::~meshtool()
  {}

  GLfloat meshtool::idMat[16] =
    {
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f};
  
  
  int meshtool::init(std::string&& title,
		 int xpos,
		 int ypos,
		 int width,
		 int height,
		 int flags)
  {
    _isRunning = true;
    if(SDL_Init(SDL_INIT_EVERYTHING) == 0)
      {
    	std::cout << "SDL init a success...\n";
    	_window = SDL_CreateWindow(title.c_str(),
    				   SDL_WINDOWPOS_CENTERED,
				   SDL_WINDOWPOS_CENTERED,
    				   width,height,
    				   flags);
    	if(_window != 0)
    	  {
    	    _renderer = SDL_CreateRenderer(_window, -1, 0);
    	  }
    	else
    	  {
    	    std::cout << "SDL_CreateRenderer failed!\n";
    	    return 3;
    	  }

      }
    else
      {
    	std::cout << "SDL init failed!...\n";
    	return 3;
      }

    
    // Initialize the OpenGL environment
    glewInit(); 
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    std::cout << "GL_VERSION= "
	      << glGetString(GL_VERSION)
	      << "\n";
    std::cout << "GL_VENDOR= "
	      << glGetString(GL_VENDOR)
	      << "\n";
    std::cout << "GL_SHADING_LANGUAGE_VERSION= "
	      << glGetString(GL_SHADING_LANGUAGE_VERSION)
	      << "\n";
    GLuint vs, fs;

    _context = SDL_GL_CreateContext(_window);
    if(&_context == 0)
      {
	std::cout << "SDL_GL_CreateContext failed!\n";
	return 2;
      }
   
    glViewport(0,0,width,height);
    std::cout << "Set viewport = (0,0,"
	      << width << ","
	      << height << ")\n";

    vs = glCreateShader(GL_VERTEX_SHADER);
    std::cout << "Created GL_VERTEX_SHADER\n";
    fs = glCreateShader(GL_FRAGMENT_SHADER);
    std::cout << "Created GL_FRAGMENT_SHADER\n";
    
    int length = strlen(shader_programs::vertex_shader);
    glShaderSource(vs, 1, &shader_programs::vertex_shader, &length);
    glCompileShader(vs);

    GLint status;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    if( status == GL_FALSE )
      {
	std::cerr << "Vertex shader compilation failed!\n";
	GLint logsize = 0;
	glGetShaderiv(vs, GL_INFO_LOG_LENGTH, &logsize);
	char log[logsize];
	int ll = sizeof(GLsizei)*logsize;
	glGetShaderInfoLog(vs, 32767, &ll, log);
	std::cerr << log << "\n";
	return 1;
      }
    else
      {
	std::cout << "Vertex shader compiled successfully...\n";
      }
    
    
    length = strlen(shader_programs::fragment_shader);
    glShaderSource(fs, 1, &shader_programs::fragment_shader, &length );
    glCompileShader(fs);

    glGetShaderiv(fs, GL_COMPILE_STATUS, &status );
    if(status == GL_FALSE)
      {
	std::cerr << "Fragment shader compilation failed!\n";
        return 1;
      }
    else
      {
	std::cout << "Fragment shader compilation successfully...\n";
      }

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    
    glLinkProgram(program);

    GLint result;
    glGetProgramiv(program,GL_LINK_STATUS,&result);
    std::cout << "Program link result= " << result << "\n";
    if(result == GL_FALSE)
      {
	GLint maxLength = 0;
	glGetProgramiv(program,
		       GL_INFO_LOG_LENGTH,
		       &maxLength);
	GLchar infoLog[maxLength+1];
	glGetProgramInfoLog(program,
			    maxLength,
			    &maxLength,
			    &infoLog[0]);
	infoLog[maxLength] = 0x00;
	std::cout << (char*)infoLog
		  << "\n";
	
      }
    glUseProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
    
    glDisable( GL_DEPTH_TEST );
    glViewport( 0, 0, width, height );

    
    const GLfloat vbo_buffer_data[] = {
    /*X,     Y,     Z  */
      -0.5f, -0.5f, 0.f,
      0.f,   0.5f,  0.f,
      0.5f,  -0.5f,  0.f };
    const GLint vbo_num_vertices = 3;
    const GLfloat vbo_color_data[] = {
      /*R,  G,    B,   */
      1.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 1.0f };
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
  
    GLuint vbo;
    const GLint stride = 7;
    glGenBuffers(1, &vbo);
    std::cout << "vbo= " << vbo << "\n";
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
		 7*3*sizeof(GLfloat),
		 NULL,
		 GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER,
		    0,
		    3*3*sizeof(GLfloat),
		    vbo_buffer_data);
    glBufferSubData(GL_ARRAY_BUFFER,
		    3*3*sizeof(GLfloat),
		    3*3*sizeof(GLfloat),
		    vbo_color_data);
    
    GLuint verts = glGetAttribLocation(program,"vPos");
    std::cout << "vPos= " << verts << "\n";
    glEnableVertexAttribArray(verts);
    glVertexAttribPointer(verts,3,GL_FLOAT,
			  GL_FALSE,
			  3*sizeof(GLfloat),
			  (const void*)0);
    GLuint colors = glGetAttribLocation(program, "vColor");
    std::cout << "vColor= " << colors << "\n";
    glEnableVertexAttribArray(colors);
    glVertexAttribPointer(colors,3,GL_FLOAT,
			  GL_FALSE,
			  3*sizeof(GLfloat),
			  (const void*)(3*3*sizeof(GLfloat)));

    
    return 0;
  }
  
  void meshtool::render()
  {
    /* Render Code */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor( 0.2, 0.2, 0.0, 0.0 );

    glUseProgram(program);
    glBindVertexArray(vao);
    GLint mM = glGetUniformLocation(program,"mM");
    angle += 0.05;
    angle = std::fmod(angle,2*M_PI);
    
    glm::mat4 modelMat = glm::rotate(glm::mat4(1.0), angle, glm::vec3(0.f, 1.f, 0.f));
    glUniformMatrix4fv(mM,1,
    		       GL_FALSE, glm::value_ptr(modelMat));

    glDrawArrays(GL_TRIANGLES,0,3);
    
    /* Send to GPU */
    SDL_GL_SwapWindow(_window );
  }

  void meshtool::update()
  {
    /* Build vertex buffer once */
    
  }
  
  void meshtool::handleEvents()
  {
    SDL_Event event;
    SDL_MouseButtonEvent* mev = 0;
    SDL_MouseMotionEvent* mmev = 0;
    if(SDL_PollEvent(&event))
      {
	switch(event.type)
	  {
	  case SDL_QUIT:
	    _isRunning = false;
	    break;
	  case SDL_MOUSEBUTTONDOWN:
	  case SDL_MOUSEBUTTONUP:
	    std::cout << "Mouse action: \n";
	    break;
	  case SDL_KEYDOWN:
	    switch(event.key.keysym.sym)
	      {
	      case 'Q':
	      case 'q':
		_isRunning = false;
		break;
	      case SDLK_LEFT:
		std::cout << "Rotating left...\n";
		// rotateModel();
		break;
	      case SDLK_RIGHT:
		std::cout << "Rotating right...\n";
		// rotateModel();
		break;
	      default:
		break;
	      }
	    break;
	  default:
	    break;
	  }
      }
  }
  
  void meshtool::clean()
  {
    std::cout << "Exiting and cleanup of utility...\n";
    glDeleteProgram(program);
    SDL_GL_DeleteContext(_context);
    SDL_DestroyWindow(_window);
    SDL_DestroyRenderer(_renderer);
    SDL_Quit();
    std::cout << "Everything cleaned and destroyed...\n";
  }
  
}

int main(int argc, char** argv)
{
  std::string filename;
  if(argc<3)
    {
      std::cout << "Usage: meshtool -f <mesh>.obj\n";
      exit(1);
    }
  else
    {
      std::string token{argv[1]};
      if(token == "-f")
	{
	  filename = std::string{argv[2]};
	  std::cout << "Opening file: " << filename << "\n";
	}
    }
  
  twg::meshtool mt;
  mt.init("meshtool converter and viewer",
	  25,
	  25,
	  600,
	  800,
	  SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

  while(mt.isRunning())
    {
      mt.handleEvents();
      mt.update();
      mt.render();
    }
  mt.clean();
}
