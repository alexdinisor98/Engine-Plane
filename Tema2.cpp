#include "Tema2.h"
#include "LabCamera_T2.h"
#include "Transform3D.h"

#include <vector>
#include <string>
#include <iostream>


#include <Core/Engine.h>

using namespace std;

Tema2::Tema2()
{
}

Tema2::~Tema2()
{
}

void Tema2::Init()
{
	renderCameraTarget = false;

	camera = new Laborator_T2::Camera_T2();
	camera->Set(glm::vec3(0, 2, 3.5f), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

	translatePlaneX = 0;
	translatePlaneY = 1;
	translatePlaneZ = 0;

	scaleFuelX = 0.7f;

	// 0 - stays still, 1 - goesUp, 2 - goesDown
	status = 0;
	angularStepZ = 0;
	angularStepX = 0;
	maxHeight = translatePlaneY;
	goDownIdx = 0;
	angularStepCloudX = 0;
	angularStepPyrY = 0;

	checkCollision = false;

	// Create the plane
	CreatePlane();

	// Create the propeller
	CreatePropeller();

	// Create the clouds
	for (int i = 0; i < numberOfClouds; i++) {
		translateCloudX[i] = -5 + 4 * i;
	}

	for (int i = 0; i < numberOfClouds; i++) {
		std::string s = std::to_string(i);
		std::string cloudName = "cloud" + s;
		CreateCloud(cloudName);
	}
	first = 3;
	second = 8;
	third = 13;
	fourth = numberPyr;
	float pivot;

	// Create Pyrs
	for (int i = 0; i < numberPyr; i++) {
		if (i < first) {
			translatePyrX[i] = 0.5 * i;
		}else if (i == first) {
			translatePyrX[i] = 2 * i;			
		}else if (i > first && i < second) {
			translatePyrX[i] = translatePyrX[i - 1] + 0.5;
		}else if (i == second) {
			translatePyrX[i] = 3 * i;
		}else if (i > second && i < third) {
			translatePyrX[i] = translatePyrX[i - 1] + 0.5;
		}else if (i == third) {
			translatePyrX[i] = 3 * i;
		}else if (i > third&& i < numberPyr) {
			translatePyrX[i] = translatePyrX[i - 1] + 0.5;
		}
	}
	translatePyrY = 1.5;
	translatePyrZ = translatePlaneZ;

	for (int i = 0; i < numberPyr; i++) {
		std::string s = std::to_string(i);
		std::string pyramidName = "pyramid" + s;
		CreatePyramid(pyramidName);
	}

	CreateRectBackLife("rectangleBack", glm::vec3(0, 0, 0));
	CreateRectFuel("rectangleFront", glm::vec3(1, 1, 0.3));


	Mesh* mesh = new Mesh("sphere");
	mesh->LoadMesh(RESOURCE_PATH::MODELS + "Primitives", "sphere.obj");
	meshes[mesh->GetMeshID()] = mesh;

	// Create a shader program for drawing face polygon with the color of the normal
	{
		Shader* shader = new Shader("ShaderTema2");
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/VertexShader.glsl", GL_VERTEX_SHADER);
		shader->AddShader("Source/Laboratoare/Tema2/Shaders/FragmentShader.glsl", GL_FRAGMENT_SHADER);
		shader->CreateAndLink();
		shaders[shader->GetName()] = shader;
	}

	projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);
}

Mesh* Tema2::CreateMesh(std::string name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned short>& indices)
{
	unsigned int VAO = 0;
	// TODO: Create the VAO and bind it
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// TODO: Create the VBO and bind it
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// TODO: Send vertices data into the VBO buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	// TODO: Crete the IBO and bind it
	unsigned int IBO;
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	// TODO: Send indices data into the IBO buffer
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

	// ========================================================================
	// This section describes how the GPU Shader Vertex Shader program receives data

	// set vertex position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);

	// set vertex normal attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));

	// set texture coordinate attribute
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3)));

	// set vertex color attribute
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));
	// ========================================================================

	// Unbind the VAO
	glBindVertexArray(0);

	// Check for OpenGL errors
	CheckOpenGLError();

	// Mesh information is saved into a Mesh object
	meshes[name] = new Mesh(name);
	meshes[name]->InitFromBuffer(VAO, static_cast<unsigned short>(indices.size()));
	meshes[name]->vertices = vertices;
	meshes[name]->indices = indices;
	return meshes[name];
}

