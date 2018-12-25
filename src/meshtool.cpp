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

  static glm::vec3 calc_normal(glm::vec3 a,
			       glm::vec3 b)
  {
    return glm::normalize(a * b);
  }

  static bool isConnected(GLushort a,
			  GLushort b,
			  GLushort c,
			  GLushort x)
  {
    if(a == x ||
       b == x ||
       c == x)
      {
	return true;
      }
    else
      {
	return false;
      }
  }

  static glm::vec3 avgVectors(glm::vec3 a,
			      glm::vec3 b)
  {
    return glm::normalize(b + (glm::vec3(0.5) * (b - a)));
  }
  
  /**
   * Static utility genNormals from a vector of vertices 
   * and element indices.
   *
   * element array format
   *
   * e0_v0, e0_v1, e0_v2,
   * e1_v0, e1_v1, e1_v2, ...
   *
   * if element index appears in k triangles,
   * then push k triangles in a single vector
   * and calculation average normal vector after
   * building the list of redundant vertices.
   *
   * The vector of normals is then rebuilt to correspond
   * to the indexed elements.
   * Search for connected vertices using
   * elements array and push key=element index and
   * value=face normal.  A positive search result
   * should return the associated triangle to allow
   * for calculation of the face normal.
   * Iterate triangle wise to calculate the face normal
   * 
   * [[e0=217,e1=34,e2=222],[e3=217,e4=34,e5=200], ...
   * 1st and 2nd triangles share 2 common vertices.
   * The normal for both vertex will need to be the
   * average of the two triangle normals.
   *
   */
  static std::vector<glm::vec3>
  avgNormals(const std::vector<glm::vec3>& vertices,
	     const std::vector<GLushort>& elements)
  {
    std::vector<glm::vec3> normals;
    normals.resize(vertices.size(), glm::vec3(0.0));
  
    for(int i=0; (i+3)<vertices.size(); i += 3)
      {
	// 1st generate the normal for the three
	// vertices
	glm::vec3 a = vertices[elements[i]];
	glm::vec3 b = vertices[elements[i+1]];
	glm::vec3 normal = calc_normal(a,b);
	for(int k=i; k<(i+3); ++k)
	  {
	    if(normals[elements[k]] == glm::vec3(0.0))
	      {
		normals[elements[k]] = normal;
	      }
	    else
	      {
		normals[elements[k]] = avgVectors(normals[elements[k]], normal);
	      }
	  }
      }
    return normals;
  }
  

  /** Static utility loadObject to load a .obj mesh description
   * file and place in a mesh struct.
   */
  static mesh loadObject(const std::string &filename) {
    std::ifstream in{filename, ios::in};
    if (!in) {
      std::cerr << "Not able to open: " << filename << "\n";
      exit(1);
    }

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<GLushort> elements;
    std::string line;

    while (std::getline(in, line)) {
      if (line.substr(0, 2) == "v ") {
	std::istringstream ss{line.substr(2)};
	glm::vec3 vv;
	ss >> vv.x;
	ss >> vv.y;
	ss >> vv.z;
	vertices.push_back(vv);
      } else if (line.substr(0, 2) == "f ") {
	std::istringstream ss{line.substr(2)};

	GLushort a;
	GLushort b;
	GLushort c;
	std::string nullStr;
	ss >> a;
	ss >> nullStr;
	ss >> b;
	ss >> nullStr;
	ss >> c;
	elements.push_back(--a);
	elements.push_back(--b);
	elements.push_back(--c);
      } else if (line[0] == '#') {
	std::cout << "OBJ FILE COMMENT: " << line.substr(1) << "\n";
      }
    }

    // Normal averaging method - TODO(Todd): get to work
    // if smoothing required.
    // normals = avgNormals(vertices, elements);

    // Workable normal calculations.
    normals.resize(vertices.size(), glm::vec3(0.0, 0.0, 0.0));
    for (int i = 0; i < elements.size(); i += 3) {
      GLushort ia = elements[i];
      GLushort ib = elements[i + 1];
      GLushort ic = elements[i + 2];
      glm::vec3 normal = glm::normalize(
					glm::cross(vertices[ib] - vertices[ia], vertices[ic] -
						   vertices[ia]));
      normals[ia] = normals[ib] = normals[ic] = normal;
    }
    return mesh{vertices, normals, elements};
  }
  

  meshtool::meshtool(mesh *m_mesh) { this->m_mesh = m_mesh; }

  meshtool::~meshtool() {}

  GLfloat meshtool::idMat[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
				 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

  int meshtool::init(std::string &&title, int xpos, int ypos, int width,
		     int height, int flags) {
    screen_width = width;
    screen_height = height;
    _isRunning = true;
    if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
      std::cout << "SDL init a success...\n";
      _window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
				 SDL_WINDOWPOS_CENTERED, width, height, flags);
      if (_window != 0) {
	_renderer = SDL_CreateRenderer(_window, -1, 0);
      } else {
	std::cout << "SDL_CreateRenderer failed!\n";
	return 3;
      }

    } else {
      std::cout << "SDL init failed!...\n";
      return 3;
    }

    // Initialize the OpenGL environment
    glewInit();
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    std::cout << "GL_VERSION= " << glGetString(GL_VERSION) << "\n";
    std::cout << "GL_VENDOR= " << glGetString(GL_VENDOR) << "\n";
    std::cout << "GL_SHADING_LANGUAGE_VERSION= "
	      << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
    GLuint vs, fs;

    _context = SDL_GL_CreateContext(_window);
    if (&_context == 0) {
      std::cout << "SDL_GL_CreateContext failed!\n";
      return 2;
    }

    glViewport(0, 0, width, height);
    std::cout << "Set viewport = (0,0," << width << "," << height << ")\n";
    _program = Program{"shaders/basic.vs",
                       "shaders/basic.fs"};
    
    glUseProgram(_program.programID);

    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // vbo format is vvvnnn, or xyzabc
    glBufferData(GL_ARRAY_BUFFER, m_mesh->size(), &m_mesh->vertices[0],
		 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			  reinterpret_cast<void *>(offsetof(Vertex, point)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
			  reinterpret_cast<void *>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &vbe);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbe);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 m_mesh->elements.size() * sizeof(GLushort), &m_mesh->elements[0],
		 GL_STATIC_DRAW);

    glDisableVertexAttribArray(vao);

    return 0;
  }

  void meshtool::render() {
    /* Render Code */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.15, 0.22, 0.15, 0.0);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(_program.programID);
    glBindVertexArray(vao);
    GLint mM = glGetUniformLocation(_program.programID, "mM");
    angleY += 0.05;
    angleY = std::fmod(angleY, 2 * M_PI);
    angleX += 0.03233;
    angleX = std::fmod(angleX, 2 * M_PI);

    glm::mat4 modelMat = glm::scale(glm::mat4(1.0), glm::vec3(0.3f, 0.3f, 0.3f));
    modelMat = glm::rotate(modelMat, angleY, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMat = glm::rotate(modelMat, angleX, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMat = glm::rotate(modelMat, angleZ, glm::vec3(0.0,0.0,1.0));

    glUniformMatrix4fv(mM, 1, GL_FALSE, glm::value_ptr(modelMat));

    // Draw triangles from vertices
    // glDrawArrays(GL_TRIANGLES,0,m_mesh->vertices.size());

    glEnableVertexAttribArray(vao);
    int size;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    glDrawElements(GL_TRIANGLES, size / sizeof(GLushort), GL_UNSIGNED_SHORT, 0);
    /* Send to GPU */
    SDL_GL_SwapWindow(_window);
  }

  void meshtool::update() { /* Build vertex buffer once */
  }

  void meshtool::handleEvents() {
    SDL_Event event;
    SDL_MouseButtonEvent *mev = 0;
    SDL_MouseMotionEvent *mmev = 0;
    if (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
	_isRunning = false;
	break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
	std::cout << "Mouse action: \n";
	break;
      case SDL_KEYDOWN:
	switch (event.key.keysym.sym) {
	case 'Q':
	case 'q':
	  _isRunning = false;
	  break;
	case SDLK_LEFT:
	  std::cout << "Rotating left...\n";
	  angleZ += 0.5f;
	  break;
	case SDLK_RIGHT:
	  std::cout << "Rotating right...\n";
	  angleZ += 0.5f;
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

  void meshtool::clean() {
    std::cout << "Exiting and cleanup of utility...\n";
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(_program);
    SDL_GL_DeleteContext(_context);
    SDL_DestroyWindow(_window);
    SDL_DestroyRenderer(_renderer);
    SDL_Quit();
    std::cout << "Everything cleaned and destroyed...\n";
  }
}

int main(int argc, char **argv) {
  std::string filename;

  if (argc < 3) {
    std::coust << "Usage: meshtool -f <mesh>.obj\n";
    exit(1);
  } else {
    std::string token{argv[1]};
    if (token == "-f") {
      filename = std::string{argv[2]};
      std::cout << "Opening file: " << filename << "\n";
      twg::mesh m_mesh = twg::loadObject(filename);
      twg::meshtool mt{&m_mesh};
      mt.init("meshtool converter and viewer", 25, 25, 800, 600,
              SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

      while (mt.isRunning()) {
        mt.handleEvents();
        mt.update();
        mt.render();
        std::this_thread::sleep_for(30ms);
      }
      mt.clean();
    }
  }
}
