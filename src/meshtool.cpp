/**
 * meshtool mesh converter and viewer utility
 *
 * Author: Todd Saharchuk, AScT.
 * Date:   October 23, 2018
 *
 *
 */
#include <meshtool.hpp>

namespace twg {  

  /** Static utility loadObject to load a .obj mesh description
   * file and place in a mesh struct.
   */
  static void loadObject(const std::string& filename,
			 mesh& m_mesh)
  {
    std::ifstream in{filename, ios::in};
    if(!in)
      {
	std::cerr << "Not able to open: "
		  << filename << "\n";
	exit(1);
      }

    std::string line;
    while(std::getline(in,line))
      {
	if(line.substr(0,2) == "v ")
	  {
	    std::istringstream ss{line.substr(2)};
	    glm::vec3 vv;
	    ss >> vv.x;
	    ss >> vv.y;
	    ss >> vv.z;
	    m_mesh.vertices.push_back(vv);
	  }
	else if(line.substr(0,2) == "f ")
	  {
	    std::istringstream ss{line.substr(2)};
	    
	    GLushort a;
	    GLushort b;
	    GLushort c;
	    std::string nullStr;
	    ss >> a; ss >> nullStr;
	    ss >> b; ss >> nullStr;
	    ss >> c;
	    m_mesh.elements.push_back(--a);
	    m_mesh.elements.push_back(--b);
	    m_mesh.elements.push_back(--c);
	  }
	else if(line[0] == '#')
	  {
	    std::cout << "OBJ FILE COMMENT: "
		      << line.substr(1) << "\n";
	  }
      }
    m_mesh.normals.resize(m_mesh.vertices.size(), glm::vec3(0.0, 0.0, 0.0));
    for(int i=0; i<m_mesh.elements.size(); i +=3)
      {
	GLushort ia = m_mesh.elements[i];
	GLushort ib = m_mesh.elements[i+1];
	GLushort ic = m_mesh.elements[i+2];
	glm::vec3 normal = glm::normalize
	  (glm::cross
	   (m_mesh.vertices[ib] - m_mesh.vertices[ia],
	    m_mesh.vertices[ic] - m_mesh.vertices[ia]));
	m_mesh.normals[ia] = m_mesh.normals[ib] = m_mesh.normals[ic] = normal;
      }
  }
			 /**
   * Static utility loadShader to load a .vs or .fs file and
   * convert to a string with newline characters to place in the
   * shader compiler.
   */
  static std::string loadShader(const std::string& filename)
  {
    ifstream in{filename, ios::in};
    if(!in)
      {
	cerr << "Cannot open: "
	     << filename << "\n";
	exit(1);
      }
    std::string output{};
    std::string line{};
    while(std::getline(in,line))
      {
	output += line;
	output += "\n";
      }
    in.close();
    std::cout << "output: " << output << "\n";
    return output;
  }
  
  meshtool::meshtool(mesh* m_mesh)
  {
    this->m_mesh = m_mesh;
  }
  
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
    screen_width = width;
    screen_height = height;
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
    	    _renderer = SDL_CreateRenderer(_window,
					   -1,
					   0);
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

    std::string vs_prog = loadShader("shaders/basic.vs");
    int length = vs_prog.size();
    const char* vs_inp = vs_prog.c_str();
    glShaderSource(vs, 1, &vs_inp , &length);
    std::cout << "Basic Vertex Shader:\n"
	      << vs_prog << "\n";
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
    

    std::string fs_prog = loadShader("shaders/basic.fs");
    std::cout << "Basic Fragment Shader:\n"
	      << fs_prog << "\n";

    int len2 = fs_prog.size();
    const char* fs_inp = fs_prog.c_str();
    glShaderSource(fs, 1, &fs_inp, &len2);
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

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
  
    glGenBuffers(1, &vbo);
    std::cout << "vbo= " << vbo << "\n";
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
		 3*m_mesh->vertices.size()*sizeof(GLfloat),
		 &m_mesh->vertices[0],
		 GL_STATIC_DRAW);
    GLuint verts = glGetAttribLocation(program,"vPos");
    std::cout << "vPos= " << verts << "\n";
    glEnableVertexAttribArray(verts);
    glVertexAttribPointer(verts,3,GL_FLOAT,
			  GL_FALSE,
			  3*sizeof(GLfloat),
			  (const void*)0);
    
    glGenBuffers(1, &vbn);
    std::cout << "vbn= " << vbn << "\n";
    glBindBuffer(GL_ARRAY_BUFFER, vbe);
    glBufferData(GL_ARRAY_BUFFER,
		 3*m_mesh->normals.size()*sizeof(GLfloat),
		 &m_mesh->normals[0],
		 GL_STATIC_DRAW);
    GLuint normals = glGetAttribLocation(program,"vNormals");
    std::cout << "vNormals= " << normals << "\n";
    glEnableVertexAttribArray(normals);
    glVertexAttribPointer(normals,3,GL_FLOAT,
			  GL_FALSE,
			  3*sizeof(GLfloat),
			  (const void*)0);
    
    glGenBuffers(1, &vbe);
    std::cout << "vbe= " << vbe << "\n";
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbe);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 3*m_mesh->elements.size()*sizeof(GLushort),
		 &m_mesh->elements[0],
		 GL_STATIC_DRAW);

    // 
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

    glm::mat4 modelMat = glm::scale(glm::mat4(1.0),
				    glm::vec3(0.3f,0.3f,0.3f));    
    modelMat = glm::rotate(modelMat, angle, glm::vec3(0.f, 1.f, 0.f));
    //modelMat = glm::rotate(modelMat, angle, glm::vec3(1.0,0.0,0.0));
    //modelMat = glm::rotate(modelMat, angle, glm::vec3(0.0,0.0,1.0));
    
    glm::mat4 view = glm::lookAt
      (glm::vec3(0.0, 0.2, 1.5f),  // eye
       glm::vec3(0.0, 0.0, 0.0),  // target point
       glm::vec3(0.0, 1.0, 0.0)); // up
    glm::mat4 projection = glm::perspective
      (glm::radians(45.0f),
       1.0f*screen_width/screen_height,
       -10.0f, 100.0f);
    //modelMat = view * modelMat;
    
    glUniformMatrix4fv(mM,1,
    		       GL_FALSE, glm::value_ptr(modelMat));

    // Draw triangles from vertices
    //glDrawArrays(GL_TRIANGLES,0,m_mesh->vertices.size());

    // Draw triangles from elements
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbe);
    int size;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER,
			   GL_BUFFER_SIZE,
			   &size);
    glDrawElements(GL_TRIANGLES,
		   size/sizeof(GLushort),
		   GL_UNSIGNED_SHORT,
		   0);
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
    glDeleteBuffers(1,&vbo);
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
  twg::mesh m_mesh{};

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
	  twg::loadObject(filename, m_mesh);
	}
    }

  twg::meshtool mt{&m_mesh};
  mt.init("meshtool converter and viewer",
	  25,
	  25,
	  800,
	  600,
	  SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

  while(mt.isRunning())
    {
      mt.handleEvents();
      mt.update();
      mt.render();
      std::this_thread::sleep_for(30ms);
    }
  mt.clean();
}
