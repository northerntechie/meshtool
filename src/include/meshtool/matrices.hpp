/**
 * meshtool mesh converter and viewer utility
 *
 * Author: Todd Saharchuk, AScT.
 * Date:   December 17, 2018
 *
 *
 */
 #pragma
 #ifndef __MATRICES_HPP
 #define __MATRICES_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>

class matrices
{
private:
    std::vector<glm::mat4> modelMatrix;
    std::vector<glm::mat4> viewMatrix;
    std::vector<glm::mat4> projectionMatrix;
    int currentMatrix;
public:
    enum matrixModes { MODEL_MATRIX = 0, VIEW_MATRIX,PROJECTION_MATRIX };
    matrices()
    {
        modelMatrix.push_back(mat4(1.0f));
        viewMatrix.push_back(mat4(1.0f));
        projectionMatrix.push_back(mat4(1.0f));
    };
}

 #endif /* end of include guard:  */
