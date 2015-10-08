﻿#include "GLWidget3D.h"
#include "MainWindow.h"
#include "OBJLoader.h"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "RuleParser.h"
#include <map>
#include "Rectangle.h"
#include "Polygon.h"
#include "GLUtils.h"
#include <QDir>

GLWidget3D::GLWidget3D(QWidget *parent) : QGLWidget(QGLFormat(QGL::SampleBuffers)) {
	// 光源位置をセット
	// ShadowMappingは平行光源を使っている。この位置から原点方向を平行光源の方向とする。
	light_dir = glm::normalize(glm::vec3(-4, -5, -8));

	// シャドウマップ用のmodel/view/projection行列を作成
	glm::mat4 light_pMatrix = glm::ortho<float>(-100, 100, -100, 100, 0.1, 200);
	glm::mat4 light_mvMatrix = glm::lookAt(-light_dir * 50.0f, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	light_mvpMatrix = light_pMatrix * light_mvMatrix;
}

/**
 * This event handler is called when the mouse press events occur.
 */
void GLWidget3D::mousePressEvent(QMouseEvent *e) {
	camera.mousePress(e->x(), e->y());
}

/**
 * This event handler is called when the mouse release events occur.
 */
void GLWidget3D::mouseReleaseEvent(QMouseEvent *e) {
}

/**
 * This event handler is called when the mouse move events occur.
 */
void GLWidget3D::mouseMoveEvent(QMouseEvent *e) {
	if (e->buttons() & Qt::LeftButton) { // Rotate
		camera.rotate(e->x(), e->y());
	} else if (e->buttons() & Qt::MidButton) { // Move
		camera.move(e->x(), e->y());
	} else if (e->buttons() & Qt::RightButton) { // Zoom
		camera.zoom(e->x(), e->y());
	}

	updateGL();
}

/**
 * This function is called once before the first call to paintGL() or resizeGL().
 */
void GLWidget3D::initializeGL() {
	renderManager.init("../shaders/vertex.glsl", "../shaders/geometry.glsl", "../shaders/fragment.glsl", false);

	// set the clear color for the screen
	qglClearColor(QColor(113, 112, 117));

	system.modelMat = glm::rotate(glm::mat4(), -3.1415926f * 0.5f, glm::vec3(1, 0, 0));

	/*
	std::vector<Vertex> vertices;
	glutils::drawGrid(60, 60, 1, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), glm::rotate(glm::mat4(), -3.1415926f * 0.5f, glm::vec3(1, 0, 0)), vertices);
	renderManager.addObject("grid", "", vertices);
	*/
}

/**
 * This function is called whenever the widget has been resized.
 */
void GLWidget3D::resizeGL(int width, int height) {
	height = height ? height : 1;
	glViewport(0, 0, width, height);
	camera.updatePMatrix(width, height);

	//rb.update(width, height);
	renderManager.resize(width, height);
}

/**
 * This function is called whenever the widget needs to be painted.
 */
void GLWidget3D::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	// Model view projection行列をシェーダに渡す
	glUniformMatrix4fv(glGetUniformLocation(renderManager.program, "mvpMatrix"),  1, GL_FALSE, &camera.mvpMatrix[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(renderManager.program, "mvMatrix"),  1, GL_FALSE, &camera.mvMatrix[0][0]);

	// pass the light direction to the shader
	//glUniform1fv(glGetUniformLocation(renderManager.program, "lightDir"), 3, &light_dir[0]);
	glUniform3f(glGetUniformLocation(renderManager.program, "lightDir"), light_dir.x, light_dir.y, light_dir.z);
	
	drawScene(0);
}

/**
 * Draw the scene.
 */
void GLWidget3D::drawScene(int drawMode) {
	if (drawMode == 0) {
		glUniform1i(glGetUniformLocation(renderManager.program, "depthComputation"), 0);
	} else {
		glUniform1i(glGetUniformLocation(renderManager.program, "depthComputation"), 1);
	}

	renderManager.renderAll();
}

void GLWidget3D::loadCGA(char* filename) {
	renderManager.removeObjects();

	/*
	std::vector<Vertex> vertices;
	glutils::drawGrid(60, 60, 1, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), glm::rotate(glm::mat4(), -3.1415926f * 0.5f, glm::vec3(1, 0, 0)), vertices);
	renderManager.addObject("grid", "", vertices);
	*/

	/*{ // for tutorial
		cga::Rectangle* lot = new cga::Rectangle("Lot", glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::mat4(), 35, 15, glm::vec3(1, 1, 1));
		system.stack.push_back(lot);
	}*/

	float object_width = 2.0f;
	float object_height = 1.0f;

	{ // for parthenon
		cga::Rectangle* start = new cga::Rectangle("Start", glm::translate(glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::vec3(-object_width*0.5f, -object_height*0.5f, 0)), glm::mat4(), object_width, object_height, glm::vec3(1, 1, 1));
		system.stack.push_back(boost::shared_ptr<cga::Shape>(start));
	}

	/*{ // This is for test.
		cga::Rectangle* lot = new cga::Rectangle("Lot", glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::mat4(), 5, 5, glm::vec3(1, 1, 1));
		system.stack.push_back(lot);
	}*/

	/*{
		std::vector<glm::vec2> points;
		points.push_back(glm::vec2(0, 0));
		points.push_back(glm::vec2(2, 0));
		points.push_back(glm::vec2(2, 2));
		points.push_back(glm::vec2(5, 2));
		points.push_back(glm::vec2(5, 5));
		points.push_back(glm::vec2(0, 5));
		cga::Polygon* lot = new cga::Polygon("Lot", glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::mat4(), points, glm::vec3(1, 1, 1), "");
		system.stack.push_back(lot);
	}*/

	try {
		cga::RuleSet ruleSet;
		cga::parseRule(filename, ruleSet);
		system.generate(ruleSet);
		system.generateGeometry(&renderManager);
	} catch (const std::string& ex) {
		std::cout << "ERROR:" << std::endl << ex << std::endl;
	} catch (const char* ex) {
		std::cout << "ERROR:" << std::endl << ex << std::endl;
	}
	
	renderManager.updateShadowMap(this, light_dir, light_mvpMatrix);

	updateGL();
}

void GLWidget3D::generateImages() {
	QDir dir("..\\cga\\windows\\");

	if (!QDir("results").exists()) QDir().mkdir("results");

	renderManager.renderingMode = RENDERING_MODE_LINE;

	camera.xrot = 90.0f;
	camera.yrot = 0.0f;
	camera.zrot = 0.0f;
	camera.pos = glm::vec3(0, 0, 1.5f);

	int origWidth = width();
	int origHeight = height();
	resize(256, 256);
	resizeGL(256, 256);

	QStringList filters;
	filters << "*.xml";
	QFileInfoList fileInfoList = dir.entryInfoList(filters, QDir::Files|QDir::NoDotAndDotDot);
	for (int i = 0; i < fileInfoList.size(); ++i) {
		int count = 0;

		for (float object_width = 1.0f; object_width <= 3.2f; object_width += 0.2f) {
			for (float object_height = 1.0f; object_height <= 2.0f; object_height += 0.2f) {
				renderManager.removeObjects();

				cga::Rectangle* start = new cga::Rectangle("Start", glm::translate(glm::rotate(glm::mat4(), -3.141592f * 0.5f, glm::vec3(1, 0, 0)), glm::vec3(-object_width*0.5f, -object_height*0.5f, 0)), glm::mat4(), object_width, object_height, glm::vec3(1, 1, 1));
				system.stack.push_back(boost::shared_ptr<cga::Shape>(start));

				try {
					cga::RuleSet ruleSet;
					cga::parseRule(fileInfoList[i].absoluteFilePath().toUtf8().constData(), ruleSet);
					system.generate(ruleSet);
					system.generateGeometry(&renderManager);
					renderManager.centerObjects();
				} catch (const std::string& ex) {
					std::cout << "ERROR:" << std::endl << ex << std::endl;
				} catch (const char* ex) {
					std::cout << "ERROR:" << std::endl << ex << std::endl;
				}

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);
				glDisable(GL_TEXTURE_2D);

				// Model view projection行列をシェーダに渡す
				glUniformMatrix4fv(glGetUniformLocation(renderManager.program, "mvpMatrix"),  1, GL_FALSE, &camera.mvpMatrix[0][0]);
				glUniformMatrix4fv(glGetUniformLocation(renderManager.program, "mvMatrix"),  1, GL_FALSE, &camera.mvMatrix[0][0]);

				// pass the light direction to the shader
				//glUniform1fv(glGetUniformLocation(renderManager.program, "lightDir"), 3, &light_dir[0]);
				glUniform3f(glGetUniformLocation(renderManager.program, "lightDir"), light_dir.x, light_dir.y, light_dir.z);
	
				drawScene(0);

				if (!QDir("results/" + fileInfoList[i].baseName()).exists()) QDir().mkdir("results/" + fileInfoList[i].baseName());

				QString filename = "results/" + fileInfoList[i].baseName() + "/" + QString("image_%1.jpg").arg(count, 3, 10, QChar('0'));
				grabFrameBuffer().save(filename);

				count++;
			}
		}
	}

	resize(origWidth, origHeight);
	resizeGL(origWidth, origHeight);
}

void GLWidget3D::hoge() {
	this->resize(256, 256);
	resizeGL(256, 256);
	updateGL();
}