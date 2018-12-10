// subsurface_scattering.cpp : Defines the entry point for the console application.
//

#include<initwindow.h>
#include<glm\glm.hpp>
#include<glm\gtc\type_ptr.hpp>
#include<glm\gtc\matrix_transform.hpp>

#include<shader.h>
#include<iostream>

#define MPI 3.1415926535897932

int angle = 9;
int phiCount;
int thetaCount;
float *ballCoordinates;
int *indices;

int topTriangleNumber;
int midTriangleNumber;

void generateBallData();
void generateIndices();
int main()
{
	init();
	GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "subsurface_scattering", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback2);
	glfwSetScrollCallback(window, scroll_callback_high_accuracy);
	ifGladLoadRight();
	
	generateBallData();
	Shader shader("./shaders/points.vs", "./shaders/points.fs");
	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ballCoordinates)*((phiCount + 1 - 2)*(thetaCount)+2) * 3, ballCoordinates, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices)*(topTriangleNumber * 2 + midTriangleNumber) * 3, indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		shader.use();
		glm::mat4 model, view, projection;
		model = model*rotate_matrix;
		view = camera.GetViewMatrix();
		projection = glm::perspective(camera.Zoom, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		shader.setMat4("model", model);
		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		shader.setFloat("currentTime", glfwGetTime());

		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, ((phiCount + 1 - 2)*(thetaCount)+2) * 3);
		glDrawElements(GL_TRIANGLES, (topTriangleNumber * 2 + midTriangleNumber) * 3, GL_UNSIGNED_INT, NULL);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
    return 0;
}

void generateBallData()
{
	phiCount = 180 / angle;//在phi上总共的间隔
	thetaCount = 180 / angle * 2;//在theta上总共的间隔

	ballCoordinates = new float[((phiCount + 1 - 2)*(thetaCount)+2) * 3];
	std::cout << "points number is: " << ((phiCount + 1 - 2)*(thetaCount)+2) << std::endl;
	int index = 1;
	int minPhi = angle;
	int maxPhi = 180 - angle;
	
	for(int p=minPhi;p<=maxPhi;p+=angle)
		for (int t = 0; t <360; t += angle)
		{
			float phi = p*1.0 / 180 * MPI;
			float theta = t*1.0 / 180 * MPI;
			ballCoordinates[3 * index + 0] = sin(phi)*cos(theta);
			ballCoordinates[3 * index + 1] = sin(phi)*sin(theta);
			ballCoordinates[3 * index + 2] = cos(phi);

			index++;
		}
	ballCoordinates[0] = 0;
	ballCoordinates[1] = 0;
	ballCoordinates[2] = 1;
	ballCoordinates[3 * index + 0] = 0;
	ballCoordinates[3 * index + 1] = 0;
	ballCoordinates[3 * index + 2] = -1;

	generateIndices();
}

void generateIndices()
{
	topTriangleNumber = 360 / angle;
	midTriangleNumber = ((180 / angle) - 2)*(360 / angle) * 2;
	indices = new int[(topTriangleNumber * 2 + midTriangleNumber) * 3];

	int count = 0;
	//上
	for (int i = 0; i < topTriangleNumber;i++)
	{
		indices[i * 3 + 0] = 0;//三角形的顶点，前40和后40个同样的顶点
		indices[i * 3 + 1] = i + 1;
		indices[i * 3 + 2] = (i + 1 == topTriangleNumber) ? 1 : i + 2;
		count++;
	}

	int columnNumbers = 180 / angle -1;
	int rowNumbers = 360 / angle;
	for (int column = 1; column < columnNumbers; column++)
		for (int row = 0; row < rowNumbers; row++)
		{
			int topLeft = (column - 1)*(360 / angle) + row + 1;
			int topRight = (topLeft % 40) ? topLeft + 1 : (column - 1)*(360 / angle) + 1;
			int bottomLeft = topLeft + rowNumbers;
			int bottomRight = topRight + rowNumbers;

			indices[count * 3 + 0] = topLeft;
			indices[count * 3 + 1] = topRight;
			indices[count * 3 + 2] = bottomLeft;
			count++;

			indices[count * 3 + 0] = bottomLeft;
			indices[count * 3 + 1] = bottomRight;
			indices[count * 3 + 2] = topRight;
			count++;
		}

	int bottomStart= (columnNumbers-1)*rowNumbers+1;
	for (int i = 0; i < topTriangleNumber; i++)
	{
		indices[count * 3 + 0] = bottomStart + i;
		indices[count * 3 + 1] = (i == topTriangleNumber - 1) ? bottomStart : i + 1 + bottomStart;
		indices[count * 3 + 2] = ((phiCount + 1 - 2)*(thetaCount)+2)-1;

		count++;
	}
}