void Tema2::FrameStart()
{
	// clears the color buffer (using the previously set color) and depth buffer
	glClearColor(0.96, 0.87, 0.7, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::ivec2 resolution = window->GetResolution();
	// sets the screen area where to draw
	glViewport(0, 0, resolution.x, resolution.y);
}

void Tema2::Update(float deltaTimeSeconds) {
	glLineWidth(10);

	if (isOrtoActive) {
		projectionMatrix = glm::ortho(left, right, buttom, top, 0.01f, 200.0f);
	}
	else {
		projectionMatrix = glm::perspective(RADIANS(fovAngle), window->props.aspectRatio, 0.01f, 200.0f);
	}


	int countCollisionPyrs = 0;
	for (int i = 0; i < numberPyr; i++) {
		bool checkCollision = intersect(translatePlaneX, translatePlaneY, translatePlaneZ, translatePyrX[i], translatePyrY, translatePyrZ,
			translatePlaneX + 0.615, translatePlaneY + 0.375, translatePlaneZ + 0.195, translatePyrX[i] + 0.15, translatePyrY + 0.225, translatePyrZ + 0.45);
		if (checkCollision) {
			indexPyrCollision[i] = 1;	
			countCollisionPyrs++;
		}
	}

	if (scaleFuelX >= 0 && scaleFuelX < 0.9) {
		if (countCollisionPyrs > 0) {
			scaleFuelX += 0.001f * countCollisionPyrs;
		}else {
			scaleFuelX -= 0.0006f;
		}
	}

	planeMatrix = glm::mat4(1);
	planeMatrix = glm::translate(planeMatrix, glm::vec3(translatePlaneX, translatePlaneY, translatePlaneZ));

	angularStepX += deltaTimeSeconds * 15.f;
	propellerMatrix = glm::mat4(1);
	propellerMatrix *= glm::translate(propellerMatrix, glm::vec3(translatePlaneX, translatePlaneY, translatePlaneZ));

	// stays still
	if (status == 0) {
		planeMatrix = Transform3D::Translate(translatePlaneX, translatePlaneY, translatePlaneZ);

		propellerMatrix = Transform3D::Translate(translatePlaneX, translatePlaneY, translatePlaneZ)
			* Transform3D::RotateOX(angularStepX);

		// goes Up
	}else if (status == 1) {
		angularStepZ = 0.2f;
		translatePlaneY += 0.0115f;
		planeMatrix = Transform3D::Translate(translatePlaneX, translatePlaneY, translatePlaneZ)
			* Transform3D::RotateOZ(angularStepZ);

		propellerMatrix = Transform3D::Translate(translatePlaneX, translatePlaneY, translatePlaneZ)
			* Transform3D::RotateOZ(angularStepZ)
			* Transform3D::RotateOX(angularStepX);
	}else {
		// goes Down
		angularStepZ = -0.25f;
		translatePlaneY -= 0.0125f;

		planeMatrix = Transform3D::Translate(translatePlaneX, translatePlaneY, translatePlaneZ)
			* Transform3D::RotateOZ(angularStepZ);

		propellerMatrix = Transform3D::Translate(translatePlaneX, translatePlaneY, translatePlaneZ)
			* Transform3D::RotateOZ(angularStepZ)
			* Transform3D::RotateOX(angularStepX);
	}

	RenderMesh(meshes["plane"], shaders["VertexNormal"], planeMatrix);
	RenderMesh(meshes["propeller"], shaders["VertexNormal"], propellerMatrix);

	angularStepCloudX += deltaTimeSeconds * 3.f;

	for (int i = 0; i < numberOfClouds; i++) {
		cloudMatrixArray[i] = glm::mat4(1);

		if (translateCloudX[i] > -8) {
			translateCloudX[i] -= 0.05;
			cloudMatrixArray[i] = glm::translate(cloudMatrixArray[i], glm::vec3(translateCloudX[i], 2.5 + 0.05 * i, -3 - i))
				* glm::scale(cloudMatrixArray[i], glm::vec3(0.1f))
				* Transform3D::RotateOX(angularStepCloudX);
		}
		else {
			translateCloudX[i] = 8;
			cloudMatrixArray[i] = glm::translate(cloudMatrixArray[i], glm::vec3(translateCloudX[i], 2.5 + 0.02 * i, -3 - 0.7 * i))
				* glm::scale(cloudMatrixArray[i], glm::vec3(0.1f))
				* Transform3D::RotateOX(angularStepCloudX);
		}
	}

	for (int i = 0; i < numberOfClouds; i++) {
		std::string s = std::to_string(i);
		std::string cloudName = "cloud" + s;
		RenderMesh(meshes[cloudName], shaders["VertexNormal"], cloudMatrixArray[i]);
	}

	angularStepPyrY += deltaTimeSeconds * 2.f;

	for (int i = 0; i < numberPyr; i++) {
		pyrMatrixArray[i] = glm::mat4(1);

		if (translatePyrX[i] > -20) {
			translatePyrX[i] -= 0.05;
			pyrMatrixArray[i] = glm::translate(pyrMatrixArray[i], glm::vec3(translatePyrX[i], translatePyrY, translatePyrZ))
				* Transform3D::RotateOY(-angularStepPyrY);
		}
		else {
			translatePyrX[i] = 20;
			indexPyrCollision[i] = 0;
			pyrMatrixArray[i] = glm::translate(pyrMatrixArray[i], glm::vec3(translatePyrX[i], translatePyrY, translatePyrZ))
				* Transform3D::RotateOY(-angularStepPyrY);
		}
	}

	for (int i = 0; i < numberPyr; i++) {
		std::string s = std::to_string(i);
		std::string name = "pyramid" + s;
		// if pyramid detected no collision, render it
		if (indexPyrCollision[i] == 0) {
			RenderMesh(meshes[name], shaders["VertexNormal"], pyrMatrixArray[i]);
		}
	}

	glm::mat4 rectBackMatrix = glm::mat4(1);
	rectBackMatrix = glm::translate(rectBackMatrix, glm::vec3(2, 2.55, 0))
		* glm::scale(rectBackMatrix, glm::vec3(0.6f));

	RenderMesh(meshes["rectangleBack"], shaders["VertexNormal"], rectBackMatrix);


	glm::mat4 rectFrontMatrix = glm::mat4(1);
	rectFrontMatrix = glm::translate(rectFrontMatrix, glm::vec3(2, 2.55, 0.2))
		* glm::scale(rectFrontMatrix, glm::vec3(scaleFuelX, 1, 1));

	RenderMesh(meshes["rectangleFront"], shaders["VertexNormal"], rectFrontMatrix);

	glm::mat4 modelMatrix = glm::mat4(1);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(-2.1, 2.5, 1))
				* glm::scale(modelMatrix, glm::vec3(0.2f));
	RenderMesh(meshes["sphere"], shaders["ShaderTema2"], modelMatrix);

		// Render the camera target. Useful for understanding where is the rotation point in Third-person camera movement
		/*if (renderCameraTarget) {
			glm::mat4 modelMatrix = glm::mat4(1);
			modelMatrix = glm::translate(modelMatrix, camera->GetTargetPosition());
			modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
			RenderMesh(meshes["sphere"], shaders["VertexNormal"], modelMatrix);
		}*/
}


