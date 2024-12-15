#include "Monster.h"

Monster::Monster()
    : Position(0.0f, 0.0f, 0.0f), Speed(0.005f), vao(0), vertex_count(0) {
    glGenVertexArrays(1, &vao);
    glGenBuffers(3, vbo);
}

Monster::~Monster() {
    glDeleteBuffers(3, vbo);
    glDeleteVertexArrays(1, &vao);
}

void Monster::InitializeFromObjData(glm::vec3* vertexdata, glm::vec3* normaldata, glm::vec3* colordata, int count) {
    vertex_count = count;

    glBindVertexArray(vao);

    // Vertex Data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertex_count, vertexdata, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // Normal Data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertex_count, normaldata, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    // Color Data
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertex_count, colordata, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

// ���� �̵� �Լ� ����
void Move(const glm::vec3& PlayerPosition) {
    // �÷��̾�� ���� ��ġ ���� ���� ���
    glm::vec3 direction = PlayerPosition - worldmatrix.position;
    float distance = glm::length(direction);

    // �Ÿ��� ����� ũ�� �̵�
    if (distance > 0.01f) {
        direction = glm::normalize(direction); // ���� ���� ����ȭ
        float speed = 0.02f; // ���� �̵� �ӵ�
        worldmatrix.position += direction * speed; // ��ġ ����
    }
}


void Monster::Render() {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    glBindVertexArray(0);
}

// ���͸� Cube�� ���� ���� �������� ����
void Monster::MonsterSpawn(CUBE& map, CUBE& monster) {
    // 1. ��(Face)�� ���Ե� ���� �ε����� ����
    std::set<int> usedVertices;
    for (int i = 0; i < map.f_count; i++) {
        usedVertices.insert(static_cast<int>(map.face[i].x - 1)); // Face�� 1-based index
        usedVertices.insert(static_cast<int>(map.face[i].y - 1));
        usedVertices.insert(static_cast<int>(map.face[i].z - 1));
    }

    // 2. ������ ���� ����(Vertex) �ε��� ã��
    std::vector<int> unusedVertices;
    for (int i = 0; i < map.v_count; i++) {
        if (usedVertices.find(i) == usedVertices.end()) {
            unusedVertices.push_back(i);
        }
    }

    // 3. ������ ���� ���� �� �����ϰ� �ϳ� ����
    if (unusedVertices.empty()) {
        std::cerr << "Error: No unused vertices found for monster spawn.\n";
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, unusedVertices.size() - 1);
    int selectedVertexIndex = unusedVertices[dis(gen)];

    // 4. ���õ� ������ ��ġ�� ���� ��ġ ����
    monster.worldmatrix.position = map.vertex[selectedVertexIndex];

    std::cout << "Monster spawned at: ("
        << monster.worldmatrix.position.x << ", "
        << monster.worldmatrix.position.y << ", "
        << monster.worldmatrix.position.z << ")\n";
}