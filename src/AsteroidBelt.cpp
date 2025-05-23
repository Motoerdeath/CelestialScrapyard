#include <iostream>
#include "../headers/AsteroidBelt.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "../headers/camera.hpp"
#include "config/config.h"
#include "../headers/Random.hpp"
#include "../headers/OrbitUtil.hpp"
#include "../headers/LightSource.hpp"

const std::vector<std::string> SHADER_PATHS = {(std::string)Project_SOURCE_DIR +"/src/shader/asteroidBelt.vert", (std::string)Project_SOURCE_DIR + "/src/shader/asteroidBelt.frag"};
const std::vector<GLenum> SHADER_TYPES = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};

AsteroidBelt::AsteroidBelt(unsigned long noiseSeed) {
    asteroidBeltProgram = std::make_unique<ShaderProgram>(SHADER_PATHS, SHADER_TYPES);

    Noise noise = Noise(noiseSeed, Noise::asteroid);
    for(int i = 0; i < CUBE_NUM_FACES; ++i) {
        glm::vec3 direction = directions[i];
        cubefaces[i] = std::make_unique<CubeFace>(direction, noise, ASTEROID_RESOLUTION);
    }

    for(int i = 0; i < CUBE_NUM_FACES; ++i) {
        cubefaces[i]->addEdgeNormals(cubefaces);
        cubefaces[i]->uploadToGPU();
    }

    offsets = new glm::vec4[NUM_ASTEROIDS];
    scaleFactors = new glm::vec4[NUM_ASTEROIDS];
    offsets[0] = glm::vec4(LightSource::getInstance().getPosition(), 0.f);
    for (int i = 1; i < NUM_ASTEROIDS; ++i) {
        glm::vec4 asteroidCenter;
        float minDistance;
        //check that asteroids aren't too close, TODO: might run indefinitely if there are too many asteroids
        do {
            minDistance = std::numeric_limits<float>::max();
            asteroidCenter = glm::vec4(getRandomPositionInOrbit(3.6f, 5.0f), 0.f);
            for (int j = 0; j < i; ++j) {
                float distance = glm::length(asteroidCenter - offsets[j]);
                if (distance < minDistance) {
                    minDistance = distance;
                }
            }
        } while (minDistance < 5.5f * ASTEROID_RADIUS);
        offsets[i] = asteroidCenter;
        scaleFactors[i] = glm::vec4(glm::length(asteroidCenter)-2.8f);
    }

    glGenBuffers(1, &offsetBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, offsetBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * NUM_ASTEROIDS, offsets, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glGenBuffers(1, &scaleFactorBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, scaleFactorBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec4) * NUM_ASTEROIDS, scaleFactors, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}

void AsteroidBelt::setUniformMatrix(glm::mat4 matrix, std::string type) {
    glUniformMatrix4fv(glGetUniformLocation(asteroidBeltProgram->name, type.c_str()), 1, GL_FALSE, glm::value_ptr(matrix));
}

void AsteroidBelt::move(float &pickedAsteroidTheta) {
    offsets[0] = glm::vec4(LightSource::getInstance().getPosition(), 0.f);
    for (int i = 1; i < NUM_ASTEROIDS - 1; ++i) {
        auto throwInfo = std::find_if(throwInfos.begin(), throwInfos.end(), [i](const ThrowInfo &info) {return info.instanceId == i;});
        if (throwInfo != throwInfos.end()) {
            //keep asteroid at center of planet after throw
            if (glm::all(glm::epsilonEqual(offsets[i], glm::vec4(0.f), 1E-1f))) {
                throwInfos.erase(throwInfo);
            } else {
                throwInfo->t += THROW_SPEED;
                offsets[i] = glm::vec4{throwInfo->direction * throwInfo->t, 0.f};
            }
        } else {
            //if (i == pickedID) {
                //offsets[i] = glm::vec4(moveInOrbitWithTheta(offsets[i], ASTEROID_SPEED, pickedAsteroidTheta), 0.f);
            //} else {
                offsets[i] = glm::vec4(moveInOrbit(offsets[i], ASTEROID_SPEED, true), 0.f);
            //}
        }
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, offsetBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::vec4) * NUM_ASTEROIDS, offsets);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}


glm::vec3 AsteroidBelt::throwTowardsCenter(float &asteroidSize) {
    if (pickedID == -1)
        return glm::vec3(0.0f);
    glm::vec3 throwDirection = offsets[pickedID];
    asteroidSize = scaleFactors[pickedID].x;
    throwInfos.push_back({throwDirection, 1.f, pickedID});
    pickedID = -1;
    return throwDirection;
}

