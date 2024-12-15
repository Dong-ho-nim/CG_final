#pragma once
#include <gl/glm/glm.hpp>
#include <gl/glew.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <set>
#include <vector>
#include "main.cpp"

class Monster : public CUBE {
public:
    Monster(); // 생성자 선언
    ~Monster();

    // 몬스터 이동 함수
    void Move(const glm::vec3& PlayerPosition);
    void Render();                             // OBJ 파일 렌더링
    void InitializeFromObjData(glm::vec3* vertexdata, glm::vec3* normaldata, glm::vec3* colordata, int count);
    void MonsterSpawn(CUBE& map, CUBE& monster);

    glm::vec3 getPosition()
    {
        return Position;
    };

private:
    glm::vec3 Position; // 몬스터의 현재 위치
    float Speed;        // 몬스터의 이동 속도

    GLuint vao;
    GLuint vbo[3];      // VBO: vertex, normal, color
    int vertex_count;   // 렌더링할 정점 개수

    void SetupBuffers(glm::vec3* vertexdata, glm::vec3* normaldata, glm::vec3* colordata, int count);
};