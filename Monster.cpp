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

// 몬스터 이동 함수 정의
void Move(const glm::vec3& PlayerPosition) {
    // 플레이어와 몬스터 위치 간의 방향 계산
    glm::vec3 direction = PlayerPosition - worldmatrix.position;
    float distance = glm::length(direction);

    // 거리가 충분히 크면 이동
    if (distance > 0.01f) {
        direction = glm::normalize(direction); // 방향 벡터 정규화
        float speed = 0.02f; // 몬스터 이동 속도
        worldmatrix.position += direction * speed; // 위치 갱신
    }
}


void Monster::Render() {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    glBindVertexArray(0);
}

// 몬스터를 Cube의 면이 없는 정점에서 스폰
void Monster::MonsterSpawn(CUBE& map, CUBE& monster) {
    // 1. 면(Face)에 포함된 정점 인덱스를 추적
    std::set<int> usedVertices;
    for (int i = 0; i < map.f_count; i++) {
        usedVertices.insert(static_cast<int>(map.face[i].x - 1)); // Face는 1-based index
        usedVertices.insert(static_cast<int>(map.face[i].y - 1));
        usedVertices.insert(static_cast<int>(map.face[i].z - 1));
    }

    // 2. 사용되지 않은 정점(Vertex) 인덱스 찾기
    std::vector<int> unusedVertices;
    for (int i = 0; i < map.v_count; i++) {
        if (usedVertices.find(i) == usedVertices.end()) {
            unusedVertices.push_back(i);
        }
    }

    // 3. 사용되지 않은 정점 중 랜덤하게 하나 선택
    if (unusedVertices.empty()) {
        std::cerr << "Error: No unused vertices found for monster spawn.\n";
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, unusedVertices.size() - 1);
    int selectedVertexIndex = unusedVertices[dis(gen)];

    // 4. 선택된 정점의 위치로 몬스터 위치 설정
    monster.worldmatrix.position = map.vertex[selectedVertexIndex];

    std::cout << "Monster spawned at: ("
        << monster.worldmatrix.position.x << ", "
        << monster.worldmatrix.position.y << ", "
        << monster.worldmatrix.position.z << ")\n";
}