void Tema2::FrameEnd() {
	DrawCoordinatSystem(camera->GetViewMatrix(), projectionMatrix);
}

void Tema2::RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix)
{
	if (!mesh || !shader || !shader->program)
		return;

	// render an object using the specified shader and the specified position
	shader->Use();
	glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
	glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

	mesh->Render();
}


void Tema2::OnInputUpdate(float deltaTime, int mods)
{
	// move the camera only if MOUSE_RIGHT button is pressed
	if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
	{
		float cameraSpeed = 2.0f;

		if (window->KeyHold(GLFW_KEY_W)) {
			camera->TranslateForward(deltaTime * cameraSpeed);
		}

		if (window->KeyHold(GLFW_KEY_A)) {
			// TODO : translate the camera to the left
			camera->TranslateRight(-deltaTime * cameraSpeed);
		}

		if (window->KeyHold(GLFW_KEY_S)) {
			// TODO : translate the camera backwards
			camera->TranslateForward(-deltaTime * cameraSpeed);
		}

		if (window->KeyHold(GLFW_KEY_D)) {
			// TODO : translate the camera to the right
			camera->TranslateRight(deltaTime * cameraSpeed);
		}

		if (window->KeyHold(GLFW_KEY_Q)) {
			// TODO : translate the camera down
			camera->TranslateUpword(-deltaTime * cameraSpeed);
		}

		if (window->KeyHold(GLFW_KEY_E)) {
			// TODO : translate the camera up
			camera->TranslateUpword(deltaTime * cameraSpeed);
		}

		if (window->KeyHold(GLFW_KEY_O)) {
			orthoLeft = -8.0f;
			orthoRight = 8.0f;
			orthoUp = 4.5f;
			orthoDown = -4.5;
			projectionMatrix = glm::ortho(orthoLeft, orthoRight, orthoDown, orthoUp, 0.01f, 200.0f);
			isOrtho = true;

		}

		if (window->KeyHold(GLFW_KEY_P)) {
			projectionMatrix = glm::perspective(90.f, 2.f, 2.f, 200.0f);
			isOrtho = false;
		}

		if (window->KeyHold(GLFW_KEY_1)) {
			left += 29.0f * deltaTime;
			right -= 29.0f * deltaTime;
		}

		if (window->KeyHold(GLFW_KEY_2)) {
			left = 29.0f * deltaTime;
			right += 29.0f * deltaTime;
		}

		if (window->KeyHold(GLFW_KEY_3)) {
			top -= 29.0f * deltaTime;
			buttom += 29.0f * deltaTime;
		}

		if (window->KeyHold(GLFW_KEY_4)) {
			top += 29.0f * deltaTime;
			buttom -= 29.0f * deltaTime;
		}

		// inversare FOV
		if (window->KeyHold(GLFW_KEY_K)) {
			projectionMatrix = glm::perspective(-90.f, -2.f, 2.f, -200.0f);
			isOrtho = false;
		}

		if (window->KeyHold(GLFW_KEY_Z)) {
			// TODO : translate the camera down
			fovAngle -= 19.0f * deltaTime;
		}

		if (window->KeyHold(GLFW_KEY_X)) {
			// TODO : translate the camera up
			fovAngle += 19.0f * deltaTime;
		}

	}
}

void Tema2::OnKeyPress(int key, int mods)
{
	// add key press event
	if (key == GLFW_KEY_T){
		renderCameraTarget = !renderCameraTarget;
	}

	if (key == GLFW_KEY_P) {
		isOrtoActive = true;
	}
	else if (key == GLFW_KEY_O) {
		isOrtoActive = false;
	}
}

void Tema2::OnKeyRelease(int key, int mods)
{
	// add key release event
}

void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
	// add mouse move event
	if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
	{
		float sensivityOX = 0.001f;
		float sensivityOY = 0.001f;

		if (window->GetSpecialKeyState() == 0) {
			renderCameraTarget = false;
			// TODO : rotate the camera in First-person mode around OX and OY using deltaX and deltaY
			// use the sensitivity variables for setting up the rotation speed
			camera->RotateFirstPerson_OX(-2 * sensivityOX * deltaY);
			camera->RotateFirstPerson_OY(-2 * sensivityOY * deltaX);
		}

		if (window->GetSpecialKeyState() && GLFW_MOD_CONTROL) {
			renderCameraTarget = true;
			// TODO : rotate the camera in Third-person mode around OX and OY using deltaX and deltaY
			// use the sensitivity variables for setting up the rotation speed
			camera->RotateThirdPerson_OX(-2 * sensivityOX * deltaY);
			camera->RotateThirdPerson_OY(-2 * sensivityOY * deltaX);
		}
	} 

	if (deltaY > 0) {
		status = 2;
	}
	else if (deltaY < 0) {
		status = 1;
	}else {
		status = 0;
	}	

}

void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button press event
}

void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
	// add mouse button release event
}

void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Tema2::OnWindowResize(int width, int height)
{
}

