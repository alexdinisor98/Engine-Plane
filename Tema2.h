#pragma once
#include <Component/SimpleScene.h>
#include <Core/GPU/Mesh.h>
#include "LabCamera_T2.h"

class Tema2 : public SimpleScene
{
public:
	Tema2();
	~Tema2();

	void Init() override;

	void CreatePyramid(std::string name);
	void CreateRectBackLife(std::string name, glm::vec3 color);
	void CreateRectFuel(std::string name, glm::vec3 color);

	bool intersect(float aMinX, float aMinY, float aMinZ, float bMinX, float bMinY, float bMinZ,
		float aMaxX, float aMaxY, float aMaxZ, float bMaxX, float bMaxY, float bMaxZ);

	void CreateCylinder(float radius, float length, unsigned int numSteps);
	void draw_cylinder(GLfloat radius,
		GLfloat height,
		GLubyte R,
		GLubyte G,
		GLubyte B);
	

	void CreatePropeller();

	void CreateCloud(std::string  cloudName);

	void CreatePlane();

	Mesh* CreateMesh(std::string name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned short>& indices);
	

private:
	void FrameStart() override;
	void Update(float deltaTimeSeconds) override;

	void FrameEnd() override;

	void RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix) override;

	void OnInputUpdate(float deltaTime, int mods) override;
	void OnKeyPress(int key, int mods) override;
	void OnKeyRelease(int key, int mods) override;
	void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
	void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
	void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
	void OnWindowResize(int width, int height) override;

protected:
	Laborator_T2::Camera_T2* camera;
	glm::mat4 projectionMatrix;
	bool renderCameraTarget;
	float orthoLeft, orthoRight, orthoUp, orthoDown;
	bool isOrtho;
	float fovAngle = 60.0f;
	bool isOrtoActive = false;
	float left = -3.0f;
	float right = 3.0f;
	float top = 3.0f;
	float buttom = -3.0f;
	float translatePlaneX, translatePlaneY, translatePlaneZ;
	glm::mat4 planeMatrix;
	glm::mat4 propellerMatrix;
	glm::mat4 cloudMatrix;
	float angularStepZ, angularStepX;
	int staysStill, goesUp, goesDown;
	float status;
	float maxHeight;
	float goDownIdx;
	float angularStepCloudX, angularStepPyrY;
	glm::mat4 cloudMatrixArray[4] = { };
	int numberOfClouds = 4;
	float translateCloudX[4] = {};
	glm::mat4 pyrMatrixArray[21] = { };
	int numberPyr = 21;
	float translatePyrX[21] = {}, translatePyrY, translatePyrZ;
	int first, second, third, fourth;
	bool isNew = false;
	float scaleFuelX;
	bool checkCollision;
	int indexPyrCollision[10] = {0};
};