void AsteroidBelt::prepareDraw(int width, int height, bool outlining) {
    Camera* camera = Camera::get_Active_Camera();

    asteroidBeltProgram->use();
    //generating Projection,Model and view matrixes for Shader.
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);
    setUniformMatrix(proj,"projection");

    glm::mat4 model = glm::mat4(ASTEROID_RADIUS + (outlining ? 0.01f : 0.0f));
    //model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    setUniformMatrix(model,"model");

    glm::mat4 view = glm::mat4(1.0f);

    glm::vec3 cameraPos = camera->get_Camera_Position();
    view = camera->get_View_Matrix();
    //Camera* camera2 = Camera::get_Active_Camera();
    //view = camera2->get_View_Matrix();
    setUniformMatrix(view,"view");
    glUniform1i(glGetUniformLocation(asteroidBeltProgram->name, "depthRender"), false);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, offsetBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scaleFactorBuffer);
    glUniform1i(glGetUniformLocation(asteroidBeltProgram->name, "picking"), picking);
    glUniform1i(glGetUniformLocation(asteroidBeltProgram->name, "pickedID"), pickedID);
}

void AsteroidBelt::executeDraw() {
    for (int i = 0; i < CUBE_NUM_FACES; ++i) {
        cubefaces[i]->drawInstanced(NUM_ASTEROIDS);
    }
}

void AsteroidBelt::draw(int width, int height, float &pickedAsteroidTheta) {
    if (!picking) {
        move(pickedAsteroidTheta);
    }

    //outline rendering based on https://learnopengl.com/Advanced-OpenGL/Stencil-testing
    glEnable(GL_STENCIL_TEST);

    //standard draw
    prepareDraw(width, height, false);
    glUniform1i(glGetUniformLocation(asteroidBeltProgram->name, "outlining"), 0);
    executeDraw();

//    //draw picked asteroid again in stencil buffer and in solid color for outline rendering
//    if (pickedID == -1) {
//        return;
//    }
//
//    prepareDraw(width, height, false);
//    glUniform1i(glGetUniformLocation(asteroidBeltProgram->name, "outlining"), 1);
//    glStencilFunc(GL_ALWAYS, 1, 0xFF);
//    glStencilMask(0xFF);
//    executeDraw();
//
//    prepareDraw(width, height, true);
//    glUniform1i(glGetUniformLocation(asteroidBeltProgram->name, "outlining"), 2);
//    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
//    glStencilMask(0x00);
//    executeDraw();
//    glStencilMask(0xFF);
//    glStencilFunc(GL_ALWAYS, 1, 0xFF);
//    glDisable(GL_STENCIL_TEST);
}


void AsteroidBelt::drawForDepthMap() {
    asteroidBeltProgram->use();
    glUniform1i(glGetUniformLocation(asteroidBeltProgram->name, "depthRender"), true);
    glUniform1i(glGetUniformLocation(asteroidBeltProgram->name, "outlining"), 0);
    LightSource::getInstance().bindLightMatrices(asteroidBeltProgram->name);

    glm::mat4 model = glm::mat4(ASTEROID_RADIUS);
    setUniformMatrix(model,"model");

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, offsetBuffer);
    //glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, scaleFactorBuffer);
    executeDraw();
}

void AsteroidBelt::pick(int width, int height, glm::vec2 mousePosition, float &pickedAsteroidTheta) {
    picking = true;
    glClear(GL_COLOR_BUFFER_BIT);
    float temp = 0.f;
    draw(width, height, temp);

    glMemoryBarrier(GL_PIXEL_BUFFER_BARRIER_BIT);

    //http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-an-opengl-hack/
    glFlush();
    glFinish();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    unsigned char data[4];
    glReadPixels((GLint) mousePosition.x, height - (GLint) mousePosition.y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
    int newID = data[0] + data[1] * 256 + data[2] * 256 * 256;
    if (newID != 0) {
        pickedID = newID;
    }
    std::cout << pickedID << std::endl;
    pickedAsteroidTheta = cartesianToSpherical(offsets[pickedID]).z;
    picking = false;
}

AsteroidBelt::~AsteroidBelt() {
    delete[] offsets;
}

float AsteroidBelt::getThrowSpeed() {
    return THROW_SPEED;
}