void Tema2::CreatePlane()
{
	{
		vector<VertexFormat> vertices
		{
			//body
			VertexFormat(glm::vec3(-0.15 , -0.15,  0.15), glm::vec3(0, 1, 1), glm::vec3(0.54, 0, 0)), // 0
			VertexFormat(glm::vec3(0.3, -0.15,  0.15), glm::vec3(1, 0, 1),  glm::vec3(0.9, 0.4, 0.2)),// 1
			VertexFormat(glm::vec3(-0.15,  0.15,  0.15), glm::vec3(1, 0, 0), glm::vec3(0.54, 0, 0.1)), // 2
			VertexFormat(glm::vec3(0.3,  0.15,  0.15), glm::vec3(0, 1, 0), glm::vec3(0.5, 0.2, 0)), // 3
			VertexFormat(glm::vec3(-0.15, -0.15, -0.15), glm::vec3(1, 1, 1),  glm::vec3(0.54, 0.2, 0)), // 4
			VertexFormat(glm::vec3(0.3, -0.15, -0.15), glm::vec3(0, 1, 1),  glm::vec3(0.9, 0.4, 0.2)),// 5
			VertexFormat(glm::vec3(-0.15,  0.15, -0.15), glm::vec3(1, 1, 0),  glm::vec3(0.54, 0, 0)), // 6
			VertexFormat(glm::vec3(0.3,  0.15, -0.15), glm::vec3(0, 0, 1),  glm::vec3(0.5, 0.2, 0)), // 7

			//head
			VertexFormat(glm::vec3(2 * 0.15, -0.8 * 0.15, 0.8 * 0.15), glm::vec3(0, 1, 1), glm::vec3(0.8, 0.36, 0.36)), // 8 - 0
			VertexFormat(glm::vec3(2.5 * 0.15, -0.8 * 0.15, 0.8 * 0.15), glm::vec3(1, 0, 1), glm::vec3(0.8, 0.36, 0.3)), // 9 - 1
			VertexFormat(glm::vec3(2 * 0.15,  0.8 * 0.15, 0.8 * 0.15), glm::vec3(1, 0, 0), glm::vec3(0.8, 0.3, 0.36)), // 10 - 2
			VertexFormat(glm::vec3(2.5 * 0.15,  0.8 * 0.15, 0.8 * 0.15), glm::vec3(0, 1, 0),glm::vec3(0.75, 0.36, 0.36)), // 11 - 3
			VertexFormat(glm::vec3(2 * 0.15, -0.8 * 0.15, -0.8 * 0.15), glm::vec3(1, 1, 1), glm::vec3(0.8, 0.36, 0.3)), // 12 - 4
			VertexFormat(glm::vec3(2.5 * 0.15, -0.8 * 0.15, -0.8 * 0.15), glm::vec3(0, 1, 1), glm::vec3(0.8, 0.3, 0.36)), // 13 - 5
			VertexFormat(glm::vec3(2 * 0.15,  0.8 * 0.15, -0.8 * 0.15), glm::vec3(1, 1, 0), glm::vec3(0.75, 0.36, 0.36)), // 14 - 6
			VertexFormat(glm::vec3(2.5 * 0.15,  0.8 * 0.15, -0.8 * 0.15), glm::vec3(0, 0, 1), glm::vec3(0.8, 0.3, 0.36)), //15 - 7

			//tail
			VertexFormat(glm::vec3(-1.5 * 0.15, -0.5 * 0.15, 0.5 * 0.15), glm::vec3(0, 1, 1), glm::vec3(0.8, 0.2, 0.2)), // 16 - 0
			VertexFormat(glm::vec3(-1 * 0.15, -0.5 * 0.15, 0.5 * 0.15), glm::vec3(1, 0, 1), glm::vec3(0.8, 0.2, 0.22)), // 17 - 1
			VertexFormat(glm::vec3(-1.5 * 0.15,  0.5 * 0.15, 0.5 * 0.15), glm::vec3(1, 0, 0), glm::vec3(0.82, 0.2, 0.2)), // 18 - 2
			VertexFormat(glm::vec3(-1 * 0.15,  0.5 * 0.15, -0.5 * 0.15), glm::vec3(0, 1, 0), glm::vec3(0.8, 0.22, 0.2)), // 19 - 3
			VertexFormat(glm::vec3(-1.5 * 0.15, -0.5 * 0.15, -0.5 * 0.15), glm::vec3(1, 1, 1), glm::vec3(0.8, 0.22, 0.22)), // 20 - 4
			VertexFormat(glm::vec3(-1 * 0.15, -0.5 * 0.15, 0.5 * 0.15), glm::vec3(0, 1, 1), glm::vec3(0.82, 0.2, 0.2)), // 21 - 5
			VertexFormat(glm::vec3(-1.5 * 0.15,  0.5 * 0.15, -0.5 * 0.15), glm::vec3(1, 1, 0), glm::vec3(0.8, 0.22, 0.2)), // 22 - 6
			//VertexFormat(glm::vec3(-1 * 0.15,  0.5 * 0.15, -0.5 * 0.15), glm::vec3(0, 0, 1), glm::vec3(0.8, 0.22, 0.2)), //23 - 7
																																													  VertexFormat(glm::vec3(-1,  0.5, -0.5), glm::vec3(0, 0, 1), glm::vec3(0.8, 0.2, 0.2)), //23 - 7
			//wing-front
			VertexFormat(glm::vec3(-0.2 * 0.15, -0.2 * 0.15,  2 * 0.15), glm::vec3(0, 1, 1), glm::vec3(0.2, 0.2, 0.2)), // 24 - 0
			VertexFormat(glm::vec3(1.2 * 0.15, -0.2 * 0.15,  2 * 0.15), glm::vec3(1, 0, 1), glm::vec3(0.2, 0.2, 0.18)), // 25 - 1
			VertexFormat(glm::vec3(-0.2 * 0.15,  0.2 * 0.15,  2 * 0.15), glm::vec3(1, 0, 0), glm::vec3(0.2, 0.118, 0.2)), // 26 - 2
			VertexFormat(glm::vec3(1.2 * 0.15,  0.2 * 0.15,  2 * 0.15), glm::vec3(0, 1, 0), glm::vec3(0.2, 0.2, 0.2)), // 27 - 3
			VertexFormat(glm::vec3(-0.2 * 0.15, -0.2 * 0.15, 1 * 0.15), glm::vec3(1, 1, 1), glm::vec3(0.2, 0.18, 0.2)), // 28 - 4
			VertexFormat(glm::vec3(1.2 * 0.15, -0.2 * 0.15, 1 * 0.15), glm::vec3(0, 1, 1), glm::vec3(0.2, 0.2, 0.2)), // 29 - 5
			VertexFormat(glm::vec3(-0.2 * 0.15,  0.2 * 0.15, 1 * 0.15), glm::vec3(1, 1, 0), glm::vec3(0.2, 0.18, 0.2)), // 30 - 6
			VertexFormat(glm::vec3(1.2 * 0.15,  0.2 * 0.15, 1 * 0.15), glm::vec3(0, 0, 1), glm::vec3(0.18, 0.2, 0.2)), // 31 - 7

			//wing-back
			VertexFormat(glm::vec3(-0.2 * 0.15, -0.2 * 0.15,  -1 * 0.15), glm::vec3(0, 1, 1),  glm::vec3(0.2, 0.2, 0.2)), // 32 - 0
			VertexFormat(glm::vec3(1.2 * 0.15, -0.2 * 0.15,  -1 * 0.15), glm::vec3(1, 0, 1),  glm::vec3(0.2, 0.18, 0.2)), // 33 - 1
			VertexFormat(glm::vec3(-0.2 * 0.15,  0.2 * 0.15,  -1 * 0.15), glm::vec3(1, 0, 0),  glm::vec3(0.18, 0.2, 0.2)), // 34 - 2
			VertexFormat(glm::vec3(1.2 * 0.15,  0.2 * 0.15,  -1 * 0.15), glm::vec3(0, 1, 0), glm::vec3(0.2, 0.118, 0.2)), // 35 - 3
			VertexFormat(glm::vec3(-0.2 * 0.15, -0.2 * 0.15, -2 * 0.15), glm::vec3(1, 1, 1),  glm::vec3(0.2, 0.2, 0.2)), // 36 - 4
			VertexFormat(glm::vec3(1.2 * 0.15, -0.2 * 0.15, -2 * 0.15), glm::vec3(0, 1, 1), glm::vec3(0.2, 0.2, 0.19)), // 37 - 5
			VertexFormat(glm::vec3(-0.2 * 0.15,  0.2 * 0.15, -2 * 0.15), glm::vec3(1, 1, 0), glm::vec3(0.2, 0.18, 0.2)), // 38 - 6
			VertexFormat(glm::vec3(1.2 * 0.15,  0.2 * 0.15, -2 * 0.15), glm::vec3(0, 0, 1), glm::vec3(0.18, 0.2, 0.2)), // 39 - 7

			//propeller stand
			VertexFormat(glm::vec3(2.5 * 0.15, -0.2 * 0.15, 0.2 * 0.15), glm::vec3(0, 1, 1), glm::vec3(1, 0.6, 0)), // 40 - 0
			VertexFormat(glm::vec3(2.8 * 0.15, -0.2 * 0.15, 0.2 * 0.15), glm::vec3(1, 0, 1),  glm::vec3(0.9, 0.64, 0)), // 41 - 1
			VertexFormat(glm::vec3(2.5 * 0.15,  0.2 * 0.15, 0.2 * 0.15), glm::vec3(1, 0, 0),   glm::vec3(1, 0.6, 0)), // 42 - 2
			VertexFormat(glm::vec3(2.8 * 0.15,  0.2 * 0.15, 0.2 * 0.15), glm::vec3(0, 1, 0),  glm::vec3(1, 0.6, 0)), // 42 - 3
			VertexFormat(glm::vec3(2.5 * 0.15, -0.2 * 0.15, -0.2 * 0.15), glm::vec3(1, 1, 1),  glm::vec3(1, 0.64, 0)), // 43 - 4
			VertexFormat(glm::vec3(2.8 * 0.15, -0.2 * 0.15, -0.2 * 0.15), glm::vec3(0, 1, 1),  glm::vec3(0.95, 0.64, 0)), // 44 - 5
			VertexFormat(glm::vec3(2.5 * 0.15,  0.2 * 0.15, -0.2 * 0.15), glm::vec3(1, 1, 0),  glm::vec3(1, 0.64, 0)), // 45 - 6
			VertexFormat(glm::vec3(2.8 * 0.15,  0.2 * 0.15, -0.2 * 0.15), glm::vec3(0, 0, 1),  glm::vec3(0.9, 0.64, 0)), //46 - 7

		};

		vector<unsigned short> indices =
		{
			0, 1, 2,		1, 3, 2,
			2, 3, 7,		2, 7, 6,
			1, 7, 3,		1, 5, 7,
			6, 7, 4,		7, 5, 4,
			0, 4, 1,		1, 4, 5,
			2, 6, 4,		0, 2, 4,

			8, 9, 10,		9, 11, 10,
			10, 11, 15,		10, 15, 14,
			9, 15, 11,		9, 13, 15,
			14, 15, 12,		15, 13, 12,
			8, 12, 9,		9, 12, 13,
			10, 14, 12,		8, 10, 12,

			//16, 17, 18,		17, 19, 18,
			//18, 19, 23,		18, 23, 22,
			//17, 23, 19,		17, 21, 23,
			//22, 23, 20,		23, 21, 20,
			//16, 20, 17,		17, 20, 21,
			//18, 22, 20,		16, 18, 20,

			24, 25, 26,		25, 27, 26,
			26, 27, 31,		26, 31, 30,
			25, 31, 27,		25, 29, 31,
			30, 31, 28,		31, 29, 28,
			24, 28, 25,		25, 28, 29,
			26, 30, 28,		24, 26, 28,

			32, 33, 34,		33, 35, 34,
			34, 35, 39,		34, 39, 38,
			33, 39, 35,		33, 37, 39,
			38, 39, 36,		39, 37, 36,
			32, 36, 33,		33, 36, 37,
			34, 38, 36,		32, 34, 36,

			40, 41, 42,		41, 43, 42,
			42, 43, 47,		42, 47, 46,
			41, 47, 43,		41, 45, 47,
			46, 47, 44,		47, 45, 44,
			40, 44, 41,		41, 44, 45,
			42, 46, 44,		40, 42, 44,
		};

		CreateMesh("plane", vertices, indices);
	}
}

