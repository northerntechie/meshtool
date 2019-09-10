/**
 * meshtool mesh converter and viewer utility
 *
 * Author: Todd Saharchuk, AScT.
 * Date:   October 23, 2018
 *
 *
 */
#pragma once
#ifndef __MESHTOOL_HPP__
#define __MESHTOOL_HPP__

#define _DEV_

#include "SDL.h"
#include <GL/glew.h>
#include <chrono>
#include <cmath>
#include <fstream>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <unordered_map>
#include <map>
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace std;

namespace twg {

#define M_PI 3.14159265358979323846 /* pi */
#define LOG(a) std::cout << a

  /**
   * Character object used in Freetype map of
   * GLchar to Character object.
   */
  struct Character {
    GLuint textureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    GLuint advance;
  };

  
  struct Vertex {
    glm::vec3 point;
    glm::vec3 normal;
    // glm::vec3 color;  // uncomment me when required
    // glm::vec2 texcoords;
  };

  struct Shader
  {
    std::string filename;
    GLuint ID; // shader id

    Shader() {}; // Empty ctor.  Must be init'd
    Shader(std::string filename, GLuint type)
      : filename{filename}
    {
      ID = glCreateShader(type);
#ifdef _DEV_
      if(type == GL_VERTEX_SHADER)
	{
	  LOG("[Ok] Created shader: GL_VERTEX_SHADER\n");
	}
      else if(type == GL_FRAGMENT_SHADER)
	{
	  LOG("[Ok] Created shader: GL_FRAGMENT_SHADER\n");
	}
#endif
      
      std::string shader_source = loadShader(filename);
      int length = shader_source.size();
      const char* ss_cstr = shader_source.c_str();
      glShaderSource(ID, 1, &ss_cstr, &length);
      glCompileShader(ID);
      
#ifdef _DEV_
      GLint status;
      glGetShaderiv(ID, GL_COMPILE_STATUS, &status);
      if(status == GL_FALSE)
	{
	  LOG("[Error] Shader ");
	  LOG(filename);
	  LOG(" compilation failed!\n");
	  GLint logsize = 0;
	  glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &logsize);
	  char log[logsize];
	  glGetShaderInfoLog(ID, 32767, &logsize, log);
	  LOG("[Shader Error] ");
	  LOG(log); LOG("\n");
	} 
      else
	{
	  LOG("[Ok] Shader ");
	  LOG(filename);
	  LOG(" compiled successfully ...\n");
	}
#endif
    };
    
    /**
     * function loadShader to load a .vs or .fs file and
     * convert to a string with newline characters to 
     * place in the shader compiler.
     */
    std::string loadShader(const std::string &filename) {
      ifstream in{filename, ios::in};
      if (!in) {
	LOG("[Error] Cannot open: ");
	LOG(filename); LOG("\n");
	exit(1);
      }
      std::string output{};
      std::string line{};
      while (std::getline(in, line)) {
	output += line;
	output += "\n";
      }
      in.close();
      return output;
    }

  };

  struct Program
  {
    Shader vertexShader;
    Shader fragmentShader;
    GLuint ID;
    Program() {}; // Empty constructor.  Must be initialized
    Program(std::string vsFilename,
	    std::string fsFilename)
      : vertexShader{vsFilename,GL_VERTEX_SHADER},
	fragmentShader{fsFilename,GL_FRAGMENT_SHADER}
    {
      ID = glCreateProgram();
      glAttachShader(ID, vertexShader.ID);
      glAttachShader(ID, fragmentShader.ID);

      glLinkProgram(ID);

#ifdef _DEV_
      GLint result;
      glGetProgramiv(ID, GL_LINK_STATUS, &result);
      LOG("[Ok] Program link result= ");
      LOG(result);
      LOG("\n");
      if(result == GL_FALSE)
	{
	  GLint maxLength = 0;
	  glGetProgramiv(ID, GL_INFO_LOG_LENGTH,
			 &maxLength);
	  GLchar infoLog[maxLength + 1];
	  glGetProgramInfoLog(ID, maxLength,
			      &maxLength, &infoLog[0]);
	  infoLog[maxLength] = 0x00;
	  LOG("[Error] ");
	  LOG(static_cast<char*>(infoLog));
	  LOG("\n");	      
	}
#endif
      glDeleteShader(vertexShader.ID);
      glDeleteShader(fragmentShader.ID);
    }
  };
  
  struct mesh {
    constexpr static int stride = 6;
    std::vector<Vertex> vertices;
    std::vector<GLushort> elements;
    mesh(std::vector<glm::vec3> points, std::vector<glm::vec3> normals,
	 std::vector<GLushort> elements)
      : elements{elements} {
      auto nit = normals.begin();
      for (auto pit = points.begin(); pit != points.end(); pit++, nit++) {
	vertices.push_back(Vertex{*pit, *nit});
      }
    }
    std::size_t size() { return 6 * vertices.size() * sizeof(GLfloat); }
  };

  /**
   * This class is the main object.  It is intended to be wrapped around
   * a GameApplication object that will determine platform capabilities.
   * The DeviceCapabilities structure is passed into the constructor to
   * initialize the game.
   *
   */
  class meshtool {
  private:
    bool _isRunning = false;
    SDL_Window *_window = 0;
    SDL_Renderer *_renderer = 0;
    SDL_Texture *_texture = 0;
    SDL_GLContext _context;
    GLuint program = 0;
    Program modelProgram;
    GLuint vao = 0;
    GLuint vbo, vbn, vbe;
    GLuint triangle;
    GLfloat angleX = 0.0f;
    GLfloat angleY = 0.0f;
    GLfloat angleZ = 0.0f;
    GLfloat scale = 0.3f;
    mesh *m_mesh;
    GLint screen_width, screen_height;
    FT_Library ft;
    FT_Face face;
    std::map<GLchar,Character> characters;
    
  public:
    meshtool(mesh *m_mesh);
    ~meshtool();

    // Class functions
    void initCharacterMap();
    int init(std::string &&title, int xpos, int ypos, int width, int height,
	     int flags);
    void render();
    void update();
    void handleEvents();
    void clean();
    
    // Get/Set functions
    bool isRunning() { return _isRunning; };

    // Data members
    static GLfloat idMat[16];
  };
  /* End class meshtool */

} /* End twg namespace */
#endif
