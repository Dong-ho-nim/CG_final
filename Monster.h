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
    Monster(); // ������ ����
    ~Monster();

    // ���� �̵� �Լ�
    void Move(const glm::vec3& PlayerPosition);
    void Render();                             // OBJ ���� ������
    void InitializeFromObjData(glm::vec3* vertexdata, glm::vec3* normaldata, glm::vec3* colordata, int count);
    void MonsterSpawn(CUBE& map, CUBE& monster);

    glm::vec3 getPosition()
    {
        return Position;
    };

private:
    glm::vec3 Position; // ������ ���� ��ġ
    float Speed;        // ������ �̵� �ӵ�

    GLuint vao;
    GLuint vbo[3];      // VBO: vertex, normal, color
    int vertex_count;   // �������� ���� ����

    void SetupBuffers(glm::vec3* vertexdata, glm::vec3* normaldata, glm::vec3* colordata, int count);
};