void Tema2::CreatePropeller()
{
	{
		vector<VertexFormat> vertices
		{
			VertexFormat(glm::vec3(2.8 * 0.15, -1.5 * 0.15,  0.3 * 0.15), glm::vec3(0, 1, 1), glm::vec3(0.5, 0, 0)), // 0
			VertexFormat(glm::vec3(3.1 * 0.15, -1.5 * 0.15,  0.3 * 0.15), glm::vec3(1, -1, 1),  glm::vec3(0.5, 0, 0)), // 1
			VertexFormat(glm::vec3(2.8 * 0.15,  1.5 * 0.15,  0.3 * 0.15), glm::vec3(1, -1, 0),  glm::vec3(0.5, 0, 0)), // 2
			VertexFormat(glm::vec3(3.1 * 0.15,  1.5 * 0.15,  0.3 * 0.15), glm::vec3(0, 1, 0),  glm::vec3(0.5, 0, 0)), // 3
			VertexFormat(glm::vec3(2.8 * 0.15, -1.5 * 0.15, -0.3 * 0.15), glm::vec3(1, 1, 1), glm::vec3(0.5, 0, 0)), // 4
			VertexFormat(glm::vec3(3.1 * 0.15, -1.5 * 0.15, -0.3 * 0.15), glm::vec3(0, 1, 1),  glm::vec3(0.5, 0, 0)), // 5
			VertexFormat(glm::vec3(2.8 * 0.15,  1.5 * 0.15, -0.3 * 0.15), glm::vec3(1, 1, 0), glm::vec3(0.5, 0, 0)), // 6
			VertexFormat(glm::vec3(3.1 * 0.15,  1.5 * 0.15, -0.3 * 0.15), glm::vec3(0, -1, 1),  glm::vec3(0.5, 0, 0)), // 7
		};

		vector<unsigned short> indices =
		{
			0, 1, 2,		1, 3, 2,
			2, 3, 7,		2, 7, 6,
			1, 7, 3,		1, 5, 7,
			6, 7, 4,		7, 5, 4,
			0, 4, 1,		1, 4, 5,
			2, 6, 4,		0, 2, 4,
		};

		CreateMesh("propeller", vertices, indices);
	}
}

void Tema2::CreateCloud(std::string  cloudName){
	{
		vector<VertexFormat> vertices
		{
			VertexFormat(glm::vec3(0.5, -1,  1), glm::vec3(0, 1, 1), glm::vec3(0.52, 0.8, 0.97)), // 0
			VertexFormat(glm::vec3(2.5, -1,  1), glm::vec3(1, 0, 1), glm::vec3(0.52, 0.8, 0.97)), // 1
			VertexFormat(glm::vec3(0,  1,  1), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1)), // 2
			VertexFormat(glm::vec3(2,  1,  1), glm::vec3(0, 1, 0),  glm::vec3(0.117, 0.56, 1)), // 3
			VertexFormat(glm::vec3(0.5, -1, -1), glm::vec3(1, 1, 1),  glm::vec3(0, 0, 1)), // 4
			VertexFormat(glm::vec3(2.5, -1, -1), glm::vec3(0, 1, 1), glm::vec3(0.52, 0.8, 0.97)), // 5
			VertexFormat(glm::vec3(0,  1, -1), glm::vec3(1, 1, 0),  glm::vec3(0, 0, 1)), // 6
			VertexFormat(glm::vec3(2,  1, -1), glm::vec3(0, 0, 1), glm::vec3(0.52, 0.8, 0.97)), // 7

			VertexFormat(glm::vec3(1, -1.5,  1), glm::vec3(0, 1, 1), glm::vec3(0, 0, 1)), // 0
			VertexFormat(glm::vec3(3, -1.5,  1), glm::vec3(1, 0, 1), glm::vec3(0.52, 0.8, 0.97)), // 1
			VertexFormat(glm::vec3(1.6,  0.8,  1), glm::vec3(1, 0, 0),  glm::vec3(0.117, 0.56, 1)), // 2
			VertexFormat(glm::vec3(3.5,  0.8,  1), glm::vec3(0, 1, 0), glm::vec3(0.52, 0.8, 0.97)), // 3
			VertexFormat(glm::vec3(1, -1.5, -1), glm::vec3(1, 1, 1), glm::vec3(0.117, 0.56, 1)), // 4
			VertexFormat(glm::vec3(3, -1.5, -1), glm::vec3(0, 1, 1),  glm::vec3(0, 0, 1)), // 5
			VertexFormat(glm::vec3(1.6,  0.8, -1), glm::vec3(1, 1, 0), glm::vec3(0.52, 0.8, 0.97)), // 6
			VertexFormat(glm::vec3(3.5,  0.8, -1), glm::vec3(0, 0, 1), glm::vec3(0.52, 0.8, 0.97)), // 7

			VertexFormat(glm::vec3(3.8, -1.2,  1), glm::vec3(0, 1, 1), glm::vec3(0.117, 0.56, 1)), // 0
			VertexFormat(glm::vec3(5.5, -1.2,  1), glm::vec3(1, 0, 1),  glm::vec3(0, 0, 1)), // 1
			VertexFormat(glm::vec3(3,  1.2,  1), glm::vec3(1, 0, 0), glm::vec3(0.117, 0.56, 1)), // 2
			VertexFormat(glm::vec3(4.5,  1.2,  1), glm::vec3(0, 1, 0), glm::vec3(0.117, 0.56, 1)), // 3
			VertexFormat(glm::vec3(3.8, -1.2, -1), glm::vec3(1, 1, 1),  glm::vec3(0, 0, 1)), // 4
			VertexFormat(glm::vec3(5.5, -1.2, -1), glm::vec3(0, 1, 1),  glm::vec3(0.117, 0.56, 1)), // 5
			VertexFormat(glm::vec3(3,  1.2, -1), glm::vec3(1, 1, 0),  glm::vec3(0.52, 0.8, 0.97)), // 6
			VertexFormat(glm::vec3(4.5,  1.2, -1), glm::vec3(0, 0, 1), glm::vec3(0.52, 0.8, 0.97)), // 7

			VertexFormat(glm::vec3(3, -0.8,  1), glm::vec3(0, 1, 1), glm::vec3(0, 0, 1)), // 0
			VertexFormat(glm::vec3(6, -0.8,  1), glm::vec3(1, 0, 1),  glm::vec3(0.52, 0.8, 0.97)), // 1
			VertexFormat(glm::vec3(4,  0.8,  1), glm::vec3(1, 0, 0), glm::vec3(0, 0, 1)), // 2
			VertexFormat(glm::vec3(6,  0.8,  1), glm::vec3(0, 1, 0), glm::vec3(0.117, 0.56, 1)), // 3
			VertexFormat(glm::vec3(3, -0.8, -1), glm::vec3(1, 1, 1), glm::vec3(0.117, 0.56, 1)), // 4
			VertexFormat(glm::vec3(6, -0.8, -1), glm::vec3(0, 1, 1),  glm::vec3(0.117, 0.56, 1)), // 5
			VertexFormat(glm::vec3(4, 0.8, -1), glm::vec3(1, 1, 0), glm::vec3(0.52, 0.8, 0.97)), // 6
			VertexFormat(glm::vec3(6, 0.8, -1), glm::vec3(0, 0, 1),  glm::vec3(0.117, 0.56, 1)), // 7
		};

		vector<unsigned short> indices =
		{
			0, 1, 2,		1, 3, 2,
			2, 3, 7,		2, 7, 6,
			1, 7, 3,		1, 5, 7,
			6, 7, 4,		7, 5, 4,
			0, 4, 1,		1, 4, 5,
			2, 6, 4,		0, 2, 4,

			8, 9, 10,		9, 11, 10,
			10, 11, 15,		10, 15, 14,
			9, 15, 11,		9, 13, 15,
			14, 15, 12,		15, 13, 12,
			8, 12, 9,		9, 12, 13,
			10, 14, 12,		8, 10, 12,

			16, 17, 18,		17, 19, 18,
			18, 19, 23,		18, 23, 22,
			17, 23, 19,		17, 21, 23,
			22, 23, 20,		23, 21, 20,
			16, 20, 17,		17, 20, 21,
			18, 22, 20,		16, 18, 20,

			24, 25, 26,		25, 27, 26,
			26, 27, 31,		26, 31, 30,
			25, 31, 27,		25, 29, 31,
			30, 31, 28,		31, 29, 28,
			24, 28, 25,		25, 28, 29,
			26, 30, 28,		24, 26, 28,
		};

		CreateMesh(cloudName, vertices, indices);
	}
}

void Tema2::CreatePyramid(std::string name){
	vector<VertexFormat> vertices{
		VertexFormat(glm::vec3(1 * 0.15, 0,  0),glm::vec3(0, 1, 1), glm::vec3(1, 0, 0.3)), // 0 
		VertexFormat(glm::vec3(0, 1.5 * 0.15,  0),glm::vec3(1, 0, 1), glm::vec3(1, 0.4, 1)), // 1
		VertexFormat(glm::vec3(0, 0,  1 * 0.15),glm::vec3(1, 0, 0),  glm::vec3(1, 0.4, 1)), // 2
		VertexFormat(glm::vec3(0, 0, -2 * 0.15),  glm::vec3(0, 1, 0), glm::vec3(0.25, 0.1, 0.5)), // 3
	};

	vector<unsigned short> indices = {
		0,1,2,
		3,1,0,
		2,1,3,
		2,3,0
	};
	Mesh* pyramid = CreateMesh(name, vertices, indices);
}

void Tema2::CreateRectBackLife(std::string name, glm::vec3 color) {
	vector<VertexFormat> vertices{
		VertexFormat(glm::vec3(0, 0, 0), glm::vec3(0, 1, 1), color), // 0 
		VertexFormat(glm::vec3(2, 0, 0), glm::vec3(1, 0, 1), color), // 1
		VertexFormat(glm::vec3(0, 0.5, 0), glm::vec3(1, 0, 0), color), // 2
		VertexFormat(glm::vec3(2, 0.5, 0), glm::vec3(0, 1, 0), color), // 3
	};

	vector<unsigned short> indices = {
		0,1,2,
		2,3,1
	};

	Mesh* life = CreateMesh(name, vertices, indices);
}

void Tema2::CreateRectFuel(std::string name, glm::vec3 color) {
	vector<VertexFormat> vertices{
		VertexFormat(glm::vec3(0, 0, 0), glm::vec3(0, 1, 1), color), // 0 
		VertexFormat(glm::vec3(0.9, 0, 0), glm::vec3(1, 0, 1), color), // 1
		VertexFormat(glm::vec3(0, 0.22, 0), glm::vec3(1, 0, 0), color), // 2
		VertexFormat(glm::vec3(0.9, 0.22, 0), glm::vec3(0, 1, 0), color), // 3
	};

	vector<unsigned short> indices = {
		0,1,2,
		2,3,1
	};

	Mesh* life = CreateMesh(name, vertices, indices);
}


//void Tema2::CreateCylinder(float radius, float length, unsigned int numSteps) {
//		vector<unsigned short> m_indices;
//		vector<VertexFormat> m_verts;
//		m_verts.resize(numSteps * 2 + 2);
//
//		float hl = length * 0.5f;
//		float a = 0.0f;
//		float twicePi = 2.0f * 3.14;
//		float step = twicePi / (float)numSteps;
//		for (int i = 0; i < numSteps; i++){
//			float x = cos(a) * radius;
//			float y = sin(a) * radius;
//			m_verts.push_back(VertexFormat(glm::vec3(x, y, hl), glm::vec3(0, 1, 0), glm::vec3(0.25, 0.1, 0.5)));
//			m_verts.push_back(VertexFormat(glm::vec3(x, y, -hl), glm::vec3(0, 1, 0), glm::vec3(0.25, 0.1, 0.5)));
//			a += step;
//		}
//
//		//m_verts[numSteps * 2 + 0] = VertexFormat(glm::vec3(0.0f, 0.0f, +hl), glm::vec3(0, 1, 0), glm::vec3(0.25, 0.1, 0.5));
//		//m_verts[numSteps * 2 + 1] = VertexFormat(glm::vec3(0.0f, 0.0f, -hl), glm::vec3(0, 1, 0), glm::vec3(0.25, 0.1, 0.5));
//
//		m_indices.resize(4 * numSteps * 3);
//
//		for (int i = 0; i < numSteps; ++i){
//			unsigned short i1 = i;
//			unsigned short i2 = (i1 + 1) % numSteps;
//			unsigned short i3 = i1 + numSteps;
//			unsigned short i4 = i2 + numSteps;
//
//			// Sides
//
//			m_indices.push_back(i1);
//			m_indices.push_back(i3);
//			m_indices.push_back(i2);
//
//			m_indices.push_back(i4);
//			m_indices.push_back(i2);
//			m_indices.push_back(i3);
//
//			// Caps
//
//			m_indices[numSteps * 6 + i * 6 + 0] = numSteps * 2 + 0;
//			m_indices[numSteps * 6 + i * 6 + 1] = i1;
//			m_indices[numSteps * 6 + i * 6 + 2] = i2;
//
//			m_indices[numSteps * 6 + i * 6 + 3] = numSteps * 2 + 1;
//			m_indices[numSteps * 6 + i * 6 + 4] = i4;
//			m_indices[numSteps * 6 + i * 6 + 5] = i3;
//		}
//
//		Mesh* cylinder = CreateMesh("cylinder", m_verts, m_indices);
//
//}

//void Tema2::draw_cylinder(GLfloat radius,
//	GLfloat height,
//	GLubyte R,
//	GLubyte G,
//	GLubyte B)
//{
//	GLfloat x = 0.0;
//	GLfloat y = 0.0;
//	GLfloat angle = 0.0;
//	GLfloat angle_stepsize = 0.1;
//
//	/** Draw the tube */
//	glColor3ub(R - 40, G - 40, B - 40);
//	glBegin(GL_QUAD_STRIP);
//	angle = 0.0;
//	while (angle < 2 * 3.14) {
//		x = radius * cos(angle);
//		y = radius * sin(angle);
//		glVertex3f(x, y, height);
//		glVertex3f(x, y, 0.0);
//		angle = angle + angle_stepsize;
//	}
//	glVertex3f(radius, 0.0, height);
//	glVertex3f(radius, 0.0, 0.0);
//	glEnd();
//
//	/** Draw the circle on top of cylinder */
//	glColor3ub(R, G, B);
//	glBegin(GL_POLYGON);
//	angle = 0.0;
//	while (angle < 2 * 3.14) {
//		x = radius * cos(angle);
//		y = radius * sin(angle);
//		glVertex3f(x, y, height);
//		angle = angle + angle_stepsize;
//	}
//	glVertex3f(radius, 0.0, height);
//	glEnd();
//	float ang = 0.01f;
//	glTranslatef(0, 40, 0);//move everyting after this line by 40 units along y-axis
//	glRotatef(ang * 5, 0, 0, 1); //spin about z-axis
//}

bool Tema2::intersect(float a_MinX, float aMinY, float aMinZ, float bMinX, float bMinY, float bMinZ,
						float aMaxX, float aMaxY, float aMaxZ, float bMaxX, float bMaxY, float bMaxZ) {
	bool check = (a_MinX <= bMaxX && aMaxX >= bMinX) &&
		(aMinY <= bMaxY && aMaxY >= bMinY) &&
		(aMinZ <= bMaxZ && aMaxZ >= bMinZ);

	return check;
}